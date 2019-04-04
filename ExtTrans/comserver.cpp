#include "stdafx.h"
#include "comserver.h"
//#include <Winsock2.h>
#include "irdaclient.h"
#include "fcs16.h"
#include "LogTask.h"

#define BUF_SIZE 16384

ComServer *ComServer::m_CServer = NULL;
byte LCPHead[6] = {0x7e, 0xff, 0x7d, 0x23, 0xc0, 0x21};
byte TerminateHead[5] = {0xff, 0x03, 0xc0, 0x21, 0x05};
byte TerminateRequest[8] = {0xff, 0x03, 0xc0, 0x21, 0x06, 0x09, 0x00, 0x04};

ComServer *ComServer::GetInstance()
{
    if (m_CServer == NULL)
    {
        m_CServer = new ComServer();
    }
    return m_CServer;
}

void ComServer::RemoveInstance()
{
    if (m_CServer != NULL)
    {
        delete m_CServer;
    }
}

ComServer::ComServer() : m_hComm(INVALID_HANDLE_VALUE)
{
    m_isTerminate = true;
    m_endEvent = CreateEvent(NULL,FALSE,FALSE,NULL);//�߳̽����¼�
}

ComServer::ComServer(const ComServer&)
{

}

ComServer& ComServer::operator=(const ComServer&)
{
    return *m_CServer;
}

ComServer::~ComServer()
{
    join();
}

void ComServer::SetCom(char *comname)
{
    memset(m_ComName,0,64);
    string s1 = "\\\\.\\";
    strncpy(m_ComName,s1.c_str(),4);
    strncpy(m_ComName+4, comname, 59);
}

void ComServer::SetComDTR(bool bDTR)
{
    DCB  dcb;
    GetCommState(m_hComm, &dcb);
    //dcb.fOutxDsrFlow = true;
    if (bDTR)
    {
        dcb.fDtrControl = 1;
    }
    else
    {
        dcb.fDtrControl = 0;
    }

    SetCommState(m_hComm, &dcb);
}

int ComServer::ComInit()
{
    /** ��ָ���Ĵ��� */
    //string s1 = "\\\\.\\COM15";
    m_hComm = CreateFile(m_ComName,  /** �豸��,COM1,COM2�� */  
        GENERIC_READ | GENERIC_WRITE, /** ����ģʽ,��ͬʱ��д */  
        0,                            /** ����ģʽ,0��ʾ������ */  
        NULL,                         /** ��ȫ������,һ��ʹ��NULL */  
        OPEN_EXISTING,                /** �ò�����ʾ�豸�������,���򴴽�ʧ�� */  
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,  //�첽ģʽ
        0);  
    /** �����ʧ�ܣ��ͷ���Դ������ */  
    if (m_hComm == INVALID_HANDLE_VALUE)  
    {  
        LogTask::LOG_PRINT(LEVEL_ERROR,"ComServer::ComInit() CreateFile %s error %d.",m_ComName, GetLastError());
        return -1;  
    }
    /** ���ô��ڵĳ�ʱʱ��,����Ϊ0,��ʾ��ʹ�ó�ʱ���� */ 
    COMMTIMEOUTS  CommTimeouts;  
    CommTimeouts.ReadIntervalTimeout = MAXDWORD;  
    CommTimeouts.ReadTotalTimeoutMultiplier = 0;  
    CommTimeouts.ReadTotalTimeoutConstant = 0;  
    CommTimeouts.WriteTotalTimeoutMultiplier = 0;  
    CommTimeouts.WriteTotalTimeoutConstant = 0;  

    BOOL bIsSuccess = SetCommTimeouts(m_hComm, &CommTimeouts);  
    //���û�����
    bIsSuccess = SetupComm(m_hComm,BUF_SIZE,BUF_SIZE); 
    DCB  dcb;  
    if (bIsSuccess)  
    {  
        /** ��ȡ��ǰ�������ò���,���ҹ��촮��DCB���� */  
        bIsSuccess = GetCommState(m_hComm, &dcb);  
        /** ����RTS flow���� */  
        dcb.fOutxDsrFlow = true;
        dcb.fDtrControl = 1;
        dcb.BaudRate = CBR_256000;//�����ʣ�ָ��ͨ���豸�Ĵ�������
        dcb.fParity = NOPARITY;// ָ����żУ��ʹ�ܡ����˳�ԱΪ1��������żУ����
        dcb.ByteSize = 8;// ͨ���ֽ�λ����4��8
        dcb.StopBits = ONESTOPBIT;//ָ��ֹͣλ��λ�����˳�Ա����������ֵ�� ONESTOPBIT 1λֹͣλ TWOSTOPBITS 2λֹͣλ

        /** ʹ��DCB�������ô���״̬ */  
        bIsSuccess = SetCommState(m_hComm, &dcb);  
    }

    /**  ��մ��ڻ����� */  
    PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);  

    memset(&m_osRead,0,sizeof(OVERLAPPED));
    m_osRead.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);
    memset(&m_osWrite,0,sizeof(OVERLAPPED));
    m_osWrite.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);

    return 0;
}

