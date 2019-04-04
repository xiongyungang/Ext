// ExtNet.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"


#include "libwebsockets.h"
#include <libFlow.h>
#include "WebSocket.h"
#include <string>


class CNetPlugin : public CFlowPlugin
{
protected:
				GM_DECLARE_METHOD(CNetPlugin,wsOpen);
				GM_DECLARE_METHOD(CNetPlugin,wsClose);
				//GM_DECLARE_METHOD(CNetPlugin,wsSetCallBackRead);
				GM_DECLARE_METHOD(CNetPlugin,wsConnect);
				GM_DECLARE_METHOD(CNetPlugin,wsWrite);
				GM_DECLARE_METHOD(CNetPlugin,wsRead);
				//GM_DECLARE_METHOD(CNetPlugin,wsAddHeader);
				//GM_DECLARE_METHOD(CNetPlugin,wsDownloadFile); //废弃

				GM_DECLARE_METHOD(CNetPlugin,httpOpen);
				GM_DECLARE_METHOD(CNetPlugin,httpClose);
				GM_DECLARE_METHOD(CNetPlugin,httpGet);
				GM_DECLARE_METHOD(CNetPlugin,httpPost);
				GM_DECLARE_METHOD(CNetPlugin,httpGetResponse);
				GM_DECLARE_METHOD(CNetPlugin,httpDownLoad);
				GM_DECLARE_METHOD(CNetPlugin,httpUpLoad);
				GM_DECLARE_METHOD(CNetPlugin,httpSetConnectTimeOut);

				GM_DECLARE_METHOD(CNetPlugin,httpRequest);

protected:
				long wsOpen();
				void wsClose(long handle);
				void wsSetCallBackRead(long handle, responsFun pCb);
				int wsConnect(long handle, LPCSTR url);
				int wsWrite(long handle, LPCSTR data);
				LPCSTR wsRead(long handle);
				//int wsAddHeader(long handle, LPCSTR key, LPCSTR value);
				//int wsDownloadFile(long handle, LPCSTR url, LPCSTR path);

				long httpOpen();
				void httpClose(long handle);
				int httpRequest(long handle,LPCSTR url,LPCSTR method = NULL,LPCSTR szData = NULL,LPCSTR szHeader = NULL,LPCSTR szFile = NULL);
				int httpGet(long handle, LPCSTR url);
				int httpPost(long handle, LPCSTR url, LPCSTR request);
				LPCSTR httpGetResponse(long handle);
				int httpDownLoad(long handle, LPCSTR url, LPCSTR path);
				int httpUpLoad(long handle, LPCSTR url, LPCSTR path, LPCSTR formName, LPCSTR formlist = NULL);
				void httpSetConnectTimeOut(long handle, int time);
				std::string  m_strResponse;

public:
				//插件名称
				virtual LPCTSTR GetName(void);

				//engine开始/插件加载事件
				virtual void OnEngineStart(CFlowEngine* pEngine);

				//显示函数统计
				virtual void showGlobalDelay(void);
};


#include "libhttpclient.h"
#include <curl/curl.h>

#pragma comment(lib,"zlib1")
#pragma comment(lib,"libeay32")
#pragma comment(lib,"ssleay32")
#pragma comment(lib,"websockets")
#pragma comment(lib,"libcurl")

#ifdef _UNICODE

#ifdef _DEBUG
#pragma comment(lib,"baseLibUD")
#pragma comment(lib,"flowcoreUD")
#pragma comment(lib,"luacoreUD")
#else
#pragma comment(lib,"baseLibUR")
#pragma comment(lib,"flowcoreU")
#pragma comment(lib,"luacoreU")
#endif

#else //!_UNICODE

#ifdef _DEBUG
#pragma comment(lib,"baseLibD")
#pragma comment(lib,"flowcoreD")
#pragma comment(lib,"luacoreD")
#else
#pragma comment(lib,"baseLibR")
#pragma comment(lib,"flowcore")
#pragma comment(lib,"luacore")
#endif

