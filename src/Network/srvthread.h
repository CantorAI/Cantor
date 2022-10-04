#ifndef SRVTHREAD_H
#define SRVTHREAD_H

#include "gthread.h"
#include "Locker.h"
#include <string>

class CantorHost;
class SrvThread : public GThread
{
public:
    SrvThread();
    ~SrvThread();

    bool Init(CantorHost *pHost);
    std::wstring ReportSessionInfo();
    void Set(int port, int maxConn)
    {
        mPort = port;
        max_clients = maxConn;
    }
    bool IsRunning()
    {
        return m_bServerIsRunning;
    }
    void SetShortSession(bool b)
    {
        mShortSession = b;
    }
    int Port() { return mPort; }
protected:
    void run() override;
private:
    bool mShortSession = false;//for registry, just receive a few Frame, then disconnected
    int max_clients=30;
    int mPort =1973;
    CantorHost* mHost;

    bool m_bServerIsRunning = false;
};

#endif // SRVTHREAD_H
