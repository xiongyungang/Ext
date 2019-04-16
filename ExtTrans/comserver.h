#pragma once
#ifndef __COMSERVER_H__  
#define __COMSERVER_H__ 
#include "TaskBase.h"
#include <string>
using namespace std;

class ComServer: public TaskBase
{
public:
    static ComServer * GetInstance();
    static void RemoveInstance();

    ~ComServer();
    int ComInit();
    void SendData(char* buf, int size);
    virtual void svc();
    virtual void stop();
    void SetCom(char *comname);
    void SetComDTR(bool bDTR);
private:
    ComServer();
    ComServer(const ComServer&);
    ComServer& operator=(const ComServer&);

    //int OpenDev(char *dev);
    void ATResponse(char *buf, int size);
    void LCPcom2irda(byte *descbuf, byte *srcbuf, int& deslen, int srclen);
    void LCPirda2com(byte *descbuf, byte *srcbuf, int& deslen, int srclen);

    void NCPcom2irda(byte *descbuf, byte *srcbuf, int& deslen, int srclen);

private:
    HANDLE  m_hComm;
    OVERLAPPED m_osRead;
    OVERLAPPED m_osWrite;
    //int m_speed;
    static ComServer *m_CServer;
    char m_ComName[64];
    bool m_isTerminate;
};

#endif