void ComServer::SendData(char* buf, int size)
{
    int m = 0;
    DWORD rsize = 0;
    if (m_isTerminate)
    {
        LogTask::LOG_PRINT(LEVEL_ERROR,"ComServer::SendData() PPP isTerminated.");
        return;
    }
    if (m_hComm == INVALID_HANDLE_VALUE)
    {
        LogTask::LOG_PRINT(LEVEL_ERROR,"ComServer::SendData() err m_hComm -1.");
        return;
    }
    //LogTask::LOG_PRINT(LEVEL_DEBUG,"ComServer::SendData() send Bytes %d.",size);
    /*
    for (int i = 0; i < size; i++)
    {
    GetLocalTime(&sysTime); 
    printf("%02X ", (byte)buf[i]);
    }
    printf("\n");
    */
    if(!WriteFile(m_hComm, buf, size, &rsize, &m_osWrite))
    {
        if(GetLastError()==ERROR_IO_PENDING)
        {
            GetOverlappedResult(m_hComm,&m_osWrite,&rsize,TRUE);
        }	
        else
        {
            LogTask::LOG_PRINT(LEVEL_ERROR,"ComServer::SendData() WriteFile err %d.", GetLastError());
        }
    }
    //LogTask::LOG_PRINT(LEVEL_DEBUG,"ComServer::SendData() WriteFile success Bytes %d.", rsize);
}