#endif //!_UNICODE

#ifdef _DEBUG
#pragma comment(lib,"syslogD")
#pragma comment(lib,"libScriptD")
#pragma comment(lib,"libhttpclientD")
#else
#pragma comment(lib,"syslog")
#pragma comment(lib,"libScriptR")
#pragma comment(lib,"libhttpclientR")
#endif

GD_IMPLEMENT(wsOpen)
				GD_IMPLEMENT(wsClose)
				//GD_IMPLEMENT(wsSetCallBackRead)
				GD_IMPLEMENT(wsConnect)
				GD_IMPLEMENT(wsWrite)
				GD_IMPLEMENT(wsRead)
				//GD_IMPLEMENT(wsAddHeader)
				//GD_IMPLEMENT(wsDownloadFile)
				GD_IMPLEMENT(httpOpen)
				GD_IMPLEMENT(httpClose)
				GD_IMPLEMENT(httpGet)
				GD_IMPLEMENT(httpPost)
				GD_IMPLEMENT(httpGetResponse)
				GD_IMPLEMENT(httpDownLoad)
				GD_IMPLEMENT(httpUpLoad)
				GD_IMPLEMENT(httpSetConnectTimeOut)
				GD_IMPLEMENT(httpRequest)

				extern "C"
{

				PFlowPlugin _PLUGIN_API_ getPlugin(DWORD magic,DWORD dwVersion)
				{
								static CNetPlugin m_nInstance;

								if (MAGIC_VMT != magic) return NULL;
								if (PLUGIN_VERSION_CURRENT != dwVersion) return NULL;
								return &m_nInstance;
				}

}

LPCTSTR CNetPlugin::GetName(void)
{
				return _T("ExtNet");
}

void CNetPlugin::OnEngineStart(CFlowEngine* pEngine)
{
				CFlowPlugin::OnEngineStart(pEngine);
				GM_REGISTER_METHOD(pEngine,this,wsOpen,NULL,"J"); 
				GM_REGISTER_METHOD(pEngine,this,wsClose,"J",NULL); 
				//GM_REGISTER_METHOD(pEngine,this,wsSetCallBackRead,"JJ",NULL);
				//wsConnect(long handle, string szMethod, string szUrl) handle从wsOpen()返回 szMethod "GET" or "POST" or ""(空字符表示ws) szUrl http/ws or https/wss
				GM_REGISTER_METHOD(pEngine,this,wsConnect,"JT","J"); 
				GM_REGISTER_METHOD(pEngine,this,wsWrite,"JT","J");  //websocket write
				GM_REGISTER_METHOD(pEngine,this,wsRead,"J","T");  //http read
				//GM_REGISTER_METHOD(pEngine,this,wsAddHeader,"JTT","J"); // wsAddHeader(long handle, string key, sting value) key "Content-Type" "charset"...
				//GM_REGISTER_METHOD(pEngine,this,wsDownloadFile,"JTT","J"); //wsDownloadFile(long handle, string szUrl, string filePath) 

				GM_REGISTER_METHOD(pEngine,this,httpOpen,NULL,"J");              //HANDLE httpOpen(VOID)
				GM_REGISTER_METHOD(pEngine,this,httpClose,"J",NULL);             //VOID httpClose(HANDLE handle)
				GM_REGISTER_METHOD(pEngine,this,httpGet,"JT","J");               //INTEGER httpGet(HANDLE handle,STRING szUrl)
				GM_REGISTER_METHOD(pEngine,this,httpPost,"JTT","J");             //INTEGER httpPost(HANDLE handle,STRING szUrl,STRING data)
				GM_REGISTER_METHOD(pEngine,this,httpGetResponse,"J","T");        //STRING httpGetResponse(HANDLE handle)
				GM_REGISTER_METHOD(pEngine,this,httpDownLoad,"JTT","J");         //INTEGER httpDownLoad(HANDLE handle,STRING szUrl,STRING path)
				GM_REGISTER_METHOD(pEngine,this,httpUpLoad,"JTTT!T","J");        //INTEGER httpUpLoad(HANDLE handle,STRING szUrl,STRING path,STRING formName,STRING formlist)
				GM_REGISTER_METHOD(pEngine,this,httpSetConnectTimeOut,"JJ",NULL);//VOID httpSetConnectTimeOut(HANDLE handle,INTEGER timeOut)
				GM_REGISTER_METHOD(pEngine,this,httpRequest,"J!TTTTT","J");      //INTEGER httpRequest(INTEGER handle,STRING szUrl,STRING szMethod=NULL,STRING data=NULL,STRING header=NULL,STRING savefile=NULL)
}

