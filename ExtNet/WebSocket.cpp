#include "stdafx.h"
#include "WebSocket.h"
#include <shlwapi.h>
#include <baselib/fileLib.h>

int CWebSocket::ws_service_callback(struct lws *wsi,
    enum lws_callback_reasons reason, void *user,
    void *in, size_t len)
{
    CWebSocket *pthis = (CWebSocket*)user;
    if (pthis)
    {
        pthis->m_Stat = reason;
    }
    logMessage1(LOGLEVEL_DEBUG,_T("dumb: == reason %d\n"),reason);
    switch (reason) {

    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        logMessage0(LOGLEVEL_DEBUG,_T("dumb: LWS_CALLBACK_CLIENT_ESTABLISHED\n"));
        if (pthis)
        {
            lws_callback_on_writable(wsi);
        }
        break;

    case LWS_CALLBACK_CLOSED:
        logMessage0(LOGLEVEL_DEBUG,_T("dumb: LWS_CALLBACK_CLOSED\n"));
        if (pthis)
        {
            pthis->m_force_exit = 1;
        }
        break;

    case LWS_CALLBACK_CLIENT_RECEIVE:
        logMessage0(LOGLEVEL_DEBUG,_T("LWS_CALLBACK_CLIENT_RECEIVE\n"));
        ((char *)in)[len] = '\0';
        if (pthis)
        {
            if (pthis->m_responsFun)
            {
                pthis->m_responsFun(in,len);
            }
            else
                pthis->saveData((char*)in, len);
        }
        //lwsl_info("rx %d '%s'\n", (int)len, (char *)in);
        break;

        /* because we are protocols[0] ... */

    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        logMessage1(LOGLEVEL_ERROR,_T("LWS_CALLBACK_CLIENT_CONNECTION_ERROR: %s\n"),in ? CSimpleA2T((char*)in,NTS) : _T("(null)"));
        if (pthis)
        {
            pthis->m_force_exit = 1;
            pthis->m_httpCompleted = 1;
        }
        break;

    case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
        logMessage0(LOGLEVEL_DEBUG,_T("LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED\n"));
        break;

    case LWS_CALLBACK_RECEIVE_CLIENT_HTTP:
        {
            logMessage(LOGLEVEL_DEBUG,_T("LWS_CALLBACK_RECEIVE_CLIENT_HTTP\n"));

            /*
            * Often you need to flow control this by something
            * else being writable.  In that case call the api
            * to get a callback when writable here, and do the
            * pending client read in the writeable callback of
            * the output.
            */
            if (pthis)
            {
                pthis->m_pWsi = wsi;
                char buffer[4096 + LWS_PRE]="";
                char *px = buffer + LWS_PRE;
                int lenx = sizeof(buffer) - LWS_PRE;
                if (lws_http_client_read(wsi, &px, &lenx) < 0)
                {
                    logMessage1(LOGLEVEL_DEBUG,_T("LWS_CALLBACK_RECEIVE_CLIENT_HTTP lws_http_client_read %d\n"),lenx);
                    pthis->m_force_exit = 1;
                    return -1;
                }
                logMessage1(LOGLEVEL_DEBUG,_T("LWS_CALLBACK_RECEIVE_CLIENT_HTTP lenx %d\n"),lenx);
            }
        }
        break;
    case LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ:
        logMessage1(LOGLEVEL_DEBUG,_T("LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ len %d\n"),len);
        if (pthis)
        {
            if (pthis->m_responsFun)
            {
                pthis->m_responsFun(in,len);
            }
            else
                pthis->saveData((char*)in, len);
        }
        break;
    case LWS_CALLBACK_CLIENT_WRITEABLE:
        logMessage0(LOGLEVEL_DEBUG,_T("LWS_CALLBACK_CLIENT_WRITEABLE\n"));
        if (pthis)
        {
            pthis->m_pWsi = wsi;
            pthis->m_sendlen = pthis->sendData();
            pthis->m_force_exit = 1;
        }
        break;
    case LWS_CALLBACK_COMPLETED_CLIENT_HTTP:
        logMessage0(LOGLEVEL_DEBUG,_T("LWS_CALLBACK_COMPLETED_CLIENT_HTTP\n"));
        if (pthis)
        {
            pthis->m_force_exit = 1;
            pthis->m_httpCompleted = 1;
        }
        break;
    case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
        logMessage0(LOGLEVEL_DEBUG,_T("LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER\n"));
        if (pthis)
        {
            pthis->m_pWsi = wsi;
            if (pthis->m_isPost)
            {
                unsigned char **p = (unsigned char **)in, *end = (*p) + len;

                std::map<std::string, std::string> &mapHeader = pthis->m_mapHeader;
                std::map<std::string, std::string>::iterator ite = mapHeader.begin();
                for (; ite != mapHeader.end(); ++ite)
                {
                    logMessage2(LOGLEVEL_DEBUG,_T("lws_add_http_header_by_name %s %s\n"),ite->first.c_str(),ite->second.c_str());
                    if (lws_add_http_header_by_name(wsi, (const unsigned char *)ite->first.c_str(), 
                        (const unsigned char *)ite->second.c_str(), ite->second.length(), p, end))
                    {
                        logMessage0(LOGLEVEL_ERROR,_T("!!!lws_add_http_header_by_name err\n"));
                        return -1;
                    }
                }
                lws_client_http_body_pending(wsi, 1);
                lws_callback_on_writable(wsi);
            }

        }
        break;
    case LWS_CALLBACK_CLIENT_HTTP_WRITEABLE:
        logMessage0(LOGLEVEL_DEBUG,_T("LWS_CALLBACK_CLIENT_HTTP_WRITEABLE\n"));
        if (pthis)
        {
            pthis->m_pWsi = wsi;
            pthis->m_sendlen = pthis->sendData();
            logMessage1(LOGLEVEL_DEBUG,_T("sendData pthis->m_sendlen %d\n"),pthis->m_sendlen);
            lws_client_http_body_pending(wsi, 0);
            pthis->m_force_exit = 1;
        }
        break;
    default:
        break;
    }

    return 0;
}

