/* <camera>
 *
 */
#define UNICODE
#define THISMOD &k_camera

#include <windows.h>
#include <stdio.h>
#include <sys/stat.h>
#include "kload_exp.h"
#include "camera.h"
#include "camera_addr.h"
#include "dllinit.h"
#include "configs.h"
#include "configs.hpp"
#include "utf8.h"

#define lang(s) getTransl("camera",s)

#include <map>
#include <list>
#include <hash_map>
#include <wchar.h>

#define SWAPBYTES(dw) \
    ((dw<<24 & 0xff000000) | (dw<<8  & 0x00ff0000) | \
    (dw>>8  & 0x0000ff00) | (dw>>24 & 0x000000ff))


// VARIABLES
HINSTANCE hInst = NULL;
KMOD k_camera = {MODID, NAMELONG, NAMESHORT, DEFAULT_DEBUG};

// GLOBALS
DWORD _angle = 0;

class config_t 
{
public:
    config_t() : angle(0) {}
    DWORD angle;
};

config_t _camera_config;

// FUNCTIONS
HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface);

void cameraConfig(char* pName, const void* pValue, DWORD a);
void cameraReadAngleCallPoint();

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
			k_camera.debug = *(DWORD*)pValue;
			break;
        case 2: // angle
			_camera_config.angle = *(DWORD*)pValue;
            _angle =_camera_config.angle;
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

    getConfig("camera", "debug", DT_DWORD, 1, fservConfig);
    getConfig("camera", "angle", DT_DWORD, 2, fservConfig);

    LOG1N(L"configuration: angle = %d",_camera_config.angle);

    // patch code to load an arbitrary camera angle
    if (_camera_config.angle!=0 && code[C_READ_CAMERA_ANGLE]!=0)
    {
        // check for demoTweak patch: if patched, we 
        // don't want to override it - too much grief :)
        // Instead: log a warning
        BYTE flag = *(BYTE*)code[C_READ_CAMERA_ANGLE];
        if (flag == 0xb9)
        {
            LOG(L"WARNING: demoTweak2010 camera patch detected.");
            LOG(L"Kitserver will not activate the camera.dll module, because");
            LOG(L"it will conflict with the patch. If you want to use the");
            LOG(L"camera.dll, you will need to run demoTweak2010 again");
            LOG(L"and reset the camera angle back to 0");
        }
        else
        {
            HookCallPoint(code[C_READ_CAMERA_ANGLE], 
                    cameraReadAngleCallPoint, 3, 1);
            LOG1N(L"Camera angle set to: %d",_camera_config.angle);
        }
    }
    else
        LOG(L"Using game-set camera angle");

	TRACE(L"Initialization complete.");
    return D3D_OK;
}

void cameraReadAngleCallPoint()
{
    __asm {
        mov ecx, _angle
        mov dword ptr ss:[ebp+0xcc], ecx
        retn
    }
}

