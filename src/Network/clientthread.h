#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include "gthread.h"
#include "Locker.h"
#include <string>
#include "tmsession.h"

class CantorHost;

class ClientThread : public GThread
{
    enum connectStatus
    {
        connectfail,
        disconnectedfromserver,
        connected
    };
public:
    ClientThread();
    ~ClientThread();

    bool Init(CantorHost *pHost,std::string strSrv,int port);
    int GetSrvPort()
    {
        return mPort;
    }
    void SetShortSession(bool b)
    {
        mShortSession = b;
    }
    void SetOnConnectedCall(ON_CONNECTED_CALL call,void* pContext)
    {
        mOnConnectedCall = call;
        mContextOfOnConnectedCall = pContext;
    }
    void Close();
    // GThread interface
protected:
    void run() override;
private:
    ON_CONNECTED_CALL mOnConnectedCall = nullptr;
    void* mContextOfOnConnectedCall = nullptr;

    bool mShortSession = false;//for registry, just receive a few Frame, then disconnected
    void SendRegFrame(TMSession* pSession);
    connectStatus clientSelect(std::string srv ,int port,TMSession* pSession);
    CantorHost* mHost =nullptr;
    std::string mClusterServer;
    int mPort =1973;
    int m_socket = 0;
    long int mLastUpdateTime = 0;
};

#endif // CLIENTTHREAD_H
