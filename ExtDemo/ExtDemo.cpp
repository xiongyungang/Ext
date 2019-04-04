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

//用来保存Demo_println函数的总的调用时间
GD_IMPLEMENT(Demo_println)

class CDemoPlugin : public CFlowPlugin
{
public:
  //插件名称
  virtual LPCTSTR GetName(void);

  //engine开始/插件加载事件
  virtual void OnEngineStart(PFlowEngine pEngine);

  //engine全部插件已加载事件
  virtual void OnEngineStarted(PFlowEngine pEngine);

  //模型加载事件
  virtual void OnLoadModel(PFlowEngine pEngine);
  
  //thread开始事件
  virtual CPluginThread* OnThreadStart(PFlowThread pThread);
 
  //节点进入事件
  virtual void OnActivityEnter(PFlowThread pThread,CPluginThread *pData);
 
  //节点离开事件
  virtual void OnActivityLeave(PFlowThread pThread,CPluginThread *pData);

  //thread结束事件
  virtual void OnThreadStop(PFlowThread pThread,CPluginThread *pData);
 
  //模型卸载事件
  virtual void OnUnloadModel(PFlowEngine pEngine);
 
  //engine结束/插件卸载事件
  virtual void OnEngineStop(PFlowEngine pEngine);

  //显示函数统计
  virtual void showGlobalDelay(void);

protected:
  CStdString m_strName;

protected:
  //定义属性和方法
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
  //注册属性和方法
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

//打印所有方法的调用时间
void CDemoPlugin::showGlobalDelay(void)
{
  GD_PRINT(Demo_println)
}

//实现属性的读写
//STRING Demo_name
GM_PROPERTY_BSTR_IMPLEMENT(CDemoPlugin,Demo_name,pGmc->m_strName)

//实现方法调用
//VOID Demo_println(STRING sText)
GM_METHOD_LD_BEGIN(CDemoPlugin,Demo_println)
  //DEBUG_BREAK/DEBUG_BREAK_IF宏只在调试版本有效,用来在调试状态无条件/有条件地进入断点
  DEBUG_BREAK_IF(1 != numIn);
  DEBUG_BREAK_IF(!GV_IS_BSTR(pIn[0]));
  logMessage1A(LOGLEVEL_ERROR,"%s",pIn[0].strVal);
GM_METHOD_LD_END()
