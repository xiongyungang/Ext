#include "stdafx.h"
#include <stdio.h>
#include <Winsock2.h>
#include <windows.h>
#include <af_irda.h>
#include "irdaclient.h"
#include "comserver.h"
#include "fcs16.h"
#include "LogTask.h"


#pragma comment(lib, "ws2_32.lib")
#define BUF_SIZE 16384
byte idtmLCPHead[4] = {0xff, 0x03, 0xc0, 0x21};
//byte terminateACKHead[5] = {0xff, 0x03, 0xc0, 0x21, 0x06};

IrdaClient *IrdaClient::m_IClient = NULL;

IrdaClient *IrdaClient::GetInstance()
{
    if (m_IClient == NULL)
    {
        m_IClient = new IrdaClient();
    }
    return m_IClient;
}

void IrdaClient::RemoveInstance()
{
    if (m_IClient != NULL)
    {
        delete m_IClient;
    }
}

IrdaClient::IrdaClient()
{
    //ClientInit();
    //printf("ClientInit end\n");
    m_endEvent = CreateEvent(NULL,FALSE,FALSE,NULL);//线程结束事件
}

IrdaClient::~IrdaClient()
{
    WSACleanup();
    join();
}

int IrdaClient::ClientInit()
{
    //GetLocalTime(&sysTime1);
    //printf("%4d/%02d/%02d %02d:%02d:%02d.%03d   IrdaClient::ClientInit()\n",sysTime1.wYear,sysTime1.wMonth,sysTime1.wDay,sysTime1.wHour,sysTime1.wMinute,sysTime1.wSecond,sysTime1.wMilliseconds);
    WORD wVersionRequested;  
    int err;  
    m_sock = INVALID_SOCKET;
    m_IsConnect = false;
    wVersionRequested = MAKEWORD( 2, 2 );   // 请求1.1版本的WinSock库  

    //初始化winSock
    err = WSAStartup( wVersionRequested, &m_wsaData );  
    if ( err != 0 ) 
    {  
        LogTask::LOG_PRINT(LEVEL_ERROR,"WSAStartup err %d",err);
        return -1;          // 返回值为零的时候是表示成功申请WSAStartup  
    }  

    if ( LOBYTE( m_wsaData.wVersion ) != 2 || HIBYTE( m_wsaData.wVersion ) != 2 ) 
    {  
        // 检查这个低字节是不是1，高字节是不是1以确定是否我们所请求的1.1版本  
        // 否则的话，调用WSACleanup()清除信息，结束函数  
        WSACleanup( );
        LogTask::LOG_PRINT(LEVEL_ERROR,"IrdaClient::ClientInit() WSAStartup wVersion err");
        return -1;   
    }

    return 0;
}

int IrdaClient::ClientStart()
{
    ClientInit();
    //int nRecvBuf=64*1024;//设置为64k
    //int nSendBuf=64*1024;//设置为64k
    char* pRecvBuf = new char[BUF_SIZE];
    //char* pSendBuf = new char[BUF_SIZE];

    while (WAIT_OBJECT_0 != WaitForSingleObject(m_endEvent,0))
    {
        if (!m_IsConnect)
        {
            Connect();
            Sleep(30);
        }
        else
        {
            int rsize = 0;
            memset(pRecvBuf, 0, BUF_SIZE);
            LogTask::LOG_PRINT(LEVEL_DEBUG,"recv() before");
            //rsize接受到的字节数
            rsize = recv(m_sock, pRecvBuf, BUF_SIZE,0);
            if (rsize <= 0)
            {
                //另一端已关闭则返回0
                if (rsize == 0)
                {
                    LogTask::LOG_PRINT(LEVEL_INFOR,"recv() client close");
                    //Sleep(3000);
                }
                //失败返回-1
                else
                {
                    LogTask::LOG_PRINT(LEVEL_ERROR,"recv() client err %d",WSAGetLastError());
                }
                ComServer::GetInstance()->SetComDTR(false);
                closesocket(m_sock);
                m_sock = INVALID_SOCKET;
                m_IsConnect = false;
                continue;
            }
            LogTask::LOG_PRINT(LEVEL_INFOR,"IrdaClient recv %d Bytes.",rsize);
            LogTask::LOG_PRINT(LEVEL_DEBUG,"IrdaClient recv buf head: %02x %02x %02x %02x %02x %02x",(byte)pRecvBuf[0],
                (byte)pRecvBuf[1],(byte)pRecvBuf[2],(byte)pRecvBuf[3],(byte)pRecvBuf[4],(byte)pRecvBuf[5]);
            //for (int i = 0; i < rsize; i++)
            //{
            //	printf("%02x ", (byte)pRecvBuf[i]);
            //}
            //printf("\n");

            if (memcmp(pRecvBuf, idtmLCPHead, 4) == 0)
            {				
                char pBuf[BUF_SIZE];
                int fcslen = 0;
                int sendlen = 0;
                //添加fcs校验
                addfcs((unsigned char *)pRecvBuf,rsize,fcslen);
                //转义
                LCPirda2com((byte*)pBuf, (byte*)pRecvBuf, sendlen, fcslen);
                ComServer::GetInstance()->SendData(pBuf, sendlen);
            }
            else
            {
                char pBuf[BUF_SIZE];
                int fcslen = 0;
                int sendlen = 0;
                //添加fcs校验
                addfcs((unsigned char *)pRecvBuf,rsize,fcslen);
                NCPirda2com((byte*)pBuf, (byte*)pRecvBuf, sendlen, fcslen);
                ComServer::GetInstance()->SendData(pBuf, sendlen);

            }
        }	
    }
    delete[]pRecvBuf;
    //delete[]pSendBuf;
    return 0;
}

