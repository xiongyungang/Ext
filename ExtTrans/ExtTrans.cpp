#include "stdafx.h"
#include <libFlow.h>
#include "LogTask.h"
#include "comserver.h"

#include "irdaclient.h"


class CTransPlugin : public CFlowPlugin
{
public:
    //插件名称
    virtual LPCTSTR GetName(void);
    //engine开始/插件加载事件
    virtual void OnEngineStart(PFlowEngine pEngine);
    //显示函数统计
    virtual void showGlobalDelay(void);

protected:
    GM_DECLARE_METHOD(CTransPlugin,transStart);
    GM_DECLARE_METHOD(CTransPlugin,transEnd);

protected:
    LogTask *m_pLogTask;
    ComServer *m_pComServer;                      
    IrdaClient *m_pIrdaClient;

protected:
    void transStart(LPCTSTR comName,int Loglevel);
    void transEnd();

};


#pragma comment(lib,"zlib1")
#pragma comment(lib,"libeay32")
#pragma comment(lib,"ssleay32")

#ifdef _DEBUG
#pragma comment(lib,"syslogD")
#pragma comment(lib,"libScriptD")
#else
#pragma comment(lib,"syslog")
#pragma comment(lib,"libScriptR")
#endif

#ifdef _UNICODE

#ifdef _DEBUG
#pragma comment(lib,"baseLibUD")
#pragma comment(lib,"flowcoreUD")
#else
#pragma comment(lib,"baseLibUR")
#pragma comment(lib,"flowcoreU")
#endif

#else //!_UNICODE

#ifdef _DEBUG
#pragma comment(lib,"baseLibD")
#pragma comment(lib,"flowcoreD")
#else
#pragma comment(lib,"baseLibR")
#pragma comment(lib,"flowcore")
#endif

#endif //!_UNICODE

GD_IMPLEMENT(transStart)
GD_IMPLEMENT(transEnd)

extern "C"
{

    PFlowPlugin _PLUGIN_API_ getPlugin(DWORD magic,DWORD dwVersion)
    {
        static CTransPlugin m_nInstance;

        if (MAGIC_VMT != magic) return NULL;
        if (PLUGIN_VERSION_CURRENT != dwVersion) return NULL;
        return &m_nInstance;
    }

}

LPCTSTR CTransPlugin::GetName(void)
{
    return _T("ExtTrans");
}

void CTransPlugin::OnEngineStart(CFlowEngine* pEngine)
{
    CFlowPlugin::OnEngineStart(pEngine);
    GM_REGISTER_METHOD(pEngine,this,transStart,"TI",NULL);//参数comName和LogLevel
    GM_REGISTER_METHOD(pEngine,this,transEnd,NULL,NULL);
}

void CTransPlugin::showGlobalDelay(void)
{
    GD_PRINTEX(transStart)
    GD_PRINTEX(transEnd)
}

GM_METHOD_LD_BEGIN(CTransPlugin,transStart)
    DEBUG_BREAK_IF(2 != numIn);
    DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[0]));
    DEBUG_BREAK_IF(!GV_IS_I4(pIn[1]));
    pGmc->transStart(pIn[0].strVal,pIn[1].lVal);
GM_METHOD_LD_END()

GM_METHOD_LD_BEGIN(CTransPlugin,transEnd)
    DEBUG_BREAK_IF(0 != numIn);
    DEBUG_BREAK_IF(0 != pGV->m_strRet.length());
    pGmc->transEnd();
GM_METHOD_LD_END()

void CTransPlugin::transStart(LPCSTR strCom,int Loglevel)
{
    char comName[64] = {0};
    strncpy(comName,strCom,64);
    if(strcmp(comName,"")==0 || atoi(comName+3) == 0)
    {
        printf("Error in COM name or log level \n");
        return;
    }

    m_pLogTask = LogTask::GetInstance();
    m_pLogTask->SetLogLevel(Loglevel);
    m_pLogTask->start();
    LogTask::LOG_PRINT(LEVEL_INFOR,"IrdatoCom %s", PRVERSION);
    m_pComServer = ComServer::GetInstance();
    m_pComServer->SetCom(comName);
    m_pComServer->start();
    m_pIrdaClient = IrdaClient::GetInstance();
    m_pIrdaClient->start();
}

void CTransPlugin::transEnd()
{
    if (m_pIrdaClient != NULL)
    {
        m_pIrdaClient->stop();
    }
    
    if (m_pComServer != NULL)
    {
        m_pComServer->stop();
    }

    if (m_pLogTask != NULL)
    {
        m_pLogTask->stop();
    }
       
}
