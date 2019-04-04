#ifdef LPSEED_KEY_EXPORTS
#define LPSEED_KEY_API __declspec(dllexport)
#else
#define LPSEED_KEY_API __declspec(dllimport)
#endif

// 此类是从 LpSeed_key.dll 导出的
class LPSEED_KEY_API CLpSeed_key {
public:
	CLpSeed_key(void);
	// TODO:  在此添加您的方法。
	};

LPSEED_KEY_API UINT32 SeedKeyEXT_LP(UINT32 seed);

LPSEED_KEY_API UINT32 SeedKeyPROG_LP(UINT32 seed);

LPSEED_KEY_API UINT32 SeedKeyEXTSupplier(UINT32 seed);

LPSEED_KEY_API UINT32 SeedKeyPROGSupplier(UINT32 seed);


