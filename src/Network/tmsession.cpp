#include "tmsession.h"
#include "srvthread.h"
#include "clientthread.h"
#include "utility.h"
#include <chrono>
#include "log.h"
#include "DataFramePack.h"
#include "FramePack.h"
#include "CantorHost.h"
#include "ConnectGraph.h"
#include "NodeManager.h"
#include "NetworkManager.h"

#define PACKSIZEWithHead (PACKET_SIZE+sizeof (CloverPacket))

#if (WIN32)
#include <windows.h>
#define CLOSE_SOCKET(sd) closesocket(sd)
#define READ_SOCKET(fd, buf, nbyte) recv(fd, buf, nbyte,0)
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netdb.h>

#define CLOSE_SOCKET(sd) close(sd)
#define READ_SOCKET(fd, buf, nbyte) read(fd, buf, nbyte)
#endif


TMReadThread::TMReadThread()
{
}

TMReadThread::TMReadThread(TMSession* parent) :
    mParent(parent)
{
}

TMReadThread::~TMReadThread()
{
}

void TMReadThread::run()
{
    while (mRun)
    {
        if (mParent)
        {
            if (!mParent->ReadProc())
            {
                mRun = false;
                break;
            }
        }
    }
    mParent->CleanDataFrameMap();
    mParent->DecRef();
}

TMWriteThread::TMWriteThread()
{
}

TMWriteThread::TMWriteThread(TMSession* parent) :
    mParent(parent)
{
}

TMWriteThread::~TMWriteThread()
{
}

void TMWriteThread::run()
{
    while (mRun)
    {
        if (mParent)
        {
            if (!mParent->WriteProc())
            {
                mRun = false;
                break;
            }
        }
    }
    mParent->DecRef();
}


unsigned int TMSession::mClientToken = 0;

TMSession::TMSession():
    mHeadFillSize(0),
    mDataFillSize(0)
{
}

TMSession::~TMSession()
{
    if (mReadThread)
    {
        delete mReadThread;
    }
    if (mWriteThread)
    {
        delete mWriteThread;
    }
}

void TMSession::Reset()
{
    //LOG << "TMSession::Reset()";

    mHeadFillSize =0;
    mDataFillSize =0;
}