void CNetPlugin::showGlobalDelay(void)
{
				GD_PRINTEX(wsOpen)
								GD_PRINTEX(wsClose)
								//GD_PRINTEX(wsSetCallBackRead)
								GD_PRINTEX(wsConnect)
								GD_PRINTEX(wsWrite)
								GD_PRINTEX(wsRead)
								//GD_PRINTEX(wsAddHeader)
								//GD_PRINTEX(wsDownloadFile)
								GD_PRINTEX(httpOpen)
								GD_PRINTEX(httpClose)
								GD_PRINTEX(httpGet)
								GD_PRINTEX(httpPost)
								GD_PRINTEX(httpGetResponse)
								GD_PRINTEX(httpDownLoad)
								GD_PRINTEX(httpUpLoad)
								GD_PRINTEX(httpSetConnectTimeOut)
}

GM_METHOD_LD_BEGIN(CNetPlugin,wsOpen)
				DEBUG_BREAK_IF(0 != numIn);
pOut[0].SetI4(pGmc->wsOpen());
GM_METHOD_LD_END()

				GM_METHOD_LD_BEGIN(CNetPlugin,wsClose)
				DEBUG_BREAK_IF(1 != numIn);
DEBUG_BREAK_IF(!GV_IS_I4(pIn[0]));
pGmc->wsClose(pIn[0].lVal);
GM_METHOD_LD_END()

				//GM_METHOD_LD_BEGIN(CNetPlugin,wsSetCallBackRead)
				//	DEBUG_BREAK_IF(2 != numIn);
				//	DEBUG_BREAK_IF(!GV_IS_I4(pIn[0]));
				//	DEBUG_BREAK_IF(!GV_IS_I4(pIn[1]));
				//	DEBUG_BREAK_IF(1 != pGV->m_strRet.length());
				//	pGmc->wsSetCallBackRead(pIn[0].lVal,(responsFun)pIn[1].lVal);
				//GM_METHOD_LD_END()

				GM_METHOD_LD_BEGIN(CNetPlugin,wsConnect)
				DEBUG_BREAK_IF(2 != numIn);
DEBUG_BREAK_IF(!GV_IS_I4(pIn[0]));
DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[1]));
pOut[0].SetI4(pGmc->wsConnect(pIn[0].lVal, pIn[1].strVal));
GM_METHOD_LD_END()

				GM_METHOD_LD_BEGIN(CNetPlugin,wsWrite)
				DEBUG_BREAK_IF(2 != numIn);
DEBUG_BREAK_IF(!GV_IS_I4(pIn[0]));
DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[1]));
DEBUG_BREAK_IF(1 != pGV->m_strRet.length());
pOut[0].SetI4(pGmc->wsWrite(pIn[0].lVal, pIn[1].strVal));
GM_METHOD_LD_END()

				GM_METHOD_LD_BEGIN(CNetPlugin,wsRead)
				DEBUG_BREAK_IF(1 != numIn);