static struct lws_protocols protocols[] = {
    { "", CWebSocket::ws_service_callback, 0, 4096},
    { NULL, NULL, 0, 0 } /* end */
};

CWebSocket::CWebSocket()
{
    m_force_exit = 0;
    m_sendlen = 0;
    m_pContext = NULL;
    m_responsFun = NULL;
    m_pWsi = NULL;
    m_writeBuf = NULL;
    m_use_ssl = 0;
    m_httpCompleted = 0;
    m_mapHeader.clear();
    m_WsDataLen = 0;
    m_isPost = 0;
    m_pFile = NULL;
    memset(m_certPath,0,sizeof(m_certPath));
    memset(m_keyPath,0,sizeof(m_keyPath));
    memset(m_caPath,0,sizeof(m_caPath));
}

CWebSocket::~CWebSocket()
{
    m_force_exit = 1;
    if (m_handle)
    {
        join();
    }
    closeConnect();

}

void CWebSocket::closeConnect()
{
    if (m_pContext)
    {
        lws_context_destroy(m_pContext);
    }
    m_pContext = NULL;
    m_force_exit = 0;
    m_sendlen = 0;
    m_writeLen = 0;
    m_httpCompleted = 0;
    m_pWsi = NULL;
    m_mapHeader.clear();
    m_isPost  = 0;
    m_WsDataLen = 0;
    m_pFile = NULL;
    if (m_writeBuf)
    {
        delete m_writeBuf;
        m_writeBuf = NULL;
    }

    m_use_ssl = 0;
}

