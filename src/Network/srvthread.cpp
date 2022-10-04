#include "srvthread.h"
#include "tmsession.h"
#include "utility.h"
#include "cantor_host.h"
#include <iostream>
#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include "log.h"
#include "NetworkManager.h"

#if (WIN32)
#include <Windows.h>
#define CLOSE_SOCKET(sd) closesocket(sd)
#define READ_SOCKET(fd, buf, nbyte) recv(fd, buf, nbyte,0)
typedef int socklen_t;
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


SrvThread::SrvThread():
    mHost(NULL)
{

}

SrvThread::~SrvThread()
{

}

bool SrvThread::Init(CantorHost* pHost)
{
   mHost = pHost;
   return true;
}

std::wstring SrvThread::ReportSessionInfo()
{
    std::wstring strRet=L"Sessions:";
    return strRet;
}

#if (WIN32)
#define CANTOR_MAXHOST 100
std::vector<std::string> DnsLookup(std::string strHostName)
{
    std::vector<std::string> output;
    struct hostent* he;
    struct in_addr** addr_list;
    if ((he = gethostbyname(strHostName.c_str())) != NULL)
    {
        addr_list = (struct in_addr**)he->h_addr_list;
        for (int i = 0; addr_list[i] != NULL; i++)
        {
            auto addr = addr_list[i];
            if (addr->s_addr == 0x100007f)
            {//skip localhost(127.0.0.1)
                continue;
            }
            char ip[CANTOR_MAXHOST];
            strcpy(ip, inet_ntoa(*addr));
            output.push_back(ip);
        }
    }
    return output;
}
#else
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <ifaddrs.h>

std::vector<std::string> LocalIFList()
{
    std::vector<std::string> output;

    struct ifaddrs* ifap, * ifa;
    struct sockaddr_in* sa;
    char* addr;

    if (getifaddrs(&ifap) == -1)
    {
        return output;
    }
    for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET)
        {
            sa = (struct sockaddr_in*)ifa->ifa_addr;
            if (sa->sin_addr.s_addr == 0x101007f)
            {//skip localhost(127.0.0.1)
                continue;
            }
            addr = inet_ntoa(sa->sin_addr);

            output.push_back(addr);
            //printf("Interface: %s\tAddress: %s\n", ifa->ifa_name, addr);
        }
    }

    freeifaddrs(ifap);
    return output;
}
#if _NS_QUERY
std::vector<std::string> DnsLookup_Query(std::string strHostName)
{
    std::vector<std::string> output;

    unsigned char resolv[4096];
    char dispbuf[4096];
    int	res_len = 4096;
    if (res_query(strHostName.c_str(),
        ns_c_in, ns_t_a, resolv, res_len) < 0)
    {
        return output;
    }
    ns_msg msg;
    ns_rr rr;
    ns_initparse(resolv, res_len, &msg);
    int l = ns_msg_count(msg, ns_s_an);
    for (int i = 0; i < l; i++)
    {
        ns_parserr(&msg, ns_s_an, 0, &rr);
        struct in_addr in;
        memcpy(&in.s_addr, ns_rr_rdata(rr), sizeof(in.s_addr));
        auto addr = inet_ntoa(in);
        bool bFound = false;
        for (auto it = output.begin(); it != output.end(); it++)
        {
            if (*it == addr)
            {
                bFound = true;
                break;
            }
        }
        if (!bFound)
        {
            output.push_back(addr);
        }
    }
    return output;
}
#endif

