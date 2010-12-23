/* <speeder>
 *
 */
#define UNICODE
#define THISMOD &k_speed

#include <windows.h>
#include <stdio.h>
#include <sys/stat.h>
#include "kload_exp.h"
#include "speeder.h"
#include "speeder_addr.h"
#include "dllinit.h"
#include "configs.h"
#include "configs.hpp"
#include "utf8.h"

#include "apihijack.h"

#define lang(s) getTransl("speeder",s)

//#include <map>
//#include <list>
//#include <hash_map>
//#include <wchar.h>

#define SWAPBYTES(dw) \
    ((dw<<24 & 0xff000000) | (dw<<8  & 0x00ff0000) | \
    (dw>>8  & 0x0000ff00) | (dw>>24 & 0x000000ff))

#define round(x) ((abs((x)-(int)(x))<0.5)?(int)(x):(int)((x)+1))

#define FLOAT_ZERO 0.0001f
#define MIN_COUNT 0.2
#define MAX_COUNT 2.5
#define DBG(x) {if (k_speed.debug) x;}

// VARIABLES
HINSTANCE hInst = NULL;
KMOD k_speed = {MODID, NAMELONG, NAMESHORT, DEFAULT_DEBUG};

// GLOBALS

class config_t 
{
public:
    config_t() : count_factor(1.0) {}
    float count_factor;
};

config_t _speeder_config;
bool _change(false);

// FUNCTIONS
HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface);

void speederConfig(char* pName, const void* pValue, DWORD a);
KEXPORT BOOL WINAPI Override_QueryPerformanceFrequency(
        LARGE_INTEGER *lpPerformanceFrequency);


/*******************/
/* DLL Entry Point */
/*******************/
EXTERN_C BOOL WINAPI DllMain(
        HINSTANCE hInstance, 
        DWORD dwReason, 
        LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		hInst = hInstance;

		RegisterKModule(THISMOD);
		hookFunction(hk_D3D_CreateDevice, initModule);
	}
	
	else if (dwReason == DLL_PROCESS_DETACH)
	{
	}
	
	return true;
}

void speederConfig(char* pName, const void* pValue, DWORD a)
{
    HANDLE handle;
    DWORD fsize;

	switch (a) {
		case 1: // debug
			k_speed.debug = *(DWORD*)pValue;
			break;
        case 2: // tick-count factor
			_speeder_config.count_factor = *(float*)pValue;
            // sanity checks
            if (_speeder_config.count_factor < MIN_COUNT)
                _speeder_config.count_factor = MIN_COUNT;
            if (_speeder_config.count_factor > MAX_COUNT)
                _speeder_config.count_factor = MAX_COUNT;
            break;
	}
	return;
}

HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface) {

	unhookFunction(hk_D3D_CreateDevice, initModule);

    LOG(L"Initializing Speeder Module");

    getConfig("speeder", "debug", DT_DWORD, 1, speederConfig);
    getConfig("speeder", "count.factor", DT_FLOAT, 2, speederConfig);

    LOG(L"configuration: count.factor = %0.3f",
                _speeder_config.count_factor);

    // patch code to enforce specified time
    if (_speeder_config.count_factor >= MIN_COUNT &&
            fabs(_speeder_config.count_factor-1.0) > FLOAT_ZERO) {
       SDLLHook Kernel32Hook = 
       {
          "KERNEL32.DLL",
          false, NULL,		// Default hook disabled, NULL function pointer.
          {
              { "QueryPerformanceFrequency", 
                  Override_QueryPerformanceFrequency },
              { NULL, NULL }
          }
       };
       HookAPICalls( &Kernel32Hook );

       LOG(L"speed adjustment enabled");

       LARGE_INTEGER metric;
       BOOL result = QueryPerformanceFrequency(&metric);
       LOG(L"old metric(%d, %d), result=%d", 
           metric.HighPart, metric.LowPart, result);
       metric.HighPart /= _speeder_config.count_factor;
       metric.LowPart /= _speeder_config.count_factor;
       LOG(L"new metric(%d, %d)", 
           metric.HighPart, metric.LowPart);
    }

	TRACE(L"Initialization complete.");
    return D3D_OK;
}

KEXPORT BOOL WINAPI Override_QueryPerformanceFrequency(
        LARGE_INTEGER *lpPerformanceFrequency)
{
    LARGE_INTEGER metric;
    BOOL result = QueryPerformanceFrequency(&metric);

    metric.HighPart /= _speeder_config.count_factor;
    metric.LowPart /= _speeder_config.count_factor;
    *lpPerformanceFrequency = metric;

    return result;
}