DEBUG_BREAK_IF(!GV_IS_I4(pIn[0]));
DEBUG_BREAK_IF(1 != pGV->m_strRet.length());
pOut[0].SetBSTR(pGmc->wsRead(pIn[0].lVal));
GM_METHOD_LD_END()

				//GM_METHOD_LD_BEGIN(CNetPlugin,wsAddHeader)
				//	DEBUG_BREAK_IF(3 != numIn);
				//	DEBUG_BREAK_IF(!GV_IS_I4(pIn[0]));
				//	DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[1]));
				//	DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[2]));
				//	DEBUG_BREAK_IF(1 != pGV->m_strRet.length());
				//	CStdString retStr;
				//	retStr = pGmc->wsAddHeader(pIn[0].lVal,pIn[1].strVal,pIn[2].strVal);
				//	pOut[0].SetBSTR(retStr.c_str(), retStr.length());
				//GM_METHOD_LD_END()
				//
				//GM_METHOD_LD_BEGIN(CNetPlugin,wsDownloadFile)
				//	DEBUG_BREAK_IF(3 != numIn);
				//	DEBUG_BREAK_IF(!GV_IS_I4(pIn[0]));
				//	DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[1]));
				//	DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[2]));
				//	DEBUG_BREAK_IF(1 != pGV->m_strRet.length());
				//	CStdString retStr;
				//	retStr = pGmc->wsDownloadFile(pIn[0].lVal,pIn[1].strVal,pIn[2].strVal);
				//	pOut[0].SetBSTR(retStr.c_str(), retStr.length());
				//GM_METHOD_LD_END()

				GM_METHOD_LD_BEGIN(CNetPlugin,httpOpen)
				DEBUG_BREAK_IF(0 != numIn);
DEBUG_BREAK_IF(1 != pGV->m_strRet.length());
pOut[0].SetI4(pGmc->httpOpen());
GM_METHOD_LD_END()

				GM_METHOD_LD_BEGIN(CNetPlugin,httpClose)
				DEBUG_BREAK_IF(1 != numIn);
DEBUG_BREAK_IF(!GV_IS_I4(pIn[0]));
DEBUG_BREAK_IF(0 != pGV->m_strRet.length());
pGmc->httpClose(pIn[0].lVal);
GM_METHOD_LD_END()

				GM_METHOD_LD_BEGIN(CNetPlugin,httpGet)
				DEBUG_BREAK_IF(2 != numIn);
DEBUG_BREAK_IF(!GV_IS_I4(pIn[0]));
DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[1]));
DEBUG_BREAK_IF(1 != pGV->m_strRet.length());
pOut[0].SetI4(pGmc->httpGet(pIn[0].lVal, pIn[1].strVal));
GM_METHOD_LD_END()

				GM_METHOD_LD_BEGIN(CNetPlugin,httpPost)
				DEBUG_BREAK_IF(3 != numIn);
DEBUG_BREAK_IF(!GV_IS_I4(pIn[0]));
DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[1]));
DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[2]));
DEBUG_BREAK_IF(1 != pGV->m_strRet.length());
pOut[0].SetI4(pGmc->httpPost(pIn[0].lVal, pIn[1].strVal, pIn[2].strVal));
GM_METHOD_LD_END()

				GM_METHOD_LD_BEGIN(CNetPlugin,httpGetResponse)
				DEBUG_BREAK_IF(1 != numIn);
DEBUG_BREAK_IF(!GV_IS_I4(pIn[0]));
DEBUG_BREAK_IF(1 != pGV->m_strRet.length());
pOut[0].SetBSTR(pGmc->httpGetResponse(pIn[0].lVal));
GM_METHOD_LD_END()

				GM_METHOD_LD_BEGIN(CNetPlugin,httpDownLoad)
				DEBUG_BREAK_IF(3 != numIn);