long CWebSocket::wsConnect(LPCSTR url)
{
    logMessage0(LOGLEVEL_DEBUG,_T("wsConnect"));
    closeConnect();

    const char *prot, *p;
    char path[300];
    struct lws_client_connect_info connectInfo;
    memset(&connectInfo, 0, sizeof(connectInfo));
    char temurl[1024];
    strncpy(temurl,url,sizeof(temurl));
    if (lws_parse_uri(temurl, &prot, &connectInfo.address, &connectInfo.port, &p))
    {
        logMessage0(LOGLEVEL_ERROR,_T("!wsConnect lws_parse_uri"));
        return -1;
    }
    logMessage4(LOGLEVEL_DEBUG,_T("wsConnect prot:%s address:%s port:%d p:%s"),prot,connectInfo.address,connectInfo.port,p);
    if (!strcmp(prot, "http") || !strcmp(prot, "ws"))
        m_use_ssl = 0;
    if (!strcmp(prot, "https") || !strcmp(prot, "wss"))
        if (!m_use_ssl)
            m_use_ssl = LCCSCF_ALLOW_SELFSIGNED |
            LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;

    path[0] = '/';
    strncpy(path + 1, p, sizeof(path) - 2);
    path[sizeof(path) - 1] = '\0';
    connectInfo.path = path;
    connectInfo.ssl_connection = m_use_ssl;
    connectInfo.host = connectInfo.address;
    connectInfo.origin = connectInfo.address;
    connectInfo.userdata = this;
    m_pContext = CreateContext();
    logMessage1(LOGLEVEL_DEBUG,_T("wsConnect CreateContext %p"),m_pContext);
    connectInfo.context = m_pContext;

    if (!strcmp(prot, "http") || !strcmp(prot, "https")) 
    {
        connectInfo.method = "GET";
    }

    m_pWsi = lws_client_connect_via_info(&connectInfo);
    logMessage0(LOGLEVEL_DEBUG,_T("wsConnect lws_client_connect_via_info"));
    if (m_responsFun)
    {
        //异步接收，创建接收线程
        start();
    }

    return 0;
}

void CWebSocket::svc()
{

    while (!m_force_exit)
    {
        lws_service(m_pContext, 500);
    }
}

lws_context* CWebSocket::CreateContext()
{
    //lws_set_log_level(0xFF, NULL);
    struct lws_context_creation_info tCreateinfo;	
    struct lws_context *pContext;
    memset(&tCreateinfo, 0, sizeof(tCreateinfo));

    tCreateinfo.port = CONTEXT_PORT_NO_LISTEN;
    tCreateinfo.protocols = protocols;
    tCreateinfo.gid = -1;
    tCreateinfo.uid = -1;

    if (m_use_ssl) 
    {
        tCreateinfo.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
        if (m_certPath[0])
            tCreateinfo.ssl_cert_filepath = m_certPath;
        if (m_keyPath[0])
            tCreateinfo.ssl_private_key_filepath = m_keyPath;
        if (m_caPath[0])
            tCreateinfo.ssl_ca_filepath = m_caPath;
    }

    pContext = lws_create_context(&tCreateinfo);
    return pContext;
}

void CWebSocket::wsSetCallBack(responsFun cbFun)
{
    m_responsFun = cbFun;
}

int CWebSocket::wsRead(LPCSTR buf, int len)
{
    logMessage0(LOGLEVEL_DEBUG,_T("wsRead"));
    int ret = 0;
    m_force_exit = 0;
    while (!m_force_exit && !m_httpCompleted)
    {
        lws_service(m_pContext, 500);
        //logMessage1(LOGLEVEL_DEBUG,"wsRead m_Stat %d ",m_Stat);
        if (m_Stat == LWS_CALLBACK_CLIENT_RECEIVE || m_Stat == LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ
            || m_Stat == LWS_CALLBACK_COMPLETED_CLIENT_HTTP || m_Stat == LWS_CALLBACK_WSI_DESTROY)
        {
            m_Stat = 0;
            int size = len < m_WsDataLen ? len : m_WsDataLen;
            memcpy((void*)buf, m_pWsData, size);
            ret = size;
            break;
        }
    }
    return ret;
}

