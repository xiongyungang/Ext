#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"
#include "libwebsockets.h"
#include <map>

class TaskBase
{
public:
    TaskBase()
    {
        m_handle = NULL;
    }
    ~TaskBase()
    {

    }
    void start()
    {
        m_handle = CreateThread(NULL, 0, run, this, 0, NULL);
        if ( m_handle == NULL)
        {
            printf("TaskBase::start CreateThread error:%d\n", GetLastError());
        }
    }
    virtual void svc() = 0;
protected:
    HANDLE m_handle;
    static DWORD WINAPI run(LPVOID  pVoid)
    {
        TaskBase *task = (TaskBase*)pVoid;
        task->svc();
        return 0;
    }
    void join()
    {
        WaitForSingleObject(m_handle, INFINITE);
    }
};

typedef void (*responsFun)(void *in, size_t len);

class CWebSocket : public TaskBase
{
public:
    CWebSocket();
    ~CWebSocket();

    virtual void svc();

    void wsSetCallBack(responsFun cbFun);
    long wsConnect(LPCSTR url);
    int wsRead(LPCSTR buf, int len);
    int wsWrite(LPCSTR buf, int len);
    int wsAddHeader(LPCSTR key, LPCSTR value);
    //int wsDownloadFile(LPCSTR url, LPCSTR path);
    //int wsUploadFile(LPCSTR url, LPCSTR filepath);

    void closeConnect();
    void saveData(char* in, size_t len);
    void setSSLcert(LPCSTR certPath, LPCSTR keyPath, LPCSTR caPath);
    int sendData();

    static int ws_service_callback(struct lws *wsi,
        enum lws_callback_reasons reason, void *user,
        void *in, size_t len);
    responsFun m_responsFun;
    int m_force_exit;
    int m_sendlen;
    struct lws *m_pWsi;
    int m_Stat;
    int m_httpCompleted;
    std::map<std::string, std::string> m_mapHeader;
protected:
    char m_pWsData[4097];
    int m_WsDataLen;
    struct lws_context *m_pContext;
    int m_use_ssl;
    char m_certPath[1024];
    char m_keyPath[1024];
    char m_caPath[1024];

    char *m_writeBuf;
    int m_writeLen;
    int m_isPost;
    FILE *m_pFile;

    lws_context* CreateContext();

};