DEBUG_BREAK_IF(!GV_IS_I4(pIn[0]));
DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[1]));
DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[2]));
DEBUG_BREAK_IF(0 != pGV->m_strRet.length());
pOut[0].SetI4(pGmc->httpDownLoad(pIn[0].lVal, pIn[1].strVal, pIn[2].strVal));
GM_METHOD_LD_END()

				GM_METHOD_LD_BEGIN(CNetPlugin,httpUpLoad)
				DEBUG_BREAK_IF(numIn > 5);
DEBUG_BREAK_IF(!GV_IS_I4(pIn[0]));
DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[1]));
DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[2]));
DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[3]));
DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[4]));
DEBUG_BREAK_IF(0 != pGV->m_strRet.length());
int retCode;
logMessage1(LOGLEVEL_DEBUG,_T("CHttpClient m_numIn(%d)\n"),numIn);
switch(numIn)
{
case 4:
				retCode = pGmc->httpUpLoad(pIn[0].lVal, pIn[1].strVal, 
								pIn[2].strVal, pIn[3].strVal);
				break;
case 5:
				retCode = pGmc->httpUpLoad(pIn[0].lVal, pIn[1].strVal, 
								pIn[2].strVal, pIn[3].strVal, pIn[4].strVal);
				break;
default:
				DEBUG_BREAK();
				break;
}

pOut[0].SetI4(retCode);
GM_METHOD_LD_END()

				GM_METHOD_LD_BEGIN(CNetPlugin,httpSetConnectTimeOut)
				DEBUG_BREAK_IF(2 != numIn);
DEBUG_BREAK_IF(!GV_IS_I4(pIn[0]));
DEBUG_BREAK_IF(0 != pGV->m_strRet.length());
pGmc->httpSetConnectTimeOut(pIn[0].lVal, pIn[1].lVal);
GM_METHOD_LD_END()

				GM_METHOD_LD_BEGIN(CNetPlugin,httpRequest)
				long retCode;
DEBUG_BREAK_IF(numIn < 2);
DEBUG_BREAK_IF(!GV_IS_I4(pIn[0]));
DEBUG_BREAK_IF(1 != pGV->m_strRet.length());
switch(numIn)
{
case 2:
				DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[1]));
				retCode = pGmc->httpRequest(pIn[0].lVal,pIn[1].strVal,NULL,NULL,NULL,NULL);
				break;
case 3:
				DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[1]));
				DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[2]));
				retCode = pGmc->httpRequest(pIn[0].lVal,pIn[1].strVal,pIn[2].strVal,NULL,NULL,NULL);
				break;
case 4:
				DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[1]));
				DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[2]));
				DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[3]));
				retCode = pGmc->httpRequest(pIn[0].lVal,pIn[1].strVal,pIn[2].strVal,pIn[3].strVal,NULL,NULL);
				break;
case 5:
				DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[1]));
				DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[2]));
				DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[3]));
				DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[4]));
				retCode = pGmc->httpRequest(pIn[0].lVal,pIn[1].strVal,pIn[2].strVal,pIn[3].strVal,pIn[4].strVal,NULL);
				break;
case 6:
				DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[1]));
				DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[2]));
				DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[3]));
				DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[4]));
				DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[5]));
				retCode = pGmc->httpRequest(pIn[0].lVal,pIn[1].strVal,pIn[2].strVal,pIn[3].strVal,pIn[4].strVal,pIn[5].strVal);
				break;
default:
				retCode = -1;
				break;
}
pOut[0].SetI4(retCode);
GM_METHOD_LD_END()

				long CNetPlugin::wsOpen()
{
				CWebSocket* pWs = new CWebSocket();
				return (long)pWs;
}

void CNetPlugin::wsClose(long handle)
{
				if (handle != NULL)
				{
								delete (CWebSocket*)handle;;
				}
}

void CNetPlugin::wsSetCallBackRead(long handle, responsFun pCb)
{
				if (handle != NULL)
				{
								CWebSocket* pWs = (CWebSocket*)handle;
								pWs->wsSetCallBack(pCb);
				}
}

