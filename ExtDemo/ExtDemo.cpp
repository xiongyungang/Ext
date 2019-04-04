// ExtGdi.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

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

//��������Demo_println�������ܵĵ���ʱ��
GD_IMPLEMENT(Demo_println)

class CDemoPlugin : public CFlowPlugin
{
public:
  //�������
  virtual LPCTSTR GetName(void);

  //engine��ʼ/��������¼�
  virtual void OnEngineStart(PFlowEngine pEngine);

  //engineȫ������Ѽ����¼�
  virtual void OnEngineStarted(PFlowEngine pEngine);

  //ģ�ͼ����¼�
  virtual void OnLoadModel(PFlowEngine pEngine);
  
  //thread��ʼ�¼�
  virtual CPluginThread* OnThreadStart(PFlowThread pThread);
 
  //�ڵ�����¼�
  virtual void OnActivityEnter(PFlowThread pThread,CPluginThread *pData);
 
  //�ڵ��뿪�¼�
  virtual void OnActivityLeave(PFlowThread pThread,CPluginThread *pData);

  //thread�����¼�
  virtual void OnThreadStop(PFlowThread pThread,CPluginThread *pData);
 
  //ģ��ж���¼�
  virtual void OnUnloadModel(PFlowEngine pEngine);
 
  //engine����/���ж���¼�
  virtual void OnEngineStop(PFlowEngine pEngine);

  //��ʾ����ͳ��
  virtual void showGlobalDelay(void);

protected:
  CStdString m_strName;

protected:
  //�������Ժͷ���
  GM_DECLARE_PROPERTY(CDemoPlugin,Demo_name);
  GM_DECLARE_METHOD(CDemoPlugin,Demo_println);
};

extern "C"
{
PFlowPlugin _PLUGIN_API_ getPlugin(DWORD magic,DWORD dwVersion)
{
static CDemoPlugin m_nInstance;

  if (PLUGIN_VERSION_CURRENT != dwVersion) return NULL;
  return &m_nInstance;
}
}

LPCTSTR CDemoPlugin::GetName(void)
{
  return _T("ExtDemo");
}

void CDemoPlugin::OnEngineStart(CFlowEngine *pEngine)
{
  CFlowPlugin::OnEngineStart(pEngine);
  //ע�����Ժͷ���
  GM_REGISTER_PROPERTY(pEngine,this,Demo_name,"T");       //STRING Demo_name
  GM_REGISTER_METHOD(pEngine,this,Demo_println,"T",NULL); //VOID Demo_println(STRING sText)
}

void CDemoPlugin::OnEngineStarted(CFlowEngine *pEngine)
{
}

void CDemoPlugin::OnLoadModel(PFlowEngine pEngine)
{
}

CPluginThread* CDemoPlugin::OnThreadStart(CFlowThread *pThread)
{
  return NULL;
}

void CDemoPlugin::OnActivityEnter(PFlowThread pThread,CPluginThread *pData)
{
}
 
void CDemoPlugin::OnActivityLeave(PFlowThread pThread,CPluginThread *pData)
{
}

void CDemoPlugin::OnThreadStop(CFlowThread *pThread,CPluginThread *pData)
{
}

void CDemoPlugin::OnUnloadModel(PFlowEngine pEngine)
{
}

void CDemoPlugin::OnEngineStop(CFlowEngine *pEngine)
{
}

//��ӡ���з����ĵ���ʱ��
void CDemoPlugin::showGlobalDelay(void)
{
  GD_PRINT(Demo_println)
}

//ʵ�����ԵĶ�д
//STRING Demo_name
GM_PROPERTY_BSTR_IMPLEMENT(CDemoPlugin,Demo_name,pGmc->m_strName)

//ʵ�ַ�������
//VOID Demo_println(STRING sText)
GM_METHOD_LD_BEGIN(CDemoPlugin,Demo_println)
  //DEBUG_BREAK/DEBUG_BREAK_IF��ֻ�ڵ��԰汾��Ч,�����ڵ���״̬������/�������ؽ���ϵ�
  DEBUG_BREAK_IF(1 != numIn);
  DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[0]));
  logMessage1A(LOGLEVEL_ERROR,"%s",pIn[0].strVal);
GM_METHOD_LD_END()
