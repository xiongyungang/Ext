#ifdef LPSEED_KEY_EXPORTS
#define LPSEED_KEY_API __declspec(dllexport)
#else
#define LPSEED_KEY_API __declspec(dllimport)
#endif

// �����Ǵ� LpSeed_key.dll ������
class LPSEED_KEY_API CLpSeed_key {
public:
	CLpSeed_key(void);
	// TODO:  �ڴ�������ķ�����
	};

LPSEED_KEY_API UINT32 SeedKeyEXT_LP(UINT32 seed);

LPSEED_KEY_API UINT32 SeedKeyPROG_LP(UINT32 seed);

LPSEED_KEY_API UINT32 SeedKeyEXTSupplier(UINT32 seed);

LPSEED_KEY_API UINT32 SeedKeyPROGSupplier(UINT32 seed);


