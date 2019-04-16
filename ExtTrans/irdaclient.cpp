#include "stdafx.h"
#include <stdio.h>
#include <Winsock2.h>
#include <windows.h>
#include <af_irda.h>
#include "irdaclient.h"
#include "comserver.h"
#include "fcs16.h"
#include <baseLib/logLib.h>



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
        m_IClient = NULL;
    }
}

IrdaClient::IrdaClient()
{
    //ClientInit();
    //printf("ClientInit end\n");
    m_endEvent = CreateEvent(NULL,FALSE,FALSE,NULL);//�߳̽����¼�
 
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
    wVersionRequested = MAKEWORD( 2, 2 );   // ����1.1�汾��WinSock��  

    //��ʼ��winSock
    err = WSAStartup( wVersionRequested, &m_wsaData );  
    if ( err != 0 ) 
    {
        logMessage1(LOGLEVEL_ERROR,_T("WSAStartup err %d. \n"), err);
        return -1;          // ����ֵΪ���ʱ���Ǳ�ʾ�ɹ�����WSAStartup  
    }  

    if ( LOBYTE( m_wsaData.wVersion ) != 2 || HIBYTE( m_wsaData.wVersion ) != 2 ) 
    {  
        // ���������ֽ��ǲ���1�����ֽ��ǲ���1��ȷ���Ƿ������������1.1�汾  
        // ����Ļ�������WSACleanup()�����Ϣ����������  
        WSACleanup( );
        logMessage0(LOGLEVEL_ERROR,_T("IrdaClient::ClientInit() WSAStartup wVersion err \n"));
        return -1;   
    }

    return 0;
}

int IrdaClient::ClientStart()
{
    ClientInit();
    //int nRecvBuf=64*1024;//����Ϊ64k
    //int nSendBuf=64*1024;//����Ϊ64k
    char* pRecvBuf = new char[BUF_SIZE];
    //char* pSendBuf = new char[BUF_SIZE];

    while (WAIT_OBJECT_0 != WaitForSingleObject(m_endEvent, 0))
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
            logMessage0(LOGLEVEL_DEBUG,_T("recv() before \n"));
            //rsize���ܵ����ֽ���
            rsize = recv(m_sock, pRecvBuf, BUF_SIZE,0);
            if (rsize <= 0)
            {
                //��һ���ѹر��򷵻�0
                if (rsize == 0)
                {
                    logMessage0(LOGLEVEL_INFO,_T("recv() client close \n"));
                    //Sleep(3000);
                }
                //ʧ�ܷ���-1
                else
                {
                    logMessage1(LOGLEVEL_ERROR,_T("recv() client err %d \n"), WSAGetLastError());
                }
                ComServer::GetInstance()->SetComDTR(false);
                closesocket(m_sock);
                m_sock = INVALID_SOCKET;
                m_IsConnect = false;
                continue;
            }
            logMessage1(LOGLEVEL_INFO,_T("IrdaClient recv %d Bytes. \n"), rsize);
            logMessage6(LOGLEVEL_DEBUG,_T("IrdaClient recv buf head: %02x %02x %02x %02x %02x %02x \n"), (byte)pRecvBuf[0],
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
                //���fcsУ��
                addfcs((unsigned char *)pRecvBuf,rsize,fcslen);
                //ת��
                LCPirda2com((byte*)pBuf, (byte*)pRecvBuf, sendlen, fcslen);
                ComServer::GetInstance()->SendData(pBuf, sendlen);
            }
            else
            {
                char pBuf[BUF_SIZE];
                int fcslen = 0;
                int sendlen = 0;
                //���fcsУ��
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
        logMessage0(LOGLEVEL_ERROR,_T("IrdaClient:SendData sock no connect. \n"));
        //Connect();
        return -1;
    }
    //LogTask::LOG_PRINT(LEVEL_INFOR,"IrdaClient:SendData send Bytes %d.",sizes);

    /**
    *��һ�������ӵ��׽ӿڷ�������
    *m_sock:һ�����ڱ�ʶ�������׽ӿڵ�������
    *buf:�������������ݵĻ�����
    *sizes:�����������ݵĳ���
    *0:����ִ�з�ʽ��һ����0
    *�������������ݵ�����
    */
    m = send(m_sock, buf, sizes, 0);
    if (m <= 0)
    {
        if (m == 0)
        {
            logMessage0(LOGLEVEL_ERROR,_T("IrdaClient::SendData recv client close. \n"));
        }
        else
        {
            logMessage0(LOGLEVEL_ERROR,_T("IrdaClient::SendData send err. \n"));
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
    //INVALID_SOCKET:��Ч�׽���
    if (m_sock != INVALID_SOCKET)
    {
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
        m_IsConnect = false;
    }

    if ((m_sock = socket(AF_IRDA, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        // call WSAGetLastError
        logMessage1(LOGLEVEL_ERROR,_T("IrdaClient:Connect socket err %d. \n"),WSAGetLastError());
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
        logMessage1(LOGLEVEL_ERROR,_T("IrdaClient:Connect getsockopt() err %d. \n"), WSAGetLastError());
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
        return -1;
    }

    if (pDevList->numDevice > 0)
    {
        logMessage1(LOGLEVEL_DEBUG,_T("IrdaClient:Connect numDevice %d. \n"), pDevList->numDevice);
        memcpy(&DestSockAddr.irdaDeviceID[0], &pDevList->Device[0].irdaDeviceID[0], 4);
        //setsockopt(m_sock,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
        //setsockopt(m_sock,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));
        if (connect(m_sock, (const struct sockaddr *) &DestSockAddr,
            sizeof(SOCKADDR_IRDA)) == SOCKET_ERROR)
        {
            // WSAGetLastError
            logMessage1(LOGLEVEL_ERROR,_T("IrdaClient:Connect connect() err %d. \n"), WSAGetLastError());
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