int CNetPlugin::wsConnect(long handle, LPCSTR szUrl)
{
				if (handle != NULL)
				{
								CWebSocket* pWs = (CWebSocket*)handle;
								return pWs->wsConnect(szUrl);
				}
				return -1;
}

int CNetPlugin::wsWrite(long handle, LPCSTR data)
{
				if (handle != NULL)
				{
								CWebSocket* pWs = (CWebSocket*)handle;
								int len = strlen(data);
								return pWs->wsWrite((char*)data, len);
				}
				return -1;
}

LPCSTR CNetPlugin::wsRead(long handle)
{
				if (handle != NULL)
				{
								CWebSocket* pWs = (CWebSocket*)handle;
								static char buf[4097] = "";
								pWs->wsRead(buf, sizeof(buf) - 1);
								return buf;
				}
				return "";
}

//int CNetPlugin::wsAddHeader(long handle, LPCSTR key, LPCSTR value)
//{
//	if (handle != NULL)
//	{
//		CWebSocket* pWs = (CWebSocket*)handle;
//		return pWs->wsAddHeader(key, value);
//	}
//	return -1;
//}
//
//int CNetPlugin::wsDownloadFile(long handle, LPCSTR szUrl, LPCSTR path)
//{
//	if (handle != NULL)
//	{
//		CWebSocket* pWs = (CWebSocket*)handle;
//		return pWs->wsDownloadFile(szUrl, path);
//	}
//	return -1;
//}

long CNetPlugin::httpOpen()
{
				CHttpClient* pHttp = new CHttpClient();
				pHttp->SetDebug(true);
				return (long)pHttp;
}

void CNetPlugin::httpClose(long handle)
{
				if (handle != NULL)
				{
								delete (CHttpClient*)handle;;
				}
}

int CNetPlugin::httpGet(long handle, LPCSTR szUrl)
{
				if (handle != NULL)
				{
								CHttpClient* pHttp = (CHttpClient*)handle;
								m_strResponse.clear();
								if (strncmp("https", szUrl, 5) == 0)
								{
												return pHttp->Gets(szUrl,m_strResponse);
								}
								else if (strncmp("http", szUrl, 4) == 0)
								{
												return pHttp->Get(szUrl,m_strResponse);
								}
				}
				return -1;
}

int CNetPlugin::httpPost(long handle, LPCSTR szUrl, LPCSTR request)
{
				if (handle != NULL)
				{
								CHttpClient* pHttp = (CHttpClient*)handle;
								m_strResponse.clear();
								if (strncmp("https", szUrl, 5) == 0)
								{
												return pHttp->Posts(szUrl,request,m_strResponse);
								}
								else if (strncmp("http", szUrl, 4) == 0)
								{
												return pHttp->Post(szUrl,request,m_strResponse);
								}
				}
				return -1;
}

LPCSTR CNetPlugin::httpGetResponse(long handle)
{
				if (handle != NULL)
				{
								CHttpClient* pHttp = (CHttpClient*)handle;
								return m_strResponse.c_str();
				}
				return "";
}

int CNetPlugin::httpDownLoad(long handle, LPCSTR szUrl, LPCSTR path)
{
				if (handle != NULL)
				{
								CHttpClient* pHttp = (CHttpClient*)handle;
								if (strncmp("https", szUrl, 5) == 0)
								{
												return pHttp->Donwloads(szUrl,path);
								}
								else if (strncmp("http", szUrl, 4) == 0)
								{
												return pHttp->Donwload(szUrl,path);
								}
				}
				return -1;
}

int CNetPlugin::httpUpLoad(long handle, LPCSTR szUrl, LPCSTR path, LPCSTR formName, LPCSTR formlist)
{
				if (handle != NULL)
				{
								CHttpClient* pHttp = (CHttpClient*)handle;
								if (strncmp("https", szUrl, 5) == 0)
								{
												return pHttp->Uploads(szUrl,path,formName,m_strResponse,formlist);
								}
								else if (strncmp("http", szUrl, 4) == 0)
								{
												return pHttp->Upload(szUrl,path,formName,m_strResponse,formlist);
								}
				}
				return -1;
}