int CWebSocket::wsWrite(LPCSTR buf, int len)
{
    logMessage0(LOGLEVEL_DEBUG,_T("wsWrite"));
    m_force_exit = 0;
    if (buf == NULL)
    {
        return -1;
    }

    if (m_writeBuf)
    {
        delete m_writeBuf;
        m_writeBuf = NULL;
        m_writeLen = 0;
    }
    m_writeBuf = new char[LWS_PRE+len+1];
    memset(m_writeBuf, 0, sizeof(m_writeBuf));
    memcpy(m_writeBuf+LWS_PRE, buf, len);
    m_writeLen = len;

    //lws_callback_on_writable(m_pWsi);

    if (!m_responsFun)
    {
        while (!m_force_exit)
        {
            lws_service(m_pContext, 500);
            //logMessage1(LOGLEVEL_DEBUG,"wsWrite m_Stat %d",m_Stat);
            if (m_Stat == LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER)
            {
                logMessage0(LOGLEVEL_DEBUG,_T("wsWrite LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER"));
                //lws_client_http_body_pending(m_pWsi, 1);
                //lws_callback_on_writable(m_pWsi);
            }
        }
        return m_sendlen;
    }
    return len;
}

int CWebSocket::sendData()
{
    if (m_writeBuf)
    {
        lws_write_protocol protocol = LWS_WRITE_TEXT;
        if (m_isPost)
        {
            protocol = LWS_WRITE_HTTP;
        }
        return lws_write(m_pWsi,(unsigned char*)m_writeBuf+LWS_PRE, m_writeLen, protocol);
    }
    return 0;
}

void CWebSocket::saveData(char* in, size_t len)
{
    memset(m_pWsData, 0, sizeof m_pWsData);
    int slen = len;
    if (slen > sizeof m_pWsData)
    {
        slen = sizeof m_pWsData;
    }
    memcpy(m_pWsData, in, slen);
    m_WsDataLen = slen;

    if (m_pFile)
    {
        logMessage1(LOGLEVEL_DEBUG,_T("CWebSocket::saveData %d"),len);
        fwrite(in,1,len,m_pFile);
    }
}

void CWebSocket::setSSLcert(LPCSTR certPath, LPCSTR keyPath, LPCSTR caPath)
{
    if (certPath)
    {
        strncpy(m_certPath, certPath, sizeof(m_certPath));
    }
    if (keyPath)
    {
        strncpy(m_keyPath, keyPath, sizeof(m_keyPath));
    }
    if (caPath)
    {
        strncpy(m_caPath, caPath, sizeof(m_caPath));
    }
}

int CWebSocket::wsAddHeader(LPCSTR key, LPCSTR value)
{
    logMessage2(LOGLEVEL_DEBUG,_T("wsAddHeader %s %s"),key,value);
    m_mapHeader[key] = value;
    return 0;
}

//int CWebSocket::wsDownloadFile(LPCSTR url, LPCSTR path)
//{
//	char filePath[MAX_PATH];
//	CStdString strUrl = url;
//	int pos = strUrl.rfind('/');
//	CStdString fileName = strUrl.substr(pos+1);
//	//if (!PathFileExists(path))
//	//{
//	//	return -1;
//	//}
//	strncpy(filePath, path,MAX_PATH);
//	PathAddBackslash(filePath);
//	fileEnsurePathExist(filePath);
//
//	strncpy(filePath+strlen(filePath),fileName.c_str(), MAX_PATH - strlen(filePath));
//
//	wsConnect("Get", url);
//	
//	m_pFile = fopen(filePath,"wb");
//	if (m_pFile == NULL)
//	{
//		return -1;
//	}
//
//	m_force_exit = 0;
//	while (!m_force_exit && !m_httpCompleted)
//	{
//		lws_service(m_pContext, 500);
//	}
//	fclose(m_pFile);
//	return 0;
//}

//int CWebSocket::wsUploadFile(LPCSTR url, LPCSTR filepath)
//{
//	return 0;
//}