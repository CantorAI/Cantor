#include "clientthread.h"
#include <errno.h>
#include <sys/types.h>
#include "tmsession.h"
#include "utility.h"
#include <vector>
#include "CantorHost.h"
#include "FramePack.h"
#include "NetworkManager.h"

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

ClientThread::ClientThread()
{

}

ClientThread::~ClientThread()
{
}

bool ClientThread::Init(CantorHost *pHost,std::string strSrv,int port)
{
    mHost = pHost;
    mClusterServer = strSrv;
    mPort =port;
    mLastUpdateTime = getCurMilliTimeStamp();
    return true;
}

#define PACKSIZEWithHead (PACKET_SIZE+sizeof (CloverPacket))

extern std::vector<std::string> DnsLookup(std::string strHostName);

void ClientThread::SendRegFrame(TMSession* pSession)
{
    std::string nodeName = ((CantorHostImpl*)mHost)->GetNodeName();
    UID nodeId = ((CantorHostImpl*)mHost)->GetNodeId();
    DataFrame frm;
    frm.head->type = (FrameType)DataFrameType::Framework_ClientRegister;
    frm.head->startTime = getCurMilliTimeStamp();
    PackDataFrame pack(frm);
    pack << nodeName << nodeId;
    pack.Finish();
    pSession->SendFrame(frm,0);
}

ClientThread::connectStatus ClientThread::clientSelect(
    std::string srv ,int port,TMSession* pSession)
{
    int sd = 0 ;
    struct sockaddr_in server ;

    // Socket Initialization
    if( (sd = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        return connectStatus::connectfail;
    }
    std::string strIp;
    if (srv != "localhost" && srv != "127.0.0.1")
    {
        auto list = DnsLookup(srv);
        if (list.size() == 0)
        {
            return connectStatus::connectfail;
        }
        strIp = list[0];
    }
    else
    {
        strIp = "127.0.0.1";
    }
    //Server info
    server.sin_addr.s_addr  = inet_addr(strIp.c_str());
    server.sin_family       = AF_INET;
    server.sin_port         = htons(port);
    // Conect to server
    if(::connect(sd, (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        CLOSE_SOCKET(sd);
        return connectStatus::connectfail;
    }
    pSession->SetSocket(sd);
    m_socket = sd;
    NetworkManager::I().AddSession(pSession);
    pSession->Start();
    SendRegFrame(pSession);
    X::ARGS args = { strIp,port };
    X::KWARGS kwargs;
    NetworkManager::I().APISET().Fire(1, args, kwargs);
    pSession->WaitToFinish();
    return connectStatus::disconnectedfromserver;
}
#if 0
bool ClientThread::ConnectToNodeServer(ClusterNodeInfo* pNodeInfo,
    TMSession* pSession)
{
    connectStatus status = connectStatus::connectfail;
    for (auto locIp : pNodeInfo->localIPs)
    {
        std::string strIp = ws2s(locIp);
        status = clientSelect(strIp, pNodeInfo->localPort, pSession);
        if (status == connectStatus::disconnectedfromserver)
        {//connected and disconnected from server, means we can use this ip
         //to connect again, so break here to return to caller
         //and caller will call this function again if need
            break;
        }
    }
    if (status == connectStatus::connectfail)
    {
        std::string strIp = ws2s(pNodeInfo->publicIp);
        if (strIp.length() > 0 && pNodeInfo->externPort!=0)
        {
            status = clientSelect(strIp, pNodeInfo->externPort, pSession);
        }
    }
    return (status == connectStatus::disconnectedfromserver);
}
#endif
void ClientThread::Close()
{
    if (m_socket != 0)
    {
        CLOSE_SOCKET(m_socket);
        m_socket = 0;
    }
    //then session thread will be ended
}
void ClientThread::run()
{
    TMSession* pSession = new TMSession();
    pSession->SetShortSession(mShortSession);
    pSession->SetOnConnectedCall(mOnConnectedCall, mContextOfOnConnectedCall);
    pSession->SetAddrInfo(mClusterServer,0,(unsigned short)mPort);
    pSession->Init(mHost);
    pSession->IncRef();
    while(true)
    {
        bool bOK = false;
        //TODO: use cluster Node info
#if 0
        if (mClusterNodeInfo)
        {
            bOK = ConnectToNodeServer(mClusterNodeInfo, pSession);
        }
        else
#endif
        {
            auto status = clientSelect(mClusterServer, mPort, pSession);
            //if connected and server disconnected this session
            //don't need to sleep, just try again by bOK is true
            bOK = (status == connectStatus::disconnectedfromserver);
        }
        if(!bOK)
        {
            US_SLEEP(1000000L);
        }
        else if (mShortSession)
        {//only need one session per call
            break;
        }
        pSession->Reset();
        //TODO: add this line
        //mThalesMiletus->RemoveSession(pSession);
    }
    pSession->DecRef();//will be deleted if refCount =0

    NetworkManager::I().RemoveClient(this);
    delete this;
}
