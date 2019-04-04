#include "stdafx.h"
#include <libFlow.h>
#include "LpSeed_key.h"

class CSeedKeyPlugin : public CFlowPlugin
{
public:
		//插件名称
		virtual LPCTSTR GetName(void);
		//engine开始/插件加载事件
		virtual void OnEngineStart(PFlowEngine pEngine);
		//显示函数统计
		virtual void showGlobalDelay(void);

protected:
		GM_DECLARE_METHOD(CSeedKeyPlugin,SeedKeyEXT_LP);
		GM_DECLARE_METHOD(CSeedKeyPlugin,SeedKeyPROG_LP);
		GM_DECLARE_METHOD(CSeedKeyPlugin,SeedKeyEXTSupplier);
		GM_DECLARE_METHOD(CSeedKeyPlugin,SeedKeyPROGSupplier);
};


#pragma comment(lib,"zlib1")
#pragma comment(lib,"libeay32")
#pragma comment(lib,"ssleay32")
#pragma comment(lib,"LpSeed_key")

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

GD_IMPLEMENT(SeedKeyEXT_LP)
GD_IMPLEMENT(SeedKeyPROG_LP)
GD_IMPLEMENT(SeedKeyEXTSupplier)
GD_IMPLEMENT(SeedKeyPROGSupplier)

extern "C"
{

PFlowPlugin _PLUGIN_API_ getPlugin(DWORD magic,DWORD dwVersion)
	{
	static CSeedKeyPlugin m_nInstance;

	if (MAGIC_VMT != magic) return NULL;
	if (PLUGIN_VERSION_CURRENT != dwVersion) return NULL;
	return &m_nInstance;
	}

}

LPCTSTR CSeedKeyPlugin::GetName(void)
{
	return _T("ExtSeedKey");
}

void CSeedKeyPlugin::OnEngineStart(CFlowEngine* pEngine)
{
	CFlowPlugin::OnEngineStart(pEngine);
	GM_REGISTER_METHOD(pEngine,this,SeedKeyEXT_LP,"I","I");
	GM_REGISTER_METHOD(pEngine,this,SeedKeyPROG_LP,"I","I");
	GM_REGISTER_METHOD(pEngine,this,SeedKeyEXTSupplier,"I","I");
	GM_REGISTER_METHOD(pEngine,this,SeedKeyPROGSupplier,"I","I");
}

void CSeedKeyPlugin::showGlobalDelay(void)
{
	GD_PRINTEX(SeedKeyEXT_LP)
	GD_PRINTEX(SeedKeyPROG_LP)
	GD_PRINTEX(SeedKeyEXTSupplier)
	GD_PRINTEX(SeedKeyPROGSupplier)
}

GM_METHOD_LD_BEGIN(CSeedKeyPlugin,SeedKeyEXT_LP)
	DEBUG_BREAK_IF(1 != numIn);
	DEBUG_BREAK_IF(1 != pGV->m_strRet.length());
	SeedKeyEXT_LP(pIn[0].lVal);
GM_METHOD_LD_END()

GM_METHOD_LD_BEGIN(CSeedKeyPlugin,SeedKeyPROG_LP)
	DEBUG_BREAK_IF(1 != numIn);
	DEBUG_BREAK_IF(1 != pGV->m_strRet.length());
	SeedKeyPROG_LP(pIn[0].lVal);
GM_METHOD_LD_END()

GM_METHOD_LD_BEGIN(CSeedKeyPlugin,SeedKeyEXTSupplier)
	DEBUG_BREAK_IF(1 != numIn);
	DEBUG_BREAK_IF(1 != pGV->m_strRet.length());
	SeedKeyEXTSupplier(pIn[0].lVal);
GM_METHOD_LD_END()

GM_METHOD_LD_BEGIN(CSeedKeyPlugin,SeedKeyPROGSupplier)
	DEBUG_BREAK_IF(1 != numIn);
	DEBUG_BREAK_IF(1 != pGV->m_strRet.length());
	SeedKeyPROGSupplier(pIn[0].lVal);
GM_METHOD_LD_END()