bool TMSession::SendFrame(DataFrame& frm, int priority)
{
    if (priority < 0 || priority >= Priority_Level)
    {
        return false;
    }
    char* pData = (char*)frm.head;
    int Size = sizeof(DataFrameHead);
    char* pData2 = frm.data;
    unsigned long long Size2 = frm.head->dataSize;

    unsigned long long allSize = Size + Size2;
    unsigned short curDataIndex = ++mDataIndex;

    int PackNum = (allSize + PACKET_SIZE - 1) / PACKET_SIZE;
    for (int i = 0; i < PackNum; i++)
    {
        int nPacketSize = PACKET_SIZE;
        int offset = i * PACKET_SIZE;
        if (i == PackNum - 1)
        {
            nPacketSize = allSize - offset;
        }
        char* pBuf = new char[sizeof(CloverPacket)+ nPacketSize];
        CloverPacket* pPack = (CloverPacket*)pBuf;
        pPack->Prfix[0] = 'X';
        pPack->Prfix[1] = 'W';
        pPack->DataSize = allSize;
        pPack->DataIndex = curDataIndex;
        pPack->PackIndex = i;
        pPack->PacketSize = nPacketSize;

        char* pPackBuf = pBuf + sizeof(CloverPacket);
        if(offset<Size)
        {
            if(offset+nPacketSize<=Size)
            {
                memcpy(pPackBuf, pData + offset, nPacketSize);
            }
            else
            {
                int part1 = Size-offset;
                int part2 = nPacketSize-part1;
                if(part2>(int)Size2)
                {
                    part2 =Size2;
                }
                memcpy(pPackBuf, pData + offset, part1);
                memcpy(pPackBuf+part1, pData2, part2);
            }
        }
        else
        {
            memcpy(pPackBuf, pData2 + offset-Size, nPacketSize);
        }
        m_lockPacketQueue.Lock();
        m_PacketQueue[priority].push_back(pPack);
        m_lockPacketQueue.Unlock();
    }
    return true;
}
void TMSession::CleanDataFrameMap()
{
    for (auto it : m_DataFrameMap)
    {
        if (it.second)
        {
            delete it.second;
        }
    }
    m_DataFrameMap.clear();
}
void TMSession::ProcessFrame(DataFrame& frm)
{
    if (frm.head->type == 
        (FrameType)DataFrameType::Framework_ClientRegister
        || frm.head->type ==
        (FrameType)DataFrameType::Framework_ClientRegister_Ack)
    {
        PackDataFrame pack(frm);
        std::string nodeName0;
        UID uid0;
        pack >> nodeName0 >> uid0;
        m_nodeLock.Lock();
        m_anotherSideNodeName = nodeName0;
        m_antoherSideNodeId = uid0;
        m_nodeLock.Unlock();
        NodeManager::I().AddNode(nodeName0, uid0, this);
        UID thisNodeId = ((CantorHostImpl*)mHost)->GetNodeId();
        if (mOnConnectedCall)
        {
            mOnConnectedCall(
                mContextOfOnConnectedCall,
                this, nodeName0, uid0);
        }
        if (frm.head->type ==
            (FrameType)DataFrameType::Framework_ClientRegister)
        {
            ConnectGraph::I().AddEdge(uid0, thisNodeId);
            std::string nodeName = ((CantorHostImpl*)mHost)->GetNodeName();
            DataFrame frm;
            frm.head->type = 
                (FrameType)DataFrameType::Framework_ClientRegister_Ack;
            frm.head->startTime = getCurMilliTimeStamp();
            PackDataFrame pack_back(frm);
            pack_back << nodeName << thisNodeId;
            pack_back.Finish();
            SendFrame(frm, 0);
        }
        else
        {//connect from this node to Server:uid0
            ConnectGraph::I().AddEdge(thisNodeId,uid0);
        }
    }
    else if (frm.head->type ==
        (FrameType)DataFrameType::Framework_RegisterToRegistry)
    {
        frm.head->format[0] = mIP;
        mHost->PushFrame(frm);
    }
    else
    {
        mHost->PushFrame(frm);
    }
}
int TMSession::Sending()
{
    m_lockPacketQueue.Lock();
    for (int i = 0; i < Priority_Level; i++)
    {
        if (m_PacketQueue[i].size() > 0)
        {
            CloverPacket* pPacket = m_PacketQueue[i][0];
            send(mSessionhandle, (char*)pPacket, 
                sizeof(CloverPacket)+pPacket->PacketSize, 0);
            //TODO: check send result
            m_PacketQueue[i].erase(m_PacketQueue[i].begin());
            delete (char*)pPacket;
            break;//only send one packet per call
        }
    }
    m_lockPacketQueue.Unlock();
    return 0;
}

void TMSession::Init(CantorHost *pHost)
{
    mHost = pHost;
    mLastUpdateTime = getCurMilliTimeStamp();

    mReadBuffer = new char[PACKSIZEWithHead];
    mReadThread = new TMReadThread(this);
    mWriteThread = new TMWriteThread(this);
}

void TMSession::Start()
{
    mReadThread->Init();
    IncRef();
    mWriteThread->Init();
    IncRef();
    mReadThread->Start();
    mWriteThread->Start();
}

void TMSession::IncRef()
{
    mRefCountLock.Lock();
    mRefCount++;
    mRefCountLock.Unlock();
}
void TMSession::DecRef()
{
    int cnt = 0;
    mRefCountLock.Lock();
    cnt = --mRefCount;
    mRefCountLock.Unlock();
    if (cnt == 0)
    {
        NetworkManager::I().RemoveSession(this);
        NodeManager::I().RemoveNode(m_antoherSideNodeId);
        delete this;
    }
}