void ComServer::svc()
{
    ComInit();
    //printf("SerialSend:svc\n");
    char pRecvBuf[BUF_SIZE];
    //char pSendBuf[BUF_SIZE];
    DWORD dwCommEvent;
    //���ô���ͨ���¼���EV_RXCHAR�����յ�һ���ֽڲ��������뻺����
    if (!SetCommMask(m_hComm, EV_RXCHAR))
    {
        LogTask::LOG_PRINT(LEVEL_ERROR,"ComServer::svc() SetCommMask error:%d.", GetLastError());
    }
    while (WAIT_OBJECT_0 != WaitForSingleObject(m_endEvent,0))
    {

        int total = 0;
        //int index = 0;
        //INVALID_HANDLE_VALUE��ʾ��Ч�ľ��ֵ
        if (m_hComm != INVALID_HANDLE_VALUE)
        {	
            //�ȴ������¼��ı���������Щ�¼���ָ�ڸú���������ǰ�������Լ����úõ��ܴ��������̵߳���Ч�¼�
            //�����ж���SetCommMask()�������õĴ���ͨ���¼��Ƿ��ѷ���
            if (WaitCommEvent(m_hComm, &dwCommEvent, NULL) == false)
            {
                LogTask::LOG_PRINT(LEVEL_ERROR,"ComServer::svc() WaitCommEvent error:%d.", GetLastError());
                break;
            }
            LogTask::LOG_PRINT(LEVEL_DEBUG,"ComServer::svc() WaitCommEvent have event.");
            //����ֵ���ض�����ʵ�ʶ�����ֽ���
            DWORD rsize = 0;
            //int r_len= 0;

            //��ʼ����ȡ���ݴ洢����
            memset(pRecvBuf, 0, BUF_SIZE);
            //char *pbuf = pRecvBuf;

            //���ڶ�ȡʱ��������
            if(!ReadFile(m_hComm, pRecvBuf+total, BUF_SIZE - total, &rsize, &m_osRead))
            {
                if(GetLastError()==ERROR_IO_PENDING)
                {
                    GetOverlappedResult(m_hComm,&m_osRead,&rsize,TRUE);
                    continue;
                }
                else
                {
                    LogTask::LOG_PRINT(LEVEL_ERROR,"ComServer::svc() ReadFile err error:%d.", GetLastError());
                }
                //PurgeComm(m_hComm, PURGE_TXABORT|
                //	PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
            }
            LogTask::LOG_PRINT(LEVEL_INFOR,"ComServer::svc() ReadFile Bytes %d.",rsize);
            //��ȡ������
            if (rsize > 0)
            {
                LogTask::LOG_PRINT(LEVEL_DEBUG,"ComServer ReadFile buf head: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",(byte)pRecvBuf[0],
                    (byte)pRecvBuf[1],(byte)pRecvBuf[2],(byte)pRecvBuf[3],(byte)pRecvBuf[4],(byte)pRecvBuf[5],(byte)pRecvBuf[6],(byte)pRecvBuf[7],(byte)pRecvBuf[8],(byte)pRecvBuf[9]);
                //char debuf[1024];
                //memset(debuf, 0, 1024);
                //for (int i = 0; i < rsize && i < 1023; i++)
                //{
                //	sprintf(debuf+strlen(debuf), "%02X ", (byte)pRecvBuf[i]);
                //}
                //LogTask::LOG_PRINT(LEVEL_DEBUG,"ComServer ReadFile buf head: %s",debuf);

                if (strncmp(pRecvBuf, "AT",2) == 0)
                {
                    m_isTerminate = false;
                    ATResponse(pRecvBuf,rsize);
                }
                else if (strncmp(pRecvBuf, "+++",3) == 0)
                {
                }
                else if (strncmp(pRecvBuf, "CLIENT",6) == 0)
                {
                    m_isTerminate = false;
                    char retbuf[] = "CLIENTSERVER";
                    SendData(retbuf, strlen(retbuf));
                    //IrdaClient::GetInstance()->Connect();
                }
                else //if (memcmp(pRecvBuf, LCPHead,2) == 0)
                {
                    //����ʵ�ʶ����ֽ������ܺ�
                    total += rsize;
                    char* pBegin = NULL;
                    char* pEnd= NULL;
                    //���ݴ洢����
                    byte *pRec = (byte *)pRecvBuf;
                    for (int i=0; i < total; i++)
                    {
                        //0x7Eת��
                        if (pRec[i] == 0x7E)
                        {
                            if (pBegin == NULL)
                            {
                                if (pRec[i+1] == 0xFF)
                                {
                                    pBegin = pRecvBuf+i;
                                }	
                            }
                            else
                            {
                                pEnd = pRecvBuf+i;
                                int packsize = pEnd - pBegin +1;
                                char pBuf[BUF_SIZE];
                                int slen = 0;
                                LCPcom2irda((byte*)pBuf, (byte*)pBegin,slen, packsize);
                                //�Ƚ��ڴ�����buf1��buf2��ǰcount���ֽ�
                                if (memcmp(pBuf, TerminateHead, 5) == 0)
                                {
                                    char tembuf[64] = {0};
                                    char sedbuf[64] = {0};
                                    int fcslen = 0;
                                    int rlen = 0;
                                    memcpy(tembuf, TerminateRequest,8);
                                    tembuf[5] = pBuf[5];
                                    addfcs((unsigned char *)tembuf,8,fcslen);
                                    LCPirda2com((byte*)sedbuf, (byte*)tembuf, rlen, fcslen);
                                    SendData((char *)sedbuf, rlen);
                                    m_isTerminate = true;
                                    //char debuf[1024];
                                    //memset(debuf, 0, 1024);
                                    //for (int i = 0; i < rlen && i < 63; i++)
                                    //{
                                    //	sprintf(debuf+strlen(debuf), "%02X ", (byte)sedbuf[i]);
                                    //}
                                    //LogTask::LOG_PRINT(LEVEL_DEBUG,"ComServer send buf head: %s",debuf);
                                }
                                //printf("SendData  hex:\n");
                                //for (int j = 0; j < slen; j++)
                                //{
                                //	printf("%02X ", (byte)pBuf[j]);
                                //}
                                //printf("\n");
                                IrdaClient::GetInstance()->SendData(pBuf, slen);
                                pBegin = NULL;
                            }
                        }
                    }
                    if (pBegin == NULL && pEnd == NULL)
                    {
                        total = 0;
                    }
                    else if (pEnd - (total-1) == pRecvBuf)
                    {
                        total = 0;
                        //printf("total 0\n");
                    }
                    else
                    {
                        total = pRecvBuf + total - pBegin;
                        //printf("===total %d===\n",total);
                        memcpy(pRecvBuf,pBegin,total);
                    }			
                }
                //else
                //{
                //	printf("ReadFile Can not discern! hex:\n");
                //	for (int i = 0; i < rsize; i++)
                //	{
                //		printf("%02X ", (byte)pRecvBuf[i]);
                //	}
                //	printf("\n");
                //	//char pBuf[BUF_SIZE];
                //	//int slen = 0;
                //	//NCPcom2irda((byte*)pBuf, (byte*)pBegin,slen, packsize);
                //	//IrdaClient::GetInstance()->SendData(pBuf, slen);
                //}
            }	
        }
        else
            Sleep(1000);
    }
}