void CNetPlugin::httpSetConnectTimeOut(long handle, int time)
{
				if (handle != NULL)
				{
								CHttpClient* pHttp = (CHttpClient*)handle;
								pHttp->SetConnectTimeOut(time);
				}
}

size_t WriteStdString(char* buffer, size_t size, size_t nmemb, std::string* str)
{
				if( NULL == str || NULL == buffer )
				{
								return -1;
				}
				*str = buffer;    
				return nmemb;
}

size_t WriteFILE(void* buffer, size_t size, size_t nmemb, FILE * fp)
{
				if( NULL == fp || NULL == buffer )
				{
								return -1;
				}
				return fwrite((char*)buffer,size,nmemb,fp);
}

int CNetPlugin::httpRequest(long handle,LPCSTR szUrl,LPCSTR szMethod,LPCSTR szData,LPCSTR szHeader,LPCSTR szFile)
{
				if (handle != NULL)
				{
								CHttpClient* pHttp = (CHttpClient*)handle;

								CURLcode res;
								FILE *pFile;

								m_strResponse.clear();
								CURL* curl = curl_easy_init();
								if(NULL == curl)
								{
												return CURLE_FAILED_INIT;
								}
								/*
								if(m_bDebug)
								{
								curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
								curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
								}
								*/
								curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);

								struct curl_slist *head = NULL;
								if (!strnull(szData))
								{
												if (strnull(szMethod))
												{
																szMethod = "POST";
												}
												if (strnull(szHeader))
												{
																head = curl_slist_append(head,"Content-Type: application/x-www-form-urlencoded");
												}
												else
												{
																head = curl_slist_append(head,szHeader);
												}
								}
								else
								{
												if (!strnull(szMethod)
																&& 0 != stricmp(szMethod,"GET"))
												{
																szMethod = "GET";
												}
								}

								//不发送 Expect: 100-continue
								head = curl_slist_append(head, "Expect:");
								curl_easy_setopt(curl, CURLOPT_HTTPHEADER, head);  
								curl_easy_setopt(curl, CURLOPT_URL, szUrl);

								if (0 == stricmp(szMethod,"POST"))
								{
												curl_easy_setopt(curl, CURLOPT_POST, 1);
												curl_easy_setopt(curl, CURLOPT_POSTFIELDS, szData);
								}
								curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);

								pFile = NULL;
								if (strnull(szFile))
								{
												curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteStdString);
												curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&m_strResponse);
								}
								else
								{
												pFile = fopen(szFile,"wb");
												if (NULL == pFile)
												{
																logMessage0(LOGLEVEL_ERROR,_T("CHttpClient::httpRequest fopen err\n"));
																return -1;
												}
												curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFILE);
												curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)(FILE*)pFile);
								}

								//if(NULL == pCaPath)
								{
												curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
												curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
								}
								/*
								else
								{
								//缺省情况就是PEM，所以无需设置，另外支持DER
								//curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
								curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
								curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
								}
								*/
								/**
								* 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
								* 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
								*/
								curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
								curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, pHttp->m_iConnectTimeOut);//连接超时，这个数值如果设置太短可能导致数据请求不到就断开了
								//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);//接收数据时超时设置，如果10秒内数据未接收完，直接退出
								res = curl_easy_perform(curl);
								logMessage1(LOGLEVEL_ERROR,_T("CHttpClient::httpRequest %s\n"),curl_easy_strerror(res));
								long res_code=0;
								res=curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res_code);
								curl_slist_free_all(head);
								curl_easy_cleanup(curl);
								if (NULL != pFile)
								{
												fclose(pFile);
								}
								return res_code;
				}
				return -1;
}
