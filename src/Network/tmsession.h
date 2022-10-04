#ifndef TMSession_H
#define TMSession_H
#include "cantor_host.h"
#include "gthread.h"
#include "Locker.h"
#include "gxydef.h"
#include <string>
#include <unordered_map>

class TMSession;
typedef void (*ON_CONNECTED_CALL)(void* pContext,TMSession* pSession,
    std::string& nodeName, UID& nodeId);


#pragma pack(push,4)
struct CloverPacket
{
    char Prfix[2];
    unsigned short DataIndex;
    unsigned int DataSize;
    unsigned short PackIndex;
    unsigned short PacketSize;
};
#pragma pack(pop)

#define PACKET_SIZE (48*1024-sizeof(CloverPacket))
#define Priority_Level 3

class DataFramePack;
class TMSession;
class TMReadThread : public GThread
{
public:
    TMReadThread();
    TMReadThread(TMSession* parent);
    ~TMReadThread();
    void Init()
    {
        mRun = true;
    }
    // GThread interface
protected:
    TMSession* mParent = nullptr;
    bool mRun = true;
    void run() override;
};
class TMWriteThread : public GThread
{
public:
    TMWriteThread();
    TMWriteThread(TMSession* parent);
    ~TMWriteThread();
    void Init()
    {
        mRun = true;
    }
    // GThread interface
protected:
    TMSession* mParent = nullptr;
    bool mRun = true;
    void run() override;
};
class TMSession
{
public:
    TMSession();
    virtual ~TMSession();
    void SetSocket(int handle)
    {
        mSessionhandle =handle;
    }
    int GetSocket() {return mSessionhandle;}
    void Init(CantorHost* pHost);
    void Start();
    void SetAddrInfo(std::string strSrv,unsigned int ip,unsigned short port)
    {
        mSrvAddr =strSrv;
        mIP =ip;
        mPort =port;
    }
    std::string GetAddr()
    {
        return mSrvAddr;
    }
    unsigned short GetPort()
    {
        return mPort;
    }
    bool SendFrame(DataFrame& frm, int priority);

//for read and write thread
    void IncRef();
    void DecRef();
    bool ReadProc();
    bool WriteProc();

    void Parse(char* data,int size);
    void ParsePacket(char*& data,int& size);
    void Reset();
    int Sending();
    void WaitToFinish();
    
    void SetOnConnectedCall(ON_CONNECTED_CALL call, void* pContext)
    {
        mOnConnectedCall = call;
        mContextOfOnConnectedCall = pContext;
    }
    void SetShortSession(bool b)
    {
        mShortSession = b;
    }
protected:
    void SendHeartFrame();
    long int mLastUpdateTime = 0;
    TMReadThread* mReadThread = nullptr;
    TMWriteThread* mWriteThread = nullptr;
    char* mReadBuffer;
private:
    ON_CONNECTED_CALL mOnConnectedCall = nullptr;
    void* mContextOfOnConnectedCall = nullptr;
    bool mShortSession = false;
    int mSessionhandle =0;

    //Fill per each CloverPacket
    CloverPacket mHead;
    int mHeadFillSize;//for CloverPacket Head
    int mDataFillSize;//for CloverPacket Data

    static unsigned int mClientToken;
private://network
    CantorHost* mHost = nullptr;
    std::string mSrvAddr;
    unsigned int mIP =0;
    unsigned short mPort =0;

    Locker mRefCountLock;
    int mRefCount = 0;
    
    Locker m_lockPacketQueue;
    std::vector<CloverPacket*> m_PacketQueue[Priority_Level];
    unsigned short mDataIndex = 0;

    //another side info
    Locker m_nodeLock;
    std::string m_anotherSideNodeName;
    UID m_antoherSideNodeId;
public:
    UID GetAntoherSideNodeId() { return m_antoherSideNodeId; }
    void CleanDataFrameMap();
private:
    void ProcessFrame(DataFrame& frm);
    //DataFrame filling
    //Only be used from Read Thread,
    //including session close phase,
    //will delete them from Read Thread
    std::unordered_map<unsigned int,DataFramePack*> m_DataFrameMap;
};

#endif // TMSession_H