void ComServer::stop()
{
    if (m_endEvent != NULL)
    {
        SetEvent(m_endEvent);
    }
}

void ComServer::ATResponse(char *pbuf, int size)
{
    if (strncmp(pbuf, "ATD", 3) == 0) //ATDXXX\r
    {
        //IrdaClient::GetInstance()->Connect();
        char retbuf[128];
        sprintf(retbuf, "\rCONNECT\r");
        SendData(retbuf, strlen(retbuf));
    }
    else if (strncmp(pbuf, "AT", 2) == 0)
    {
        SendData("\rOK\r",4);
    }

}

//��ת��
void ComServer::LCPcom2irda(byte *descbuf, byte *srcbuf, int& deslen, int srclen)
{
    int j = 0;
    for (int i = 0; i < srclen; i++)
    {
        if (srcbuf[i] == 0x7D)
        {
            byte a = srcbuf[i+1]^0x20;
            i++;
            descbuf[j] = a;
        }
        else
        {
            if (i == 0)
            {
                continue;
            }
            else
                descbuf[j] = srcbuf[i];
        }
        j++;
    }

    deslen = j-3;//flag+fcs
}

//ת��
void ComServer::LCPirda2com(byte *descbuf, byte *srcbuf, int& deslen, int srclen)
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


void ComServer::NCPcom2irda(byte *descbuf, byte *srcbuf, int& deslen, int srclen)
{
    //memcpy(descbuf, srcbuf+1, srclen-4);

    //deslen = srclen-4;//flag+fcs
    int j = 0;
    for (int i = 0; i < srclen; i++)
    {
        if (srcbuf[i] == 0x7D)
        {
            byte a = srcbuf[i+1]^0x20;
            i++;
            descbuf[j] = a;
        }
        else
        {
            if (i == 0)
            {
                continue;
            }
            else
                descbuf[j] = srcbuf[i];
        }
        j++;
    }

    deslen = j-3;//flag+fcs
}