bool TMSession::ReadProc()
{
    bool bOK = true;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(mSessionhandle, &readfds);
    int ret = select(mSessionhandle + 1, &readfds, NULL, NULL, NULL);
    if ((ret < 0) && (errno != EINTR))
    {
        bOK = false;
    }
    else if (FD_ISSET(mSessionhandle, &readfds))
    {
        int valread;
        if ((valread = READ_SOCKET(mSessionhandle, 
            mReadBuffer, PACKSIZEWithHead)) <= 0)
        {
            //todo:process error
            SetSocket(0);
            CLOSE_SOCKET(mSessionhandle);
            bOK = false;
        }
        else
        {
            Parse(mReadBuffer, valread);
        }
    }
    return bOK;
}
void TMSession::SendHeartFrame()
{

}
bool TMSession::WriteProc()
{
    bool bOK = true;
    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(mSessionhandle, &writefds);
    int ret = select(mSessionhandle + 1, nullptr, &writefds, NULL, NULL);
    if ((ret < 0) && (errno != EINTR))
    {
        bOK = false;
    }
    else if (FD_ISSET(mSessionhandle, &writefds))
    {
        int iRes = Sending();
        long int i64NowTime = getCurMilliTimeStamp();
        if (iRes > 0)
        {
            mLastUpdateTime = i64NowTime;
        }
        if (i64NowTime - mLastUpdateTime > 30 * 1000)
        {
            mLastUpdateTime = i64NowTime;
            SendHeartFrame();
        }
    }
    return bOK;
}

void TMSession::Parse(char* data,int size)
{
    while(size)
    {
        ParsePacket(data,size);
    }
}

void TMSession::ParsePacket(char *&data, int &size)
{
    //Phase One: Fill mHead for CloverPacket
    if(mHeadFillSize<(int)sizeof(CloverPacket))
    {
        int needSize = sizeof(CloverPacket)-mHeadFillSize;
        if(needSize>size)
        {
            needSize = size;
        }
        if(needSize>0)
        {
            char* pHeadBuf = (char*)&mHead;
            memcpy(pHeadBuf+mHeadFillSize,data,needSize);
            mHeadFillSize+=needSize;
            size -=needSize;
            data+=needSize;
        }
    }
    if(mHeadFillSize<(int)sizeof(CloverPacket))
    {
        //will fill by next packet
        return;
    }
    if(!(mHead.Prfix[0]=='X' && mHead.Prfix[1] =='W'))
    {
        LOG << "Wrong packet";
        //Reset to next packet
        mDataFillSize =0;
        mHeadFillSize =0;
        return;
    }
    //Head is ready
    DataFramePack* pDFPack = nullptr;
    unsigned int dataIdex = mHead.DataIndex;
    auto it = m_DataFrameMap.find(dataIdex);
    if (it == m_DataFrameMap.end())
    {
        pDFPack = new DataFramePack(dataIdex);
        m_DataFrameMap.emplace(std::make_pair(dataIdex, pDFPack));
    }
    else
    {
        pDFPack = it->second;
    }
    //Phase fill the payload data
    //until it reaches mHead.PacketSize
    int packSize = mHead.PacketSize;
    //current packet is not full
    if((int)pDFPack->GetCurPacketFillSize() < packSize)
    {
        int needSize =packSize- pDFPack->GetCurPacketFillSize();
        if(needSize>size)
        {
            needSize = size;
        }
        if(needSize>0)
        {
            bool bFrameIsReady = pDFPack->AppendData(data, needSize);
            if (bFrameIsReady)
            {
                ProcessFrame(pDFPack->GetFrame());
                it = m_DataFrameMap.find(dataIdex);
                if (it != m_DataFrameMap.end())
                {
                    m_DataFrameMap.erase(it);
                }
                delete pDFPack;
                //reset
                pDFPack = nullptr;
                mHeadFillSize = 0;
            }
            size -=needSize;
            data+=needSize;
        }
    }
    //Current Packet is full
    if(pDFPack != nullptr &&
        (int)pDFPack->GetCurPacketFillSize() ==packSize)
    {
        //Reset to next packet
        pDFPack->ResetForNextPacket();
        mHeadFillSize =0;
    }
}

void TMSession::WaitToFinish()
{
    if (mReadThread)
    {
        mReadThread->WaitToEnd();
    }
    if (mWriteThread)
    {
        mWriteThread->WaitToEnd();
    }
}