void IrdaClient::svc()
{
    ClientStart();
}

void IrdaClient::stop()
{
    if (m_endEvent != NULL)
    {
        SetEvent(m_endEvent);
    }

}

int IrdaClient::SendData(char* buf, int sizes)
{
    int m = 0;
    if (!m_IsConnect)
    {
        LogTask::LOG_PRINT(LEVEL_ERROR,"IrdaClient:SendData sock no connect.");
        //Connect();
        return -1;
    }
    //LogTask::LOG_PRINT(LEVEL_INFOR,"IrdaClient:SendData send Bytes %d.",sizes);

    /**
    *向一个已连接的套接口发送数据
    *m_sock:一个用于标识已连接套接口的描述字
    *buf:包含待发送数据的缓冲区
    *sizes:缓冲区中数据的长度
    *0:调用执行方式，一般置0
    *返回所发送数据的总数
    */
    m = send(m_sock, buf, sizes, 0);
    if (m <= 0)
    {
        if (m == 0)
        {
            LogTask::LOG_PRINT(LEVEL_ERROR,"IrdaClient::SendData recv client close.");
        }
        else
        {
            LogTask::LOG_PRINT(LEVEL_ERROR,"IrdaClient::SendData send err.");
        }
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
        m_IsConnect = false;
        return -1;
    }
    //LogTask::LOG_PRINT(LEVEL_INFOR,"IrdaClient:SendData send success Bytes %d.",m);
    return 0;
}

int IrdaClient::Connect()
{
    //INVALID_SOCKET:无效套接字
    if (m_sock != INVALID_SOCKET)
    {
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
        m_IsConnect = false;
    }

    if ((m_sock = socket(AF_IRDA, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        // call WSAGetLastError
        LogTask::LOG_PRINT(LEVEL_ERROR,"IrdaClient:Connect socket err %d.",WSAGetLastError());
        return -1;
    }

    SOCKADDR_IRDA DestSockAddr = { AF_IRDA, 0, 0, 0, 0, "IrNetv1" };
    unsigned char DevListBuff[sizeof(DEVICELIST)];
    int DevListLen = sizeof(DevListBuff);
    PDEVICELIST pDevList = (PDEVICELIST) &DevListBuff;
    pDevList->numDevice = 0;

    if (getsockopt(m_sock, SOL_IRLMP, IRLMP_ENUMDEVICES,
        (char *)pDevList, &DevListLen) == SOCKET_ERROR)
    {
        // WSAGetLastError 
        LogTask::LOG_PRINT(LEVEL_ERROR,"IrdaClient:Connect getsockopt() err %d.",WSAGetLastError());
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
        return -1;
    }

    if (pDevList->numDevice > 0)
    {
        LogTask::LOG_PRINT(LEVEL_DEBUG,"IrdaClient:Connect numDevice %d.",pDevList->numDevice);
        memcpy(&DestSockAddr.irdaDeviceID[0], &pDevList->Device[0].irdaDeviceID[0], 4);
        //setsockopt(m_sock,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
        //setsockopt(m_sock,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));
        if (connect(m_sock, (const struct sockaddr *) &DestSockAddr,
            sizeof(SOCKADDR_IRDA)) == SOCKET_ERROR)
        {
            // WSAGetLastError
            LogTask::LOG_PRINT(LEVEL_ERROR,"IrdaClient:Connect connect() err %d.",WSAGetLastError());
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
            return -1;
        }
        ComServer::GetInstance()->SetComDTR(true);
        m_IsConnect = true;
    }
    return 0;
}

void IrdaClient::LCPirda2com(byte *descbuf, byte *srcbuf, int& deslen, int srclen)
{
    descbuf[0] = 0x7E;
    int j = 1;
    for (int i = 0; i < srclen; i++)
    {
        if (srcbuf[i] < 0x20 || srcbuf[i] == 0x7E || srcbuf[i] == 0x7D)
        {
            descbuf[j] = 0x7D;
            descbuf[j+1] = srcbuf[i]^0x20;
            j += 2;
        }
        else
        {
            descbuf[j] = srcbuf[i];
            j++;
        }	
    }
    descbuf[j] = 0x7E;
    deslen = j+1;
}

void IrdaClient::NCPirda2com(byte *descbuf, byte *srcbuf, int& deslen, int srclen)
{
    descbuf[0] = 0x7E;
    int j = 1;
    for (int i = 0; i < srclen; i++)
    {
        if (/*srcbuf[i] < 0x20 || */srcbuf[i] == 0x7E || srcbuf[i] == 0x7D)
        {
            descbuf[j] = 0x7D;
            descbuf[j+1] = srcbuf[i]^0x20;
            j += 2;
        }
        else
        {
            descbuf[j] = srcbuf[i];
            j++;
        }	
    }
    //memcpy(descbuf+1,srcbuf,srclen);
    //descbuf[srclen+1] = 0x7E;
    //deslen = srclen+2;
    descbuf[j] = 0x7E;
    deslen = j+1;
}
