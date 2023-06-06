/* <time>
 *
 */
#define UNICODE
#define THISMOD &k_time

#include <windows.h>
#include <stdio.h>
#include <sys/stat.h>
#include "kload_exp.h"
#include "time.h"
#include "time_addr.h"
#include "dllinit.h"
#include "configs.h"
#include "configs.hpp"
#include "utf8.h"

#define lang(s) getTransl("time",s)

//#include <map>
//#include <list>
//#include <unordered_map>
//#include <wchar.h>

#define SWAPBYTES(dw) \
    ((dw<<24 & 0xff000000) | (dw<<8  & 0x00ff0000) | \
    (dw>>8  & 0x0000ff00) | (dw>>24 & 0x000000ff))

#define round(x) ((abs((x)-(int)(x))<0.5)?(int)(x):(int)((x)+1))

#define DBG(x) {if (k_time.debug) x;}

// VARIABLES
HINSTANCE hInst = NULL;
KMOD k_time = {MODID, NAMELONG, NAMESHORT, DEFAULT_DEBUG};

// GLOBALS

class config_t 
{
public:
    config_t() : match_time(0), stamina_factor(1.0) {}
    float stamina_factor;
    BYTE match_time;
};

config_t _time_config;

BYTE _savedValue = 0;
DWORD _src;
DWORD _dest;

// FUNCTIONS
HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface);

void timeConfig(char* pName, const void* pValue, DWORD a);
void timeReadCallPoint();
void timeRestoreCallPoint();
DWORD timeRead(BYTE* dest, BYTE* src);
DWORD timeRestore(BYTE* dest, BYTE* src);

/*******************/
/* DLL Entry Point */
/*******************/
EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		hInst = hInstance;

		RegisterKModule(THISMOD);
		
		if (!checkGameVersion()) {
			LOG(L"Sorry, your game version isn't supported!");
			return false;
		}

		copyAdresses();
		hookFunction(hk_D3D_CreateDevice, initModule);
	}
	
	else if (dwReason == DLL_PROCESS_DETACH)
	{
	}
	
	return true;
}

void fservConfig(char* pName, const void* pValue, DWORD a)
{
    HANDLE handle;
    DWORD fsize;

	switch (a) {
		case 1: // debug
			k_time.debug = *(DWORD*)pValue;
			break;
        case 2: // match time
			_time_config.match_time = *(DWORD*)pValue;
            break;
        case 3: // stamina factor
			_time_config.stamina_factor = *(float*)pValue;
            break;
	}
	return;
}

HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface) {

	unhookFunction(hk_D3D_CreateDevice, initModule);

    LOG(L"Initializing Camera Module");

    getConfig("time", "debug", DT_DWORD, 1, fservConfig);
    getConfig("time", "match.time", DT_DWORD, 2, fservConfig);
    getConfig("time", "stamina.factor", DT_FLOAT, 3, fservConfig);

    LOG1N(L"configuration: match.time = %d",_time_config.match_time);
    DBG(LOG1N(L"configuration: stamina.factor = %0.3f",
                _time_config.stamina_factor));

    // patch code to enforce specified time
    if (_time_config.match_time != 0)
    {
        HookCallPoint(code[C_READ_SETTINGS], 
                timeReadCallPoint, 6, 0);
        HookCallPoint(code[C_RESTORE_SETTINGS], 
                timeRestoreCallPoint, 6, 0);

        BYTE enduranceValue = max(1, 
            round(_time_config.match_time * _time_config.stamina_factor));
        DBG(LOG1N(L"enduranceValue = %d", enduranceValue));

        BYTE* bptr = (BYTE*)code[C_SET_CUP_ENDURANCE];
        DWORD protection = 0;
        DWORD newProtection = PAGE_EXECUTE_READWRITE;
        if (bptr && VirtualProtect(bptr, 8, newProtection, &protection)) 
        {
            bptr[0] = 0xb0;  // mov al, enduranceValue
            bptr[1] = enduranceValue;
            bptr[2] = 0x90;
            bptr[3] = 0xeb; // jmp short <to-use-al>
            bptr[4] = 0x20;
            bptr[5] = 0x90;
        }
    }

	TRACE(L"Initialization complete.");
    return D3D_OK;
}

void timeReadCallPoint()
{
    __asm {
        mov _src, esi
        mov _dest, edi
        rep movs dword ptr es:[edi], dword ptr ds:[esi]  // execute replaced
        mov ecx, dword ptr ds:[ebx+0x28]                 // code
        pushfd
        push eax
        push ebx
        push ecx
        push edx
        push edi
        push esi
        push ebp
        mov eax,_src
        push eax
        mov eax,_dest
        push eax
        call timeRead
        add esp,8  // pop args
        pop ebp
        pop esi
        pop edi
        pop edx
        pop ecx
        pop ebx
        pop eax
        popfd
        retn
    }
}

DWORD timeRead(BYTE* dest, BYTE* src)
{
    DBG(LOG2N(L"timeRead:: dest=%08x, src=%08x", (DWORD)dest, (DWORD)src));
    if ((DWORD)dest == data[SETTINGS_ADDR])
    {
        if (src[4]!=_time_config.match_time)
        {
            _savedValue = src[4];
            DBG(LOG1N(L"Saving value: %d", _savedValue));
        }
        dest[4] = _time_config.match_time;
    }
    return 0;
}

void timeRestoreCallPoint()
{
    __asm {
        mov ecx,0x22e   // execute replace code
        lea edi,dword ptr ss:[esp+0x10+4]
        pushfd
        push eax
        push ebx
        push ecx
        push edx
        push edi
        push esi
        push ebp
        push esi
        push edi
        call timeRestore
        add esp,8  // pop args
        pop ebp
        pop esi
        pop edi
        pop edx
        pop ecx
        pop ebx
        pop eax
        popfd
        retn
    }
}

DWORD timeRestore(BYTE* dest, BYTE* src)
{
    DBG(LOG2N(L"timeRestore:: dest=%08x, src=%08x", (DWORD)dest, (DWORD)src));
    if (_savedValue!=0)
    {
        DBG(LOG1N(L"Restoring saved value: %d", _savedValue));
        src[4] = _savedValue;
    }
    _savedValue = 0;
    return 0;
}