std::vector<std::string> DnsLookup(std::string strHostName)
{
    std::vector<std::string> output;
    auto pushIP = [&](std::string ip)
    {
        bool bFind = false;
        for (auto it = output.begin(); it != output.end(); it++)
        {
            if (*it == ip)
            {
                bFind = true;
                break;
            }
        }
        if (!bFind)
        {
            output.push_back(ip);
        }
    };
    struct hostent* he;
    struct in_addr** addr_list;
    if ((he = gethostbyname(strHostName.c_str())) != NULL)
    {
        addr_list = (struct in_addr**)he->h_addr_list;
        for (int i = 0; addr_list[i] != NULL; i++)
        {
            auto addr = addr_list[i];
            if (addr->s_addr == 0x101007f)
            {//if it is localhost, check local IF
                std::vector<std::string> list = LocalIFList();
                for (auto li : list)
                {
                    pushIP(li);
                }
            }
            else
            {
                char ip[NI_MAXHOST];
                strcpy(ip, inet_ntoa(*addr));
                pushIP(ip);
            }
        }
    }
    return output;
}

std::vector<std::string> DnsLookup1(std::string strHostName)
{
    std::vector<std::string> output;

    struct addrinfo hints;
    struct addrinfo* result, * rp;
    int s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = PF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(strHostName.c_str(),NULL, &hints, &result);
    if (s != 0) 
    {
        return output;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) 
    {
        if (rp->ai_addr && rp->ai_family == AF_INET)
        {
            struct sockaddr_in* sa = (struct sockaddr_in*)rp->ai_addr;
            char* addr = inet_ntoa(sa->sin_addr);
            if (sa->sin_addr.s_addr == 0x100007f)
            {//skip localhost(127.0.0.1)
                continue;
            }
            output.push_back(addr);
            printf("Interface:Address: %s\n",addr);
        }
    }
    freeaddrinfo(result);
    return output;
}

#endif
#define TRUE   1
#define FALSE  0
#define PACKSIZEWithHead (PACKET_SIZE+sizeof (CloverPacket))
void SrvThread::run()
{
    char szHostName[255];
    gethostname(szHostName, 255);
    auto iplist = DnsLookup(szHostName);
    //TODO: add this
    //mHost->SetLocalIPList(iplist);

    int opt = TRUE;
    int master_socket , addrlen , new_socket,activity;

    int max_sd;
    struct sockaddr_in address;
    //set of socket descriptors
    fd_set readfds;


    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        LOG << "create socket failed";
        return;
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
          sizeof(opt)) < 0 )
    {
        LOG << "setsockopt failed";
        return;
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(mPort);

    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        LOG << "Cantor Network Server:bind failed";
        return;
    }
    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3))
    {
        LOG << "Cantor Network Server:listen failed";
        return;
    }
    LOG << "Cantor Network Server is listening,Port:"<< mPort
        <<",Max Client Num :"<<max_clients;
    m_bServerIsRunning = true;
    //accept the incoming connection
    addrlen = sizeof(address);

    bool bIdle = false;
    while(TRUE)
    {
        if(bIdle)
        {
            US_SLEEP(1);
        }
        bIdle = true;
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        activity = select( max_sd + 1 , &readfds , NULL, NULL , NULL);

        if ((activity < 0) && (errno!=EINTR))
        {
            LOG <<"Select error";
            continue;
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket,
                    (struct sockaddr *)&address, 
                (socklen_t*)&addrlen))<0)
            {
                LOG << "accept error";
                continue;
            }
            std::string strIP = inet_ntoa(address.sin_addr);
            auto port = ntohs(address.sin_port);

            TMSession* pSession = new TMSession();
            pSession->SetShortSession(mShortSession);
            pSession->Init(mHost);
            NetworkManager::I().AddSession(pSession);
            pSession->SetSocket(new_socket);
            pSession->Reset();
            pSession->SetAddrInfo(strIP, address.sin_addr.s_addr, port);
            X::ARGS args = { strIP,port };
            X::KWARGS kwargs;
            NetworkManager::I().APISET().Fire(0, args, kwargs);
            LOG << "Cantor:Session(IP:"<< strIP <<",Port:" << port << ") connected";
            pSession->Start();
            bIdle =false;
        }
    }
    m_bServerIsRunning = false;
}

