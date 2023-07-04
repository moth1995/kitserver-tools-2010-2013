/* Kserv module */
#define UNICODE
#define THISMOD &k_kserv

#include <windows.h>
#include <stdio.h>
#include "kload_exp.h"
#include "teaminfo.h"
#include "gdb.h"
#include "kserv.h"
#include "kserv_addr.h"
#include "dllinit.h"
#include "pngdib.h"
#include "utf8.h"
#include "commctrl.h"
#include "afsio.h"
#include "afsreader.h"
#include "soft\zlib123-dll\include\zlib.h"

#if _CPPLIB_VER < 503
#define  __in
#define  __out 
#define  __in
#define  __out_opt
#define  __inout_opt
#endif

#define lang(s) getTransl("kserv",s)

#include <map>
#include <unordered_map>
#include <wchar.h>

#define CREATE_FLAGS 0
#define SWAPBYTES(dw) \
    ((dw<<24 & 0xff000000) | (dw<<8  & 0x00ff0000) | \
    (dw>>8  & 0x0000ff00) | (dw>>24 & 0x000000ff))

#define COLOR_BLACK D3DCOLOR_RGBA(0,0,0,128)
#define COLOR_AUTO D3DCOLOR_RGBA(135,135,135,255)
#define COLOR_CHOSEN D3DCOLOR_RGBA(210,210,210,255)
#define COLOR_INFO D3DCOLOR_RGBA(0xb0,0xff,0xb0,0xff)

#define NUM_TEAMS 500
#define NUM_TEAMS_TOTAL 500

#define NUM_SLOTS 256
#define BIN_FONT_FIRST   1657
#define BIN_FONT_LAST    2680
#define BIN_NUMBER_FIRST 2681
#define BIN_NUMBER_LAST  3704
#define BIN_KIT_FIRST    5277
#define BIN_KIT_LAST     5788

#define XBIN_KIT_FIRST    9885
#define XBIN_KIT_LAST     10396
#define XBIN_FONT_FIRST   10873
#define XBIN_FONT_LAST    11896
#define XBIN_NUMBER_FIRST 11897
#define XBIN_NUMBER_LAST  12920

#define XSLOT_FIRST 0x900
#define XSLOT_LAST  0x9ff
#define XBIN_LAST XBIN_NUMBER_LAST

#define KITS_IMG        0x0c   // dt0c.img
#define EXPANSION_IMG   0x0f   // dt0f.img

#define NUM_SLOTS_DT0F 128
#define DT0F_BIN_FONT_FIRST   397
#define DT0F_BIN_FONT_LAST    908
#define DT0F_BIN_NUMBER_FIRST 909
#define DT0F_BIN_NUMBER_LAST  1420
#define DT0F_BIN_KIT_FIRST    1923
#define DT0F_BIN_KIT_LAST     2178


HINSTANCE hInst = NULL;
KMOD k_kserv = {MODID, NAMELONG, NAMESHORT, DEFAULT_DEBUG};

enum {GA, PA, GB, PB};

enum
{
    BIN_KIT_GK,
    BIN_KIT_PL,
    BIN_FONT_GA,
    BIN_FONT_GB,
    BIN_FONT_PA,
    BIN_FONT_PB,
    BIN_NUMS_GA,
    BIN_NUMS_GB,
    BIN_NUMS_PA,
    BIN_NUMS_PB,
};

class kserv_config_t 
{
public:
    kserv_config_t() : _use_description(true) 
    { 
       memset(_techfit, 0, sizeof(_techfit));
    }
    bool _use_description;
    wstring _map_file;
    BYTE _techfit[0x10000];
};

/**
 * Utility class to keep track of buffers
 * and memory allocations/deallocations
 */
class kserv_buffer_manager_t
{
public:
    kserv_buffer_manager_t() : _unpacked(NULL), _packed(NULL) {}
    ~kserv_buffer_manager_t() 
    {
        if (_unpacked) { HeapFree(GetProcessHeap(),0,_unpacked); }
        if (_packed) { HeapFree(GetProcessHeap(),0,_packed); }
        for (list<BYTE*>::iterator bit = _buffers.begin();
                bit != _buffers.end();
                bit++)
            HeapFree(GetProcessHeap(),0,*bit);
        TRACE(L"buffers deallocated.");
    }
    UNPACKED_BIN* new_unpacked(size_t size)
    {
        _unpacked = (UNPACKED_BIN*)HeapAlloc(GetProcessHeap(),
                HEAP_ZERO_MEMORY, size);
        return _unpacked;
    }
    PACKED_BIN* new_packed(size_t size)
    {
        _packed = (PACKED_BIN*)HeapAlloc(GetProcessHeap(),
                HEAP_ZERO_MEMORY, size);
        return _packed;
    }
    BYTE* new_buffer(size_t size)
    {
        BYTE* _buffer = (BYTE*)HeapAlloc(GetProcessHeap(),
                HEAP_ZERO_MEMORY, size);
        if (_buffer) _buffers.push_back(_buffer);
        return _buffer;
    }

    UNPACKED_BIN* _unpacked;
    PACKED_BIN* _packed;
    list<BYTE*> _buffers;
};

typedef struct _ORG_TEAM_KIT_INFO
{
    TEAM_KIT_INFO tki;
    bool ga;
    bool pa;
    bool gb;
    bool pb;
} ORG_TEAM_KIT_INFO;

typedef map<wstring,Kit>::iterator kit_iter_t;

typedef struct _KIT_PICK
{
    WORD teamIndex;
    wstring kitKey;
    kit_iter_t iter;
} KIT_PICK;

class SWITCH_KIT
{
public:
    SWITCH_KIT(
            KIT_CHOICE* _kc, 
            kit_iter_t& _iter,
            kit_iter_t& _iter_begin,
            kit_iter_t& _iter_end,
            bool p_1st) :
        kc(_kc),
        iter(_iter),
        iter_begin(_iter_begin),
        iter_end(_iter_end),
        _1st(p_1st)
    {}

    KIT_CHOICE* kc;
    map<wstring,Kit>::iterator& iter;
    map<wstring,Kit>::iterator& iter_begin;
    map<wstring,Kit>::iterator& iter_end;
    bool _1st;
};

// VARIABLES
static int _myPage = -1;

#define NUM_INITED_KITS 10
KIT_OBJECT* initedKits[NUM_INITED_KITS];
int nextInitedKitsIdx = 0;

WORD _lastHomePA = 0xffff;
WORD _lastHomePB = 0xffff;
WORD _lastAwayPA = 0xffff;
WORD _lastAwayPB = 0xffff;
WORD _lastHome = 0xffff;
WORD _lastAway = 0xffff;
bool _homeDone = false;

bool _widescreenFlag = false;

GDB* _gdb;
kserv_config_t _kserv_config;
unordered_map<int,ORG_TEAM_KIT_INFO> _orgTeamKitInfo;
unordered_map<WORD,WORD> _slotMap;
unordered_map<WORD,WORD> _reverseSlotMap;
typedef unordered_map<WORD,KitCollection>::iterator kc_iter_t;
CRITICAL_SECTION _cs;

typedef unordered_map<WORD,WORD> slot_map_t;
class slot_maps_t 
{
public:
    slot_map_t ga;
    slot_map_t gb;
    slot_map_t pa;
    slot_map_t pb;
};

slot_maps_t _slotMaps;
slot_maps_t _reverseSlotMaps;
slot_map_t _takenSlots;

// kit iterators
map<wstring,Kit>::iterator g_iterHomePA;
map<wstring,Kit>::iterator g_iterHomePB;
map<wstring,Kit>::iterator g_iterAwayPA;
map<wstring,Kit>::iterator g_iterAwayPB;
map<wstring,Kit>::iterator g_iterHomeGA;
map<wstring,Kit>::iterator g_iterHomeGB;
map<wstring,Kit>::iterator g_iterAwayGA;
map<wstring,Kit>::iterator g_iterAwayGB;

map<wstring,Kit>::iterator g_iterHomePA_begin;
map<wstring,Kit>::iterator g_iterHomePB_begin;
map<wstring,Kit>::iterator g_iterAwayPA_begin;
map<wstring,Kit>::iterator g_iterAwayPB_begin;
map<wstring,Kit>::iterator g_iterHomeGA_begin;
map<wstring,Kit>::iterator g_iterHomeGB_begin;
map<wstring,Kit>::iterator g_iterAwayGA_begin;
map<wstring,Kit>::iterator g_iterAwayGB_begin;

map<wstring,Kit>::iterator g_iterHomePA_end;
map<wstring,Kit>::iterator g_iterHomePB_end;
map<wstring,Kit>::iterator g_iterAwayPA_end;
map<wstring,Kit>::iterator g_iterAwayPB_end;
map<wstring,Kit>::iterator g_iterHomeGA_end;
map<wstring,Kit>::iterator g_iterHomeGB_end;
map<wstring,Kit>::iterator g_iterAwayGA_end;
map<wstring,Kit>::iterator g_iterAwayGB_end;

WORD _firstXslot = XSLOT_FIRST;
WORD _nextXslot = XSLOT_FIRST;
bool _home1st = true;
bool _away1st = true;

struct
{
    KIT_CHOICE* home1st;
    KIT_CHOICE* home2nd;
    KIT_CHOICE* away1st;
    KIT_CHOICE* away2nd;
} _kitChoices;

unordered_map<WORD,KIT_PICK> _kitPicks; // map: slot -> KIT_PICK
//unordered_map<WORD,bool> _plKitPicks; // reverse map: teamIndex -> flag
//unordered_map<WORD,bool> _gkKitPicks; // reverse map: teamIndex -> flag

// FUNCTIONS
HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface);
void kservConfig(char* pName, const void* pValue, DWORD a);
void kservConfigModels(char* pName, const void* pValue, DWORD a);
void kservConfigModels2(char* pName, const void* pValue, DWORD a);
	
WORD GetTeamId(KIT_CHOICE* kc);
WORD GetTeamIndex(KIT_CHOICE* kc);
WORD GetTeamIdByIndex(int index);
WORD GetTeamIndexById(WORD id);
WORD GetTeamIndexBySlot(WORD slot, int binType);
void GetTeamIndexesBySlot(WORD slot, int binType, WORD& teamA, WORD& teamB);
bool FindTeamInGDB(WORD teamIndex, KitCollection*& kcol);
char* GetTeamNameByIndex(int index, TEAM_NAME*);
char* GetTeamNameById(WORD id);
void kservAfterReadNamesCallPoint();
void kservAfterReadNamesCallPoint2();
KEXPORT void kservAfterReadNames();
DWORD WINAPI kservAfterReadNamesDelayed(LPCVOID param=NULL);
void kservAfterReadNamesAsync();
void DumpSlotsInfo(TEAM_KIT_INFO* teamKitInfo=NULL, TEAM_NAME* teamNames=NULL);
void kservKeyboardEvent(int code1, WPARAM wParam, LPARAM lParam);
void kservReadEditData(LPCVOID dta, DWORD size);
void kservWriteEditData(LPCVOID dta, DWORD size);
void InitSlotMapInThread(TEAM_KIT_INFO* teamKitInfo=NULL);
DWORD WINAPI InitSlotMap(LPCVOID param=NULL);
void RelinkKit(WORD teamIndex, WORD slot, KIT_INFO& kitInfo,
               slot_map_t& slotMap, slot_map_t& revereSlotMap);
void ResetTeamKitInfo(WORD teamIndex, WORD slot, TEAM_KIT_INFO* tki=NULL);
void RestoreTeamKitInfos(TEAM_KIT_INFO* tki=NULL);
void RestoreTeamKitInfo(WORD teamIndex, int which, TEAM_KIT_INFO* tki=NULL);
bool kservGetFileInfo(DWORD afsId, DWORD binId, HANDLE& hfile, DWORD& fsize);
bool CreatePipeForKitBin(DWORD afsId, DWORD binId, HANDLE& handle, DWORD& size);
bool CreatePipeForFontBin(DWORD afsId, DWORD binId, HANDLE& handle, DWORD& size);
bool CreatePipeForNumbersBin(DWORD afsId, DWORD binId, HANDLE& handle, DWORD& size);
int GetBinType(DWORD afsId, DWORD id);
void ApplyKitAttributes(map<wstring,Kit>& m, const wchar_t* kitKey, KIT_INFO& ki);
KEXPORT void ApplyKitAttributes(const map<wstring,Kit>::iterator kiter, KIT_INFO& ki);
void RGBAColor2KCOLOR(const RGBAColor& color, KCOLOR& kcolor);
void KCOLOR2RGBAColor(const KCOLOR kcolor, RGBAColor& color);

DWORD LoadPNGTexture(BITMAPINFO** tex, const wchar_t* filename);
void ApplyAlphaChunk(RGBQUAD* palette, BYTE* memblk, DWORD size);
static int read_file_to_mem(const wchar_t *fn,unsigned char **ppfiledata, int *pfilesize);
void ApplyDIBTexture(TEXTURE_ENTRY* tex, BITMAPINFO* bitmap);
void FreePNGTexture(BITMAPINFO* bitmap);
void ReplaceTexturesInBin(UNPACKED_BIN* bin, wstring files[], size_t n);
void InitIterators();
void ResetKit(SWITCH_KIT& sw);

void kservReadNumSlotsCallPoint1();
void kservReadNumSlotsCallPoint4();
KEXPORT DWORD kservReadNumSlots(DWORD slot);

void kservReadUniformPickCallPoint();
KEXPORT void kservReadUniformPick(BYTE* selection);
void kservCheckKitReloadFlagCallPoint();
KEXPORT void kservCheckKitReloadFlag(KIT_CHOICE* pKC);
void SwitchKit(SWITCH_KIT& sw, bool advanceIter=true);

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
        InitializeCriticalSection(&_cs);
	}
	
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		LOG(L"Shutting down this module...");
        if (_gdb) delete _gdb;
        DeleteCriticalSection(&_cs);
	}
	
	return true;
}

WORD GetTeamIdByIndex(int index)
{
    if (index < 0 || index >= NUM_TEAMS_TOTAL)
        return 0xffff; // invalid index
    TEAM_KIT_INFO* teamKitInfo = (TEAM_KIT_INFO*)(*(DWORD*)dta[PLAYERS_DATA] 
            + dta[TEAM_KIT_INFO_OFFSET]);
    return teamKitInfo[index].id;
}

char* GetTeamNameByIndex(int index, TEAM_NAME* teamNames)
{
    if (index < 0 || index >= NUM_TEAMS_TOTAL)
        return NULL; // invalid index
    if (!teamNames)
        teamNames = (TEAM_NAME*)(*(DWORD*)dta[PLAYERS_DATA] 
            + dta[TEAM_NAMES_OFFSET]);

    return (char*)&(teamNames[index].name);
}

char* GetTeamNameById(WORD id)
{
    TEAM_NAME* teamNames = (TEAM_NAME*)(*(DWORD*)dta[PLAYERS_DATA] 
            + dta[TEAM_NAMES_OFFSET]);
    for (int i=0; i<NUM_TEAMS_TOTAL; i++)
    {
        if (teamNames[i].teamId == id)
            return (char*)&(teamNames[i].name);
    }
    return NULL;
}

WORD GetTeamIndexById(WORD id)
{
    TEAM_KIT_INFO* teamKitInfo = (TEAM_KIT_INFO*)(*(DWORD*)dta[PLAYERS_DATA] 
            + dta[TEAM_KIT_INFO_OFFSET]);
    for (int i=0; i<NUM_TEAMS_TOTAL; i++)
    {
        if (teamKitInfo[i].id == id)
            return i;
    }
    return 0xffff;
}

HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface)
{
	unhookFunction(hk_D3D_CreateDevice, initModule);

    LOG(L"Initializing Kserv Module");
	
	// hooks
    HookCallPoint(code[C_AFTER_READ_NAMES], 
            kservAfterReadNamesCallPoint, 6, 5);
    HookCallPoint(code[C_AFTER_APPLY_CONTENT], 
            kservAfterReadNamesCallPoint, 6, 5);
    //HookCallPoint(code[C_AFTER_APPLY_CONTENT], 
    //        kservAfterReadNamesCallPoint2, 6, 0);

    HookCallPoint(code[C_READ_NUM_SLOTS1], kservReadNumSlotsCallPoint1, 6, 1);
    HookCallPoint(code[C_READ_NUM_SLOTS2], kservReadNumSlotsCallPoint1, 6, 1);
    HookCallPoint(code[C_READ_NUM_SLOTS3], kservReadNumSlotsCallPoint4, 6, 0);
    HookCallPoint(code[C_READ_UNIFORM_PICK], kservReadUniformPickCallPoint, 
            6, 1);
    HookCallPoint(code[C_CHECK_KIT_RELOAD_FLAG], 
            kservCheckKitReloadFlagCallPoint, 
            6, 2);


    getConfig("kserv", "debug", DT_DWORD, 1, kservConfig);
    getConfig("kserv", "use.description", DT_DWORD, 2, kservConfig);
    getConfig("kserv", "techfit.model", DT_DWORD, C_ALL, kservConfigModels);
    getConfig("kserv", "tight.model", DT_DWORD, C_ALL, kservConfigModels2);
    getConfig("kserv", "gdb.uni.map", DT_STRING, 3, kservConfig);
    LOG(L"debug = %d", k_kserv.debug);
    LOG(L"use.description = %d", _kserv_config._use_description);

    // Load GDB
    LOG(L"pesDir: {%s}",getPesInfo()->pesDir);
    LOG(L"myDir : {%s}",getPesInfo()->myDir);
    LOG(L"gdbDir: {%sGDB}",getPesInfo()->gdbDir);

    wstring uniMapFile(getPesInfo()->gdbDir);
    if (_kserv_config._map_file.empty()) {
        uniMapFile += L"GDB\\uni\\map.txt";
    } 
    else {
        if (_kserv_config._map_file[1]==L':') {
            // absolute file name
            uniMapFile = _kserv_config._map_file;
        }
        else {
            uniMapFile += L"GDB\\uni\\";
            uniMapFile += _kserv_config._map_file;
        }
    }
    LOG(L"GDB uni mapFile: {%s}", uniMapFile.c_str());

    _gdb = new GDB(getPesInfo()->gdbDir, uniMapFile, false);
    LOG(L"Teams in GDB map: %d", _gdb->uni.size());

    // initialize techfit-models map to defaults, if none
    // specified in the config file 
    bool noneInConfig(true);
    for (int i=0; i<sizeof(_kserv_config._techfit); i++) {
       if (_kserv_config._techfit[i] == 1) {
          LOG(L"techfit model: %d", i);
          noneInConfig = false;
       }
       else if (_kserv_config._techfit[i] == 2) {
          LOG(L"tight model: %d", i);
          noneInConfig = false;
       }
    }
    if (noneInConfig) {
       LOG(L"defaulting to these techfit models: 35-41,54");
       LOG(L"defaulting to these tight models: 24");
       _kserv_config._techfit[24] = 2;
       _kserv_config._techfit[35] = 1;
       _kserv_config._techfit[36] = 1;
       _kserv_config._techfit[37] = 1;
       _kserv_config._techfit[38] = 1;
       _kserv_config._techfit[39] = 1;
       _kserv_config._techfit[40] = 1;
       _kserv_config._techfit[41] = 1;
       _kserv_config._techfit[54] = 1;
    }

    // initialize iterators
    InitIterators();

    // add callbacks
    addReadEditDataCallback(kservReadEditData);
    addWriteEditDataCallback(kservWriteEditData);
    afsioAddCallback(kservGetFileInfo);

	TRACE(L"Initialization complete.");
	return D3D_OK;
}

void kservConfig(char* pName, const void* pValue, DWORD a)
{
	switch (a) {
		case 1: // debug
			k_kserv.debug = *(DWORD*)pValue;
			break;
        case 2: // use-description
            _kserv_config._use_description = *(DWORD*)pValue == 1;
            break;
        case 3: // gdb.uni.map
            _kserv_config._map_file = wstring((wchar_t*)pValue);
	}
	return;
}

void kservConfigModels(char* pName, const void* pValue, DWORD a)
{
   WORD model = *(DWORD*)pValue;
   _kserv_config._techfit[model] = 1;
}

void kservConfigModels2(char* pName, const void* pValue, DWORD a)
{
   WORD model = *(DWORD*)pValue;
   _kserv_config._techfit[model] = 2;
}

KEXPORT DWORD hookVtableFunction(DWORD vtableAddr, DWORD offset, void* func)
{
	DWORD* vtableEntry = (DWORD*)(vtableAddr + offset*4);
	DWORD orig = *vtableEntry;
	
	DWORD protection = 0;
    DWORD newProtection = PAGE_READWRITE;
    if (VirtualProtect(vtableEntry, 4, newProtection, &protection)) 
		*vtableEntry = (DWORD)func;
	return orig;
}

VOID WINAPI ColorFillRed (D3DXVECTOR4* pOut, const D3DXVECTOR2* pTexCoord, 
const D3DXVECTOR2* pTexelSize, LPVOID pData)
{
	// red
    *pOut = D3DXVECTOR4(0.7f, 0.3f, 0.3f, 1.0f);
}

VOID WINAPI ColorFillBlue (D3DXVECTOR4* pOut, const D3DXVECTOR2* pTexCoord, 
const D3DXVECTOR2* pTexelSize, LPVOID pData)
{
	// blue
    *pOut = D3DXVECTOR4(0.3f, 0.3f, 0.7f, 1.0f);
}

void kservAfterReadNamesCallPoint()
{
    __asm {
        mov dword ptr ds:[eax+0x4e24], 0x1f4  // execute replaced code
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        call kservAfterReadNames
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

void kservAfterReadNamesCallPoint2()
{
    __asm {
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        call kservAfterReadNames//Async
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        add esp,4 // pop ret-address
        retn 8
    }
}

KEXPORT void kservAfterReadNames()
{
    // dump slot information
    if (k_kserv.debug)
        DumpSlotsInfo();

    // initialize kit slots
    InitSlotMapInThread();
}

DWORD WINAPI kservAfterReadNamesDelayed(LPCVOID param)
{
    Sleep((DWORD)param);
    kservAfterReadNames();
    return 0;
}

void kservAfterReadNamesAsync()
{
    DWORD threadId;
    HANDLE initThread = CreateThread( 
        NULL,                   // default security attributes
        0,                      // use default stack size  
        (LPTHREAD_START_ROUTINE)kservAfterReadNamesDelayed,
        (LPVOID)3000,           // delay in milliseconds
        0,                      // use default creation flags 
        &threadId);             // returns the thread identifier 
}

void DumpSlotsInfo(TEAM_KIT_INFO* teamKitInfo, TEAM_NAME* teamNames)
{
    // team names are stored in Utf-8, so we write the bytes as is.
    if (!teamKitInfo)
        teamKitInfo = (TEAM_KIT_INFO*)(*(DWORD*)dta[PLAYERS_DATA] 
                + dta[TEAM_KIT_INFO_OFFSET]);
    TRACE(L"teamKitInfo = %08x", (DWORD)teamKitInfo);
    TRACE(L"teamNames = %08x", (DWORD)teamNames);
    TRACE(L"sizeof(KIT_INFO) = %08x", sizeof(KIT_INFO));
    TRACE(L"sizeof(TEAM_KIT_INFO) = %08x", sizeof(TEAM_KIT_INFO));

    wstring filename(getPesInfo()->myDir);
    filename += L"\\relinks.txt";
    //filename += L"\\uni.txt";
    FILE* f = _wfopen(filename.c_str(),L"wt");
    if (!f)
        return;

    for (int i=0; i<NUM_TEAMS; i++)
    {
        WORD teamId = teamKitInfo[i].id;
        if (teamId == 0xffff)
            continue;
        fprintf(f, "slot: %6d\tteam: %3d (%04x) %s\n", 
            (short)teamKitInfo[i].ga.slot, 
            i, teamId, GetTeamNameByIndex(i, teamNames));
        //char* name = GetTeamNameByIndex(i, teamNames);
        //if (name[0]!='\0')
        //    fprintf(f, "%3d, %s\n", i, name);
        //else
        //    fprintf(f, "%3d, (EMPTY)\n", i);
    }
    fclose(f);
}

void InitSlotMapInThread(TEAM_KIT_INFO* teamKitInfo)
{
    if (!teamKitInfo)
        teamKitInfo = (TEAM_KIT_INFO*)(*(DWORD*)dta[PLAYERS_DATA] 
                + dta[TEAM_KIT_INFO_OFFSET]);

    DWORD threadId;
    HANDLE initThread = CreateThread( 
        NULL,                   // default security attributes
        0,                      // use default stack size  
        (LPTHREAD_START_ROUTINE)InitSlotMap, // thread function name
        teamKitInfo,            // argument to thread function 
        0,                      // use default creation flags 
        &threadId);             // returns the thread identifier 
}

void SetSlot(map<wstring,Kit>& m, const wchar_t* key, WORD slot)
{
    map<wstring,Kit>::iterator it = m.find(key);
    if (it != m.end())
    {
        it->second.slot = slot;
        //LOG(L"SetSlot:: %s --> %04x", key, slot);
    }
}

/**
 * Get next available slot for extra kits.
 * If run out of slots, then start reusing the oldest.
 * (Hoping that the game will have purged them from its
 * cache by that time).
 */
WORD GetNextXslot()
{
    WORD slot = _nextXslot++;
    if (_nextXslot > XSLOT_LAST)
        _nextXslot = _firstXslot;
    return slot;
}

DWORD WINAPI InitSlotMap(LPCVOID param)
{
    EnterCriticalSection(&_cs);
    TEAM_KIT_INFO* teamKitInfo = (TEAM_KIT_INFO*)param;
    if (!teamKitInfo)
        teamKitInfo = (TEAM_KIT_INFO*)(*(DWORD*)dta[PLAYERS_DATA] 
                + dta[TEAM_KIT_INFO_OFFSET]);

    //_slotMap.clear();
    //_reverseSlotMap.clear();
    _slotMaps.ga.clear();
    _slotMaps.gb.clear();
    _slotMaps.pa.clear();
    _slotMaps.pb.clear();
    _reverseSlotMaps.ga.clear();
    _reverseSlotMaps.gb.clear();
    _reverseSlotMaps.pa.clear();
    _reverseSlotMaps.pb.clear();
    _takenSlots.clear();
    _orgTeamKitInfo.clear();

    // linked (or re-linked teams)
    for (WORD i=0; i<NUM_TEAMS; i++)
    {
        /*
        short slot = (short)teamKitInfo[i].pa.slot;
        TRACE(L"team (index=%d, id=0x%04x): slot=0x%04x",
                i, teamKitInfo[i].id, (WORD)slot);
        if (slot >= 0)
        {
            _slotMap.insert(pair<WORD,WORD>((WORD)slot,i));
            _reverseSlotMap.insert(pair<WORD,WORD>(i,(WORD)slot));
        }
        */

        short slot;
        slot = (short)teamKitInfo[i].ga.slot;
        if (slot >= 0) {
            _slotMaps.ga.insert(pair<WORD,WORD>((WORD)slot,i));
            _reverseSlotMaps.ga.insert(pair<WORD,WORD>(i, (WORD)slot));
            _takenSlots.insert(pair<WORD,bool>(slot,true));
        }
        slot = (short)teamKitInfo[i].gb.slot;
        if (slot >= 0) {
            _slotMaps.gb.insert(pair<WORD,WORD>((WORD)slot,i));
            _reverseSlotMaps.gb.insert(pair<WORD,WORD>(i, (WORD)slot));
            _takenSlots.insert(pair<WORD,bool>(slot,true));
        }
        slot = (short)teamKitInfo[i].pa.slot;
        if (slot >= 0) {
            _slotMaps.pa.insert(pair<WORD,WORD>((WORD)slot,i));
            _reverseSlotMaps.pa.insert(pair<WORD,WORD>(i, (WORD)slot));
            _takenSlots.insert(pair<WORD,bool>(slot,true));
        }
        slot = (short)teamKitInfo[i].pb.slot;
        if (slot >= 0) {
            _slotMaps.pb.insert(pair<WORD,WORD>((WORD)slot,i));
            _reverseSlotMaps.pb.insert(pair<WORD,WORD>(i, (WORD)slot));
            _takenSlots.insert(pair<WORD,bool>(slot,true));
        }

        /*
        // debug
        if (i==88) {
            //teamKitInfo[i].pb.sleevePatchRightPosLong = 1;
            //teamKitInfo[i].pb.sleevePatchRightPosShort = 5;
            //teamKitInfo[i].pb.sleevePatchLeftPosLong = 1;
            //teamKitInfo[i].pb.sleevePatchLeftPosShort = 5;
            DumpData(&teamKitInfo[i], sizeof(TEAM_KIT_INFO));
        }
        */
    }
    //LOG(L"Normal slots taken: %d", _slotMap.size());
    LOG(L"Normal slots taken: %d", _takenSlots.size());

    // GDB teams
    WORD nextSlot = XSLOT_FIRST;
    for (unordered_map<WORD,KitCollection>::iterator git = _gdb->uni.begin();
            git != _gdb->uni.end();
            git++)
    {
        //unordered_map<WORD,WORD>::iterator rit = _reverseSlotMap.find(git->first);
        //bool toRelink = (rit == _reverseSlotMap.end());
        bool gaRelink(false);
        bool gbRelink(false);
        bool paRelink(false);
        bool pbRelink(false);

        // store original attributes
        WORD i = git->first;
        if (i >= NUM_TEAMS)
            continue; // safety check, until we implement support for
                      // add additional teams
                      
        ORG_TEAM_KIT_INFO o;
        ZeroMemory(&o, sizeof(ORG_TEAM_KIT_INFO));
        memcpy(&o.tki, &teamKitInfo[i], sizeof(TEAM_KIT_INFO));
        //DumpData(&o.tki, sizeof(TEAM_KIT_INFO));
        if (git->second.ga!=git->second.goalkeepers.end())
        {
            unordered_map<WORD,WORD>::iterator rit = 
                _reverseSlotMaps.ga.find(git->first);
            gaRelink = (rit == _reverseSlotMaps.ga.end());

            o.ga = true;
            if (gaRelink) RelinkKit(
                i, nextSlot, teamKitInfo[i].ga,
                _slotMaps.ga, _reverseSlotMaps.ga);
        }
        if (git->second.gb!=git->second.goalkeepers.end())
        {
            unordered_map<WORD,WORD>::iterator rit = 
                _reverseSlotMaps.gb.find(git->first);
            gbRelink = (rit == _reverseSlotMaps.gb.end());

            o.gb = true;
            if (gbRelink) RelinkKit(
                i, nextSlot, teamKitInfo[i].gb,
                _slotMaps.gb, _reverseSlotMaps.gb);
        }
        if (git->second.pa!=git->second.players.end())
        {
            unordered_map<WORD,WORD>::iterator rit = 
                _reverseSlotMaps.pa.find(git->first);
            paRelink = (rit == _reverseSlotMaps.pa.end());

            o.pa = true;
            if (paRelink) RelinkKit(
                i, nextSlot, teamKitInfo[i].pa,
                _slotMaps.pa, _reverseSlotMaps.pa);
        }
        if (git->second.pb!=git->second.players.end())
        {
            unordered_map<WORD,WORD>::iterator rit = 
                _reverseSlotMaps.pb.find(git->first);
            pbRelink = (rit == _reverseSlotMaps.pb.end());

            o.pb = true;
            if (pbRelink) RelinkKit(
                i, nextSlot, teamKitInfo[i].pb,
                _slotMaps.pb, _reverseSlotMaps.pb);
        }
        _orgTeamKitInfo.insert(pair<int,ORG_TEAM_KIT_INFO>(i,o));

        // apply attributes
        ApplyKitAttributes(git->second.goalkeepers, 
                L"ga",teamKitInfo[git->first].ga);
        ApplyKitAttributes(git->second.players, 
                L"pa",teamKitInfo[git->first].pa);
        ApplyKitAttributes(git->second.goalkeepers, 
                L"gb",teamKitInfo[git->first].gb);
        ApplyKitAttributes(git->second.players, 
                L"pb",teamKitInfo[git->first].pb);

        if (k_kserv.debug)
            LOG(L"setting *a/*b slots for team: %d", i);
        SetSlot(git->second.goalkeepers,L"ga",teamKitInfo[i].ga.slot);
        SetSlot(git->second.goalkeepers,L"gb",teamKitInfo[i].gb.slot);
        SetSlot(git->second.players,L"pa",teamKitInfo[i].pa.slot);
        SetSlot(git->second.players,L"pb",teamKitInfo[i].pb.slot);

        //DumpData(&teamKitInfo[git->first], sizeof(TEAM_KIT_INFO));

        if (!(o.ga||o.gb||o.pa||o.pb))
        {
            LOG(L"WARNING: team %d (%s) appears to have no GDB kits", 
                    i, git->second.foldername.c_str()); 
        }

        // move to next slot
        if ((o.ga||o.gb||o.pa||o.pb) && (
                gaRelink||gbRelink||paRelink||pbRelink))
        {
            LOG(L"team %d dynamically relinked to x-slot 0x%x", i, nextSlot); 
            nextSlot++;
        }
    }
    size_t numTaken = _slotMaps.ga.size();
    numTaken = max(numTaken, _slotMaps.gb.size());
    numTaken = max(numTaken, _slotMaps.pa.size());
    numTaken = max(numTaken, _slotMaps.pb.size());
    LOG(L"Total slots taken: %d", numTaken);

    // keep track of slots for extra kits
    _firstXslot = _nextXslot = nextSlot;

    // extend dt0c.img
    afsioExtendSlots(KITS_IMG, XBIN_LAST+1);
    LOG(L"KITS_IMG extended to %d bins", XBIN_LAST+1);

    // dump slot information
    if (k_kserv.debug)
        DumpSlotsInfo();
 
    LeaveCriticalSection(&_cs);
    return 0;
}

void RelinkKit(WORD teamIndex, WORD slot, KIT_INFO& kitInfo,
               slot_map_t& slotMap, slot_map_t& reverseSlotMap)
{
    kitInfo.slot = slot;
    slotMap.insert(pair<WORD,WORD>(slot,teamIndex));
    reverseSlotMap.insert(pair<WORD,WORD>(teamIndex,slot));
}

void ApplyKitAttributes(map<wstring,Kit>& m, const wchar_t* kitKey, KIT_INFO& ki)
{
    map<wstring,Kit>::iterator kiter = m.find(kitKey);
    if (kiter != m.end())
        ApplyKitAttributes(kiter, ki);
}

KEXPORT void ApplyKitAttributes(const map<wstring,Kit>::iterator kiter, KIT_INFO& ki)
{
    // load kit attributes from config.txt, if needed
    _gdb->loadConfig(kiter->second);

    // apply attributes
    if (kiter->second.attDefined & MODEL)
    {
        ki.model = kiter->second.model;
        ki.techfit = _kserv_config._techfit[ki.model];
    }
    if (kiter->second.attDefined & COLLAR)
        ki.collar = kiter->second.collar;
    if (kiter->second.attDefined & FRONT_NUMBER_SHOW)
        ki.frontNumberShow = kiter->second.frontNumberShow;
    if (kiter->second.attDefined & SHORTS_NUMBER_LOCATION)
        ki.shortsNumberPosition = kiter->second.shortsNumberLocation;
    if (kiter->second.attDefined & NAME_SHOW)
        ki.nameShow = kiter->second.nameShow;
    if (kiter->second.attDefined & NAME_SHAPE)
        ki.nameShape = kiter->second.nameShape;

    if (kiter->second.attDefined & FRONT_NUMBER_Y)
        ki.frontNumberY = kiter->second.frontNumberY;
    if (kiter->second.attDefined & FRONT_NUMBER_X)
        ki.frontNumberX = kiter->second.frontNumberX;
    if (kiter->second.attDefined & FRONT_NUMBER_SIZE)
        ki.frontNumberSize = kiter->second.frontNumberSize;
    if (kiter->second.attDefined & SLEEVE_PATCH_RIGHT_POS_SHORT)
        ki.sleevePatchRightPosShort = kiter->second.sleevePatchRightPosShort;
    if (kiter->second.attDefined & SLEEVE_PATCH_RIGHT_POS_LONG)
        ki.sleevePatchRightPosLong = kiter->second.sleevePatchRightPosLong;
    if (kiter->second.attDefined & SLEEVE_PATCH_LEFT_POS_SHORT)
        ki.sleevePatchLeftPosShort = kiter->second.sleevePatchLeftPosShort;
    if (kiter->second.attDefined & SLEEVE_PATCH_LEFT_POS_LONG)
        ki.sleevePatchLeftPosLong = kiter->second.sleevePatchLeftPosLong;
    if (kiter->second.attDefined & SHORTS_NUMBER_SIZE)
        ki.shortsNumberSize = kiter->second.shortsNumberSize;
    if (kiter->second.attDefined & SHORTS_NUMBER_Y)
        ki.shortsNumberY = kiter->second.shortsNumberY;
    if (kiter->second.attDefined & SHORTS_NUMBER_X)
        ki.shortsNumberX = kiter->second.shortsNumberX;
    if (kiter->second.attDefined & NUMBER_Y)
        ki.numberY = kiter->second.numberY;
    if (kiter->second.attDefined & NUMBER_SIZE)
        ki.numberSize = kiter->second.numberSize;
    if (kiter->second.attDefined & NAME_Y)
        ki.nameY = kiter->second.nameY;
    if (kiter->second.attDefined & NAME_SIZE)
        ki.nameSize = kiter->second.nameSize;
    if (kiter->second.attDefined & ICON_TYPE)
        ki.iconType = kiter->second.iconType;

    if (kiter->second.attDefined & MAIN_COLOR) 
    {
        RGBAColor2KCOLOR(kiter->second.mainColor, ki.mainColor);
        // kit selection uses all 5 shirt colors - not only main (first one)
        for (int i=0; i<4; i++)
            RGBAColor2KCOLOR(kiter->second.mainColor, ki.editShirtColors[i]);
    }
    // shirt second color
    if (kiter->second.attDefined & SECOND_COLOR)
        RGBAColor2KCOLOR(kiter->second.secondColor, ki.secondColor);
    // shorts main color
    if (kiter->second.attDefined & SHORTS_MAIN_COLOR)
        RGBAColor2KCOLOR(kiter->second.shortsFirstColor, ki.shortsFirstColor);
    // socks color
    if (kiter->second.attDefined & SOCKS_COLOR)
        RGBAColor2KCOLOR(kiter->second.socksColor, ki.socksColor);
}

void RGBAColor2KCOLOR(const RGBAColor& color, KCOLOR& kcolor)
{
    kcolor = 0x8000
        +((color.r>>3) & 31)
        +0x20*((color.g>>3) & 31)
        +0x400*((color.b>>3) & 31);
}

void KCOLOR2RGBAColor(const KCOLOR kcolor, RGBAColor& color)
{
    color.r = (kcolor & 31)<<3;
    color.g = (kcolor>>5 & 31)<<3;
    color.b = (kcolor>>10 & 31)<<3;
    color.a = 0xff;
}

void RestoreTeamKitInfos(TEAM_KIT_INFO* teamKitInfo)
{
    if (!teamKitInfo)
        teamKitInfo = (TEAM_KIT_INFO*)(
                *(DWORD*)dta[PLAYERS_DATA] 
                + dta[TEAM_KIT_INFO_OFFSET]);

    for (unordered_map<int,ORG_TEAM_KIT_INFO>::iterator it = _orgTeamKitInfo.begin();
            it != _orgTeamKitInfo.end();
            it++)
    {
        if (it->second.ga) 
            memcpy(&teamKitInfo[it->first].ga, 
                    &it->second.tki.ga, sizeof(KIT_INFO));
        if (it->second.pa) 
            memcpy(&teamKitInfo[it->first].pa, 
                    &it->second.tki.pa, sizeof(KIT_INFO));
        if (it->second.gb) 
            memcpy(&teamKitInfo[it->first].gb, 
                    &it->second.tki.gb, sizeof(KIT_INFO));
        if (it->second.pb) 
            memcpy(&teamKitInfo[it->first].pb, 
                    &it->second.tki.pb, sizeof(KIT_INFO));

        LOG(L"TeamKitInfo for %d restored", it->first);
    }
}

void RestoreTeamKitInfo(WORD teamIndex, int which, TEAM_KIT_INFO* teamKitInfo)
{
    if (!teamKitInfo)
        teamKitInfo = (TEAM_KIT_INFO*)(
                *(DWORD*)dta[PLAYERS_DATA] 
                + dta[TEAM_KIT_INFO_OFFSET]);

    unordered_map<int,ORG_TEAM_KIT_INFO>::iterator it;
    it = _orgTeamKitInfo.find(teamIndex);
    if (it != _orgTeamKitInfo.end())
    {
        switch(which)
        {
            case GA:
                if (it->second.ga) 
                    memcpy(&teamKitInfo[it->first].ga, 
                            &it->second.tki.ga, sizeof(KIT_INFO));
                break;
            case PA:
                if (it->second.pa) 
                    memcpy(&teamKitInfo[it->first].pa, 
                            &it->second.tki.pa, sizeof(KIT_INFO));
                break;
            case GB:
                if (it->second.gb) 
                    memcpy(&teamKitInfo[it->first].gb, 
                            &it->second.tki.gb, sizeof(KIT_INFO));
                break;
            case PB:
                if (it->second.pb) 
                    memcpy(&teamKitInfo[it->first].pb, 
                            &it->second.tki.pb, sizeof(KIT_INFO));
                break;
        }
    }
}

void ResetTeamKitInfo(WORD teamIndex, WORD slot, TEAM_KIT_INFO* teamKitInfo)
{
    if (!teamKitInfo)
        teamKitInfo = (TEAM_KIT_INFO*)(
                *(DWORD*)dta[PLAYERS_DATA] 
                + dta[TEAM_KIT_INFO_OFFSET]);

    KitCollection* kcol;
    if (FindTeamInGDB(teamIndex, kcol) && !kcol->disabled)
    {
        if (teamKitInfo[teamIndex].ga.slot == slot)
        {
            ApplyKitAttributes(kcol->goalkeepers,
                    L"ga",teamKitInfo[teamIndex].ga);
            teamKitInfo[teamIndex].ga.slot = kcol->ga->second.slot;
            LOG(L"TeamKitInfo for %d (ga) reset", teamIndex);
        }
        if (teamKitInfo[teamIndex].gb.slot == slot)
        {
            ApplyKitAttributes(kcol->goalkeepers,
                    L"gb",teamKitInfo[teamIndex].gb);
            teamKitInfo[teamIndex].gb.slot = kcol->gb->second.slot;
            LOG(L"TeamKitInfo for %d (gb) reset", teamIndex);
        }
        if (teamKitInfo[teamIndex].pa.slot == slot)
        {
            ApplyKitAttributes(kcol->players,
                    L"pa",teamKitInfo[teamIndex].pa);
            teamKitInfo[teamIndex].pa.slot = kcol->pa->second.slot;
            LOG(L"TeamKitInfo for %d (pa) reset", teamIndex);
        }
        if (teamKitInfo[teamIndex].pb.slot == slot)
        {
            ApplyKitAttributes(kcol->players,
                    L"pb",teamKitInfo[teamIndex].pb);
            teamKitInfo[teamIndex].pb.slot = kcol->pb->second.slot;
            LOG(L"TeamKitInfo for %d (pb) reset", teamIndex);
        }
    }
}

/**
 * edit data read callback
 */
void kservReadEditData(LPCVOID buf, DWORD size)
{
    // initialize kit slots
    TEAM_KIT_INFO* teamKitInfo = (TEAM_KIT_INFO*)((BYTE*)buf 
            + 0x1a0 + dta[TEAM_KIT_INFO_OFFSET] - 8);
    LOG(L"teamKitInfo = %p", teamKitInfo);
    InitSlotMap(teamKitInfo);

    TEAM_NAME* teamNames = (TEAM_NAME*)((BYTE*)buf 
            + 0x1a0 + dta[TEAM_NAMES_OFFSET] - 8); 

    // dump slot information again
    if (k_kserv.debug)
        DumpSlotsInfo(teamKitInfo, teamNames);
}

/**
 * edit data write callback
 */
void kservWriteEditData(LPCVOID buf, DWORD size)
{
    // undo re-linking: we don't want dynamic relinking
    // to be saved in Edit data
    TEAM_KIT_INFO* teamKitInfo = (TEAM_KIT_INFO*)((BYTE*)buf 
            + 0x1a0 + dta[TEAM_KIT_INFO_OFFSET] - 8);
    RestoreTeamKitInfos(teamKitInfo);
}

/**
 * AFSIO callback
 */
bool kservGetFileInfo(DWORD afsId, DWORD binId, HANDLE& hfile, DWORD& fsize)
{
    if (afsId == KITS_IMG)
    {
        // regular slots
        if (BIN_KIT_FIRST <= binId && binId <= BIN_KIT_LAST) 
            return CreatePipeForKitBin(afsId, binId, hfile, fsize);
        else if (BIN_NUMBER_FIRST <= binId && binId <= BIN_NUMBER_LAST) 
            return CreatePipeForNumbersBin(afsId, binId, hfile, fsize);
        else if (BIN_FONT_FIRST <= binId && binId <= BIN_FONT_LAST) 
            return CreatePipeForFontBin(afsId, binId, hfile, fsize);

        // x-slots
        else if (XBIN_KIT_FIRST <= binId && binId <= XBIN_KIT_LAST) 
            return CreatePipeForKitBin(afsId, binId, hfile, fsize);
        else if (XBIN_NUMBER_FIRST <= binId && binId <= XBIN_NUMBER_LAST) 
            return CreatePipeForNumbersBin(afsId, binId, hfile, fsize);
        else if (XBIN_FONT_FIRST <= binId && binId <= XBIN_FONT_LAST) 
            return CreatePipeForFontBin(afsId, binId, hfile, fsize);
    }
    else if (afsId == EXPANSION_IMG)
    {
        //LOG(L"Loading expansion BIN: afdId=0x%02x, binId=%d",
        //        afsId, binId);
        if (DT0F_BIN_KIT_FIRST<=binId && binId<=DT0F_BIN_KIT_LAST) 
            return CreatePipeForKitBin(afsId, binId, hfile, fsize);
        else if (DT0F_BIN_NUMBER_FIRST<=binId && binId<=DT0F_BIN_NUMBER_LAST) 
            return CreatePipeForNumbersBin(afsId, binId, hfile, fsize);
        else if (DT0F_BIN_FONT_FIRST<=binId && binId<=DT0F_BIN_FONT_LAST) 
            return CreatePipeForFontBin(afsId, binId, hfile, fsize);
    }

    return false;
}

int GetBinType(DWORD afsId, DWORD id)
{
    if (afsId == KITS_IMG)
    {
        // normal slots
        if (BIN_KIT_FIRST <= id && id <= BIN_KIT_LAST)
        {
            return BIN_KIT_GK + ((id - BIN_KIT_FIRST)%2);
        }
        else if (BIN_FONT_FIRST <= id && id <= BIN_FONT_LAST)
        {
            return BIN_FONT_GA + ((id - BIN_FONT_FIRST)%4);
        }
        else if (BIN_NUMBER_FIRST <= id && id <= BIN_NUMBER_LAST)
        {
            return BIN_NUMS_GA + ((id - BIN_NUMBER_FIRST)%4);
        }

        // x-slots
        else if (XBIN_KIT_FIRST <= id && id <= XBIN_KIT_LAST)
        {
            return BIN_KIT_GK + ((id - XBIN_KIT_FIRST)%2);
        }
        else if (XBIN_FONT_FIRST <= id && id <= XBIN_FONT_LAST)
        {
            return BIN_FONT_GA + ((id - XBIN_FONT_FIRST)%4);
        }
        else if (XBIN_NUMBER_FIRST <= id && id <= XBIN_NUMBER_LAST)
        {
            return BIN_NUMS_GA + ((id - XBIN_NUMBER_FIRST)%4);
        }
    }
    else if (afsId == EXPANSION_IMG)
    {
        if (DT0F_BIN_KIT_FIRST <= id && id <= DT0F_BIN_KIT_LAST)
        {
            return BIN_KIT_GK + ((id - DT0F_BIN_KIT_FIRST)%2);
        }
        else if (DT0F_BIN_FONT_FIRST <= id && id <= DT0F_BIN_FONT_LAST)
        {
            return BIN_FONT_GA + ((id - DT0F_BIN_FONT_FIRST)%4);
        }
        else if (DT0F_BIN_NUMBER_FIRST <= id && id <= DT0F_BIN_NUMBER_LAST)
        {
            return BIN_NUMS_GA + ((id - DT0F_BIN_NUMBER_FIRST)%4);
        }
    }

    return -1;
}

WORD GetTeamIndexBySlot(WORD slot, int binType)
{
    slot_map_t::iterator sit;

    switch (binType) {
        case BIN_FONT_GA:
        case BIN_NUMS_GA:
            sit = _slotMaps.ga.find(slot);
            if (sit != _slotMaps.ga.end()) {
                return sit->second;
            }
            break;
        case BIN_FONT_GB:
        case BIN_NUMS_GB:
            sit = _slotMaps.gb.find(slot);
            if (sit != _slotMaps.gb.end()) {
                return sit->second;
            }
            break;
        case BIN_FONT_PA:
        case BIN_NUMS_PA:
            sit = _slotMaps.pa.find(slot);
            if (sit != _slotMaps.pa.end()) {
                return sit->second;
            }
            break;
        case BIN_FONT_PB:
        case BIN_NUMS_PB:
            sit = _slotMaps.pb.find(slot);
            if (sit != _slotMaps.pb.end()) {
                return sit->second;
            }
            break;
    }

    unordered_map<WORD,KIT_PICK>::iterator zit = _kitPicks.find(slot);
    if (zit != _kitPicks.end()) {
        return zit->second.teamIndex;
    }
    return 0xffff;
}

void GetTeamIndexesBySlot(WORD slot, int binType, WORD& teamA, WORD& teamB)
{
    teamA = teamB = 0xffff;
    slot_map_t::iterator sit;

    switch (binType) {
        case BIN_KIT_GK:
            sit = _slotMaps.ga.find(slot);
            if (sit != _slotMaps.ga.end()) {
                teamA = sit->second;
            }
            sit = _slotMaps.gb.find(slot);
            if (sit != _slotMaps.gb.end()) {
                teamB = sit->second;
            }
            break;
        case BIN_KIT_PL:
            sit = _slotMaps.pa.find(slot);
            if (sit != _slotMaps.pa.end()) {
                teamA = sit->second;
            }
            sit = _slotMaps.pb.find(slot);
            if (sit != _slotMaps.pb.end()) {
                teamB = sit->second;
            }
            break;
    }

    if (teamA == 0xffff && teamB == 0xffff) {
        unordered_map<WORD,KIT_PICK>::iterator zit = _kitPicks.find(slot);
        if (zit != _kitPicks.end()) {
            teamA = teamB = zit->second.teamIndex;
        }
    }
}

bool FindTeamInGDB(WORD teamIndex, KitCollection*& kcol)
{
    unordered_map<WORD,KitCollection>::iterator it = _gdb->uni.find(teamIndex);
    if (it != _gdb->uni.end())
    {
        kcol = &it->second;
        return true;
    }
    kcol = &_gdb->dummyHome;
    return false;
}

/**
 * Create a pipe and write a dynamically created BIN into it.
 */
bool CreatePipeForKitBin(DWORD afsId, DWORD binId, HANDLE& handle, DWORD& size)
{
    if (k_kserv.debug)
        LOG(L"Kit-Bin:: Loading binId=%d", binId);

    // first step: determine the team, and see if we have
    // this team in the GDB
    WORD slot = (binId - BIN_KIT_FIRST) >> 1;
    if (afsId==EXPANSION_IMG)
        slot = NUM_SLOTS + ((binId - DT0F_BIN_KIT_FIRST) >> 1);

    int binType = GetBinType(afsId, binId);
    TRACE(L"binType: %d", binType);
    WORD teamIndexA = 0xffff, teamIndexB = 0xffff;
    GetTeamIndexesBySlot(slot, binType, teamIndexA, teamIndexB);
    TRACE(L"teamIndexA = %d", teamIndexA);
    TRACE(L"teamIndexB = %d", teamIndexB);
    KitCollection* kcolA;
    KitCollection* kcolB;
    bool noTeamA = (!FindTeamInGDB(teamIndexA, kcolA) && 
            ((afsId==KITS_IMG && binId <= BIN_KIT_LAST) ||
                    afsId==EXPANSION_IMG));
    bool noTeamB = (!FindTeamInGDB(teamIndexB, kcolB) && 
            ((afsId==KITS_IMG && binId <= BIN_KIT_LAST) ||
                    afsId==EXPANSION_IMG));
    if (noTeamA && noTeamB) {
        return false; // not in GDB: rely on afs kit
    }

    // check disabled flag
    if (kcolA->disabled && kcolB->disabled)
    {
        LOG(L"WARN: GDB kits for team %d (%s) are disabled.",
                teamIndexA, kcolA->foldername.c_str());
        LOG(L"WARN: GDB kits for team %d (%s) are disabled.",
                teamIndexB, kcolB->foldername.c_str());
        return false;
    }

    // create the unpacked bin-data in memory
    kserv_buffer_manager_t bm;
    DWORD texSize = sizeof(TEXTURE_ENTRY_HEADER) + 256*sizeof(PALETTE_ENTRY)
        + 1024*512; /*data*/ 
    uLongf unpackedSize = sizeof(UNPACKED_BIN_HEADER) + sizeof(ENTRY_INFO)*2
        + 2*texSize;
    bm.new_unpacked(unpackedSize);
    if (!bm._unpacked) 
    {
        LOG(L"Unable to allocate buffer for binId=%d", binId);
        return false;
    }

    // initialize the bin structure
    bm._unpacked->header.numEntries = 2;
    bm._unpacked->header.unknown1 = 8;
    bm._unpacked->entryInfo[0].offset = 0x20;
    bm._unpacked->entryInfo[0].size = texSize;
    bm._unpacked->entryInfo[0].indexOffset = 0xffffffff;
    bm._unpacked->entryInfo[1].offset = 0x20 + texSize;
    bm._unpacked->entryInfo[1].size = texSize;
    bm._unpacked->entryInfo[1].indexOffset = 0xffffffff;
    for (int i=0; i<2; i++)
    {
        TEXTURE_ENTRY* te = (TEXTURE_ENTRY*)((BYTE*)bm._unpacked 
                + bm._unpacked->entryInfo[i].offset);
        memcpy(&(te->header.sig),"WE00",4);
        te->header.unknown1 = 0;
        te->header.unknown2 = 0;
        te->header.unknown3 = 3;
        te->header.bpp = 8;
        te->header.width = 1024;
        te->header.height = 512;
        te->header.paletteOffset = sizeof(TEXTURE_ENTRY_HEADER);
        te->header.dataOffset = sizeof(TEXTURE_ENTRY_HEADER) 
            + 256*sizeof(PALETTE_ENTRY);
        te->palette[0].r = 0x88; // set 1st color to grey.
        te->palette[0].g = 0x88; // as an indicator in-game that
        te->palette[0].b = 0x88; // the kit didn't load from GDB
        te->palette[0].a = 0xff;
    }

    static int tcount=0;
    tcount++;
    TRACE(L"******** tcount = %d *******", tcount);

    unordered_map<WORD,KIT_PICK>::iterator zit;
    wstring files[2];
    KitCollection* kcols[2];
    kcols[0] = kcolA;
    kcols[1] = kcolB;

    for (int i=0; i<2; i++)
    {
        if (kcols[i]->disabled)
            continue;

        wstring filename(getPesInfo()->gdbDir);
        map<wstring,Kit>::iterator kkit;
        map<wstring,Kit>::iterator kkit_end;
        //LOG(L"filename = {%s}", filename.c_str());
        switch (binType)
        {
            case BIN_KIT_GK:
                kkit_end = kcols[i]->goalkeepers.end();
                kkit = (i==0)?
                    kcols[i]->goalkeepers.find(L"ga"):
                    kcols[i]->goalkeepers.find(L"gb");
                if (kkit == kcols[i]->goalkeepers.end())
                {
                    LOG(L"WARN: %s kit not found!", 
                            ((i==0)?L"ga":L"gb"));
                }
                break;
            case BIN_KIT_PL:
                kkit_end = kcols[i]->players.end();
                zit = _kitPicks.find(slot);
                if (zit != _kitPicks.end())
                {
                    kkit = kcols[i]->players.find(zit->second.kitKey);
                }
                else
                {
                    kkit = (i==0)?
                        kcols[i]->players.find(L"pa"):
                        kcols[i]->players.find(L"pb");
                    if (kkit == kcols[i]->players.end())
                    {
                        LOG(L"WARN: %s kit not found!", 
                                ((i==0)?L"pa":L"pb"));
                    }
                }
                break;
        }
        files[i] = filename;
        if (kkit != kkit_end)
            files[i] += kkit->second.foldername;
        else
            LOG(L"WARN: kit image not found. Using default grey.");
        files[i] += L"\\kit.png";
    }
    ReplaceTexturesInBin(bm._unpacked, files, 2);
    //DumpData(bm._unpacked, unpackedSize);
    
    // pack
    uLongf packedSize = unpackedSize*2; // big buffer just in case;
    bm.new_packed(packedSize);
    int retval = compress((BYTE*)bm._packed->data,&packedSize,(BYTE*)bm._unpacked,unpackedSize);
    if (retval != Z_OK) {
        LOG(L"BIN re-compression failed. retval=%d", retval);
        return false;
    }
    memcpy(bm._packed->header.sig,"\x00\x01\x01WESYS",8);
    bm._packed->header.sizePacked = packedSize;
    bm._packed->header.sizeUnpacked = unpackedSize;
    size = packedSize + 0x10;

    // write the data to a pipe
    HANDLE pipeRead, pipeWrite;
    if (!CreatePipe(&pipeRead, &pipeWrite, NULL, size*2))
    {
        LOG(L"Unable to create a pipe");
        return false;
    }
    DWORD written = 0;
    if (!WriteFile(pipeWrite, bm._packed, size, &written, 0))
    {
        LOG(L"Unable to write to a pipe");
        return false;
    }
    handle = pipeRead;
    return true;
}

/**
 * Create a pipe and write a dynamically created BIN into it.
 */
bool CreatePipeForFontBin(DWORD afsId, DWORD binId, HANDLE& handle, DWORD& size)
{
    if (k_kserv.debug)
        LOG(L"Font-Bin:: Loading binId=%d", binId);

    // first step: determine the team, and see if we have
    // this team in the GDB
    WORD slot = (binId - BIN_FONT_FIRST) >> 2;
    if (afsId==EXPANSION_IMG)
        slot = NUM_SLOTS + ((binId - DT0F_BIN_FONT_FIRST) >> 2);

    int binType = GetBinType(afsId, binId);
    WORD teamIndex = GetTeamIndexBySlot(slot, binType);
    KitCollection* kcol;
    if (!FindTeamInGDB(teamIndex, kcol) && 
            ((afsId==KITS_IMG && binId <= BIN_FONT_LAST) ||
                    afsId==EXPANSION_IMG))
        return false; // not in GDB: rely on afs kit

    // check disabled flag
    if (kcol->disabled)
    {
        LOG(L"WARN: GDB kits for team %d (%s) are disabled.",
                teamIndex, kcol->foldername.c_str());
        return false;
    }

    // create the unpacked bin-data in memory
    kserv_buffer_manager_t bm;
    DWORD texSize = sizeof(TEXTURE_ENTRY_HEADER) + 256*sizeof(PALETTE_ENTRY)
        + 256*64; /*data*/ 
    uLongf unpackedSize = sizeof(UNPACKED_BIN_HEADER) + sizeof(ENTRY_INFO)*2
        + texSize; // we only actually need 1 ENTRY_INFO, but let's have 2 
                   // for nice alignment (header-size then remains 8 DWORDs)
    bm.new_unpacked(unpackedSize);
    if (!bm._unpacked) 
    {
        LOG(L"Unable to allocate buffer for binId=%d", binId);
        return false;
    }

    // initialize the bin structure
    bm._unpacked->header.numEntries = 1;
    bm._unpacked->header.unknown1 = 8;
    bm._unpacked->entryInfo[0].offset = 0x20;
    bm._unpacked->entryInfo[0].size = texSize;
    bm._unpacked->entryInfo[0].indexOffset = 0xffffffff;
    TEXTURE_ENTRY* te = (TEXTURE_ENTRY*)((BYTE*)bm._unpacked 
            + bm._unpacked->entryInfo[0].offset);
    memcpy(&(te->header.sig),"WE00",4);
    te->header.unknown1 = 0;
    te->header.unknown2 = 0;
    te->header.unknown3 = 3;
    te->header.bpp = 8;
    te->header.width = 256;
    te->header.height = 64;
    te->header.paletteOffset = sizeof(TEXTURE_ENTRY_HEADER);
    te->header.dataOffset = sizeof(TEXTURE_ENTRY_HEADER) 
        + 256*sizeof(PALETTE_ENTRY);
    te->palette[0].r = 0xa8; // set 1st color to light grey.
    te->palette[0].g = 0xa8; // as an indicator in-game that
    te->palette[0].b = 0xa8; // the font didn't load from GDB
    te->palette[0].a = 0xff;

    wstring filename(getPesInfo()->gdbDir);
    map<wstring,Kit>::iterator kkit;
    map<wstring,Kit>::iterator kkit_end;
    unordered_map<WORD,KIT_PICK>::iterator zit;
    switch (binType)
    {
        case BIN_FONT_GA:
            kkit_end = kcol->goalkeepers.end();
            kkit = kcol->goalkeepers.find(L"ga");
            break;
        case BIN_FONT_GB:
            kkit_end = kcol->goalkeepers.end();
            kkit = kcol->goalkeepers.find(L"gb");
            break;
        case BIN_FONT_PA:
            kkit_end = kcol->players.end();
            zit = _kitPicks.find(slot);
            if (zit != _kitPicks.end())
            {
                kkit = kcol->players.find(zit->second.kitKey);
            }
            else
            {
                kkit = kcol->players.find(L"pa");
            }
            break;
        case BIN_FONT_PB:
            kkit_end = kcol->players.end();
            zit = _kitPicks.find(slot);
            if (zit != _kitPicks.end())
            {
                kkit = kcol->players.find(zit->second.kitKey);
            }
            else
            {
                kkit = kcol->players.find(L"pb");
            }
            break;
    }
    wstring files[1];
    files[0] = filename;
    if (kkit != kkit_end)
        files[0] += kkit->second.foldername;
    else
        LOG(L"WARN: font image not found. Using default grey.");
    files[0] += L"\\font.png";

    ReplaceTexturesInBin(bm._unpacked, files, 1);
    
    // pack
    uLongf packedSize = unpackedSize*2; // big buffer just in case;
    bm.new_packed(packedSize);
    int retval = compress((BYTE*)bm._packed->data,&packedSize,(BYTE*)bm._unpacked,unpackedSize);
    if (retval != Z_OK) {
        LOG(L"BIN re-compression failed. retval=%d", retval);
        return false;
    }
    memcpy(bm._packed->header.sig,"\x00\x01\x01WESYS",8);
    bm._packed->header.sizePacked = packedSize;
    bm._packed->header.sizeUnpacked = unpackedSize;
    size = packedSize + 0x10;
    //DumpData(bm._packed, size);

    // write the data to a pipe
    HANDLE pipeRead, pipeWrite;
    if (!CreatePipe(&pipeRead, &pipeWrite, NULL, size*2))
    {
        LOG(L"Unable to create a pipe");
        return false;
    }
    DWORD written = 0;
    if (!WriteFile(pipeWrite, bm._packed, size, &written, 0))
    {
        LOG(L"Unable to write to a pipe");
        return false;
    }
    handle = pipeRead;
    return true;
}

/**
 * Create a pipe and write a dynamically created BIN into it.
 */
bool CreatePipeForNumbersBin(DWORD afsId, DWORD binId, HANDLE& handle, DWORD& size)
{
    if (k_kserv.debug)
        LOG(L"Numbers-Bin:: Loading binId=%d", binId);

    // first step: determine the team, and see if we have
    // this team in the GDB
    WORD slot = (binId - BIN_NUMBER_FIRST) >> 2;
    if (afsId==EXPANSION_IMG)
        slot = NUM_SLOTS + ((binId - DT0F_BIN_NUMBER_FIRST) >> 2);

    int binType = GetBinType(afsId, binId);
    WORD teamIndex = GetTeamIndexBySlot(slot, binType);
    KitCollection* kcol;
    if (!FindTeamInGDB(teamIndex, kcol) && 
            ((afsId==KITS_IMG && binId <= BIN_NUMBER_LAST) ||
                    afsId==EXPANSION_IMG))
        return false; // not in GDB: rely on afs kit

    // check disabled flag
    if (kcol->disabled)
    {
        LOG(L"WARN: GDB kits for team %d (%s) are disabled.",
                teamIndex, kcol->foldername.c_str());
        return false;
    }

    // create the unpacked bin-data in memory
    kserv_buffer_manager_t bm;
    DWORD texSize = sizeof(TEXTURE_ENTRY_HEADER) + 256*sizeof(PALETTE_ENTRY)
        + 512*256; /*data*/ 
    uLongf unpackedSize = sizeof(UNPACKED_BIN_HEADER) + sizeof(ENTRY_INFO)*2
        + texSize; // one extra ENTRY_INFO for header padding
    bm.new_unpacked(unpackedSize);
    if (!bm._unpacked) 
    {
        LOG(L"Unable to allocate buffer for binId=%d", binId);
        return false;
    }

    // initialize the bin structure
    bm._unpacked->header.numEntries = 1;
    bm._unpacked->header.unknown1 = 8;
    bm._unpacked->entryInfo[0].offset = 0x20;
    bm._unpacked->entryInfo[0].size = texSize;
    bm._unpacked->entryInfo[0].indexOffset = 0xffffffff;
    TEXTURE_ENTRY* te = (TEXTURE_ENTRY*)((BYTE*)bm._unpacked 
            + bm._unpacked->entryInfo[0].offset);
    memcpy(&(te->header.sig),"WE00",4);
    te->header.unknown1 = 0;
    te->header.unknown2 = 0;
    te->header.unknown3 = 3;
    te->header.bpp = 8;
    te->header.width = 512;
    te->header.height = 256;
    te->header.paletteOffset = sizeof(TEXTURE_ENTRY_HEADER);
    te->header.dataOffset = sizeof(TEXTURE_ENTRY_HEADER) 
        + 256*sizeof(PALETTE_ENTRY);
    te->palette[0].r = 0xc8; // set 1st color to light grey.
    te->palette[0].g = 0xc8; // as an indicator in-game that
    te->palette[0].b = 0xc8; // the numbers didn't load from GDB
    te->palette[0].a = 0xff;

    wstring dirname(getPesInfo()->gdbDir);
    map<wstring,Kit>::iterator kkit;
    map<wstring,Kit>::iterator kkit_end;
    unordered_map<WORD,KIT_PICK>::iterator zit;
    switch (binType)
    {
        case BIN_NUMS_GA:
            kkit_end = kcol->goalkeepers.end();
            kkit = kcol->goalkeepers.find(L"ga");
            break;
        case BIN_NUMS_GB:
            kkit_end = kcol->goalkeepers.end();
            kkit = kcol->goalkeepers.find(L"gb");
            break;
        case BIN_NUMS_PA:
            kkit_end = kcol->players.end();
            zit = _kitPicks.find(slot);
            if (zit != _kitPicks.end())
            {
                kkit = kcol->players.find(zit->second.kitKey);
            }
            else
            {
                kkit = kcol->players.find(L"pa");
            }
            break;
        case BIN_NUMS_PB:
            kkit_end = kcol->players.end();
            zit = _kitPicks.find(slot);
            if (zit != _kitPicks.end())
            {
                kkit = kcol->players.find(zit->second.kitKey);
            }
            else
            {
                kkit = kcol->players.find(L"pb");
            }
            break;

    }
    wstring files[1];
    files[0] = dirname;
    if (kkit != kkit_end)
        files[0] += kkit->second.foldername;
    else
        LOG(L"WARN: numbers image not found. Using default grey.");
    files[0] += L"\\numbers.png";
    ReplaceTexturesInBin(bm._unpacked, files, 1);
    
    // pack
    uLongf packedSize = unpackedSize*2; // big buffer just in case;
    bm.new_packed(packedSize);
    int retval = compress((BYTE*)bm._packed->data,&packedSize,(BYTE*)bm._unpacked,unpackedSize);
    if (retval != Z_OK) {
        LOG(L"BIN re-compression failed. retval=%d", retval);
        return false;
    }
    memcpy(bm._packed->header.sig,"\x00\x01\x01WESYS",8);
    bm._packed->header.sizePacked = packedSize;
    bm._packed->header.sizeUnpacked = unpackedSize;
    size = packedSize + 0x10;
    //DumpData(bm._packed, size);

    // write the data to a pipe
    HANDLE pipeRead, pipeWrite;
    if (!CreatePipe(&pipeRead, &pipeWrite, NULL, size*2))
    {
        LOG(L"Unable to create a pipe");
        return false;
    }
    DWORD written = 0;
    if (!WriteFile(pipeWrite, bm._packed, size, &written, 0))
    {
        LOG(L"Unable to write to a pipe");
        return false;
    }
    handle = pipeRead;
    return true;
}

void ReplaceTexturesInBin(UNPACKED_BIN* bin, wstring files[], size_t n)
{
    for (int i=0; i<n; i++)
    {
        TEXTURE_ENTRY* tex = (TEXTURE_ENTRY*)((BYTE*)bin + bin->entryInfo[i].offset);
        BITMAPINFO* bmp = NULL;
        DWORD texSize = LoadPNGTexture(&bmp, files[i].c_str());
        if (texSize)
        {
            ApplyDIBTexture(tex, bmp);
            FreePNGTexture(bmp);
        }
    }
}

// Load texture from PNG file. Returns the size of loaded texture
DWORD LoadPNGTexture(BITMAPINFO** tex, const wchar_t* filename)
{
    if (k_kserv.debug)
        LOG(L"LoadPNGTexture: loading %s", (wchar_t*)filename);
    DWORD size = 0;

    PNGDIB *pngdib;
    LPBITMAPINFOHEADER* ppDIB = (LPBITMAPINFOHEADER*)tex;

    pngdib = pngdib_p2d_init();
	//TRACE(L"LoadPNGTexture: structure initialized");

    BYTE* memblk;
    int memblksize;
    if(read_file_to_mem(filename,&memblk, &memblksize)) {
        LOG(L"LoadPNGTexture: unable to read PNG file: %s", filename);
        return 0;
    }
    //TRACE(L"LoadPNGTexture: file read into memory");

    pngdib_p2d_set_png_memblk(pngdib,memblk,memblksize);
	pngdib_p2d_set_use_file_bg(pngdib,1);
	pngdib_p2d_run(pngdib);

	//TRACE(L"LoadPNGTexture: run done");
    pngdib_p2d_get_dib(pngdib, ppDIB, (int*)&size);
	//TRACE(L"LoadPNGTexture: get_dib done");

    pngdib_done(pngdib);
	TRACE(L"LoadPNGTexture: done done");

	TRACE(L"LoadPNGTexture: *ppDIB = %08x", (DWORD)*ppDIB);
    if (*ppDIB == NULL) {
        LOG(L"LoadPNGTexture: ERROR - unable to load PNG image.");
        return 0;
    }

    // read transparency values from tRNS chunk
    // and put them into DIB's RGBQUAD.rgbReserved fields
    ApplyAlphaChunk((RGBQUAD*)&((BITMAPINFO*)*ppDIB)->bmiColors, memblk, memblksize);

    HeapFree(GetProcessHeap(), 0, memblk);

	TRACE(L"LoadPNGTexture: done");
	return size;
}

/**
 * Extracts alpha values from tRNS chunk and applies stores
 * them in the RGBQUADs of the DIB
 */
void ApplyAlphaChunk(RGBQUAD* palette, BYTE* memblk, DWORD size)
{
    bool got_alpha = false;

    // find the tRNS chunk
    DWORD offset = 8;
    while (offset < size) {
        PNG_CHUNK_HEADER* chunk = (PNG_CHUNK_HEADER*)(memblk + offset);
        if (chunk->dwName == MAKEFOURCC('t','R','N','S')) {
            int numColors = SWAPBYTES(chunk->dwSize);
            BYTE* alphaValues = memblk + offset + sizeof(chunk->dwSize) + sizeof(chunk->dwName);
            for (int i=0; i<numColors; i++) {
                palette[i].rgbReserved = alphaValues[i];
            }
            got_alpha = true;
            break;
        }
        // move on to next chunk
        offset += sizeof(chunk->dwSize) + sizeof(chunk->dwName) + 
            SWAPBYTES(chunk->dwSize) + sizeof(DWORD); // last one is CRC
    }

    // initialize alpha to all-opaque, if haven't gotten it
    if (!got_alpha) {
        LOG(L"ApplyAlphaChunk::WARNING: no transparency.");
        for (int i=0; i<256; i++) {
            palette[i].rgbReserved = 0xff;
        }
    }
}

// Read a file into a memory block.
static int read_file_to_mem(const wchar_t *fn,unsigned char **ppfiledata, int *pfilesize)
{
	HANDLE hfile;
	DWORD fsize;
	//unsigned char *fbuf;
	BYTE* fbuf;
	DWORD bytesread;

	hfile=CreateFile(fn,GENERIC_READ,FILE_SHARE_READ,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hfile==INVALID_HANDLE_VALUE) return 1;

	fsize=GetFileSize(hfile,NULL);
	if(fsize>0) {
		//fbuf=(unsigned char*)GlobalAlloc(GPTR,fsize);
		//fbuf=(unsigned char*)calloc(fsize,1);
        fbuf = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fsize);
		if(fbuf) {
			if(ReadFile(hfile,(void*)fbuf,fsize,&bytesread,NULL)) {
				if(bytesread==fsize) { 
					(*ppfiledata)  = fbuf;
					(*pfilesize) = (int)fsize;
					CloseHandle(hfile);
					return 0;   // success
				}
			}
			free((void*)fbuf);
		}
	}
	CloseHandle(hfile);
	return 1;  // error
}

// Substitute kit textures with data from DIB
// Currently supports only 4bit and 8bit paletted DIBs
void ApplyDIBTexture(TEXTURE_ENTRY* tex, BITMAPINFO* bitmap)
{
    TRACE(L"Applying DIB texture");

	BYTE* srcTex = (BYTE*)bitmap;
	BITMAPINFOHEADER* bih = &bitmap->bmiHeader;
	DWORD palOff = bih->biSize;
    DWORD numColors = bih->biClrUsed;
    if (numColors == 0)
        numColors = 1 << bih->biBitCount;

    DWORD palSize = numColors*4;
	DWORD bitsOff = palOff + palSize;

    if (bih->biBitCount!=4 && bih->biBitCount!=8)
    {
        LOG(L"ERROR: Unsupported bit-depth. Must be 4- or 8-bit paletted image.");
        return;
    }
    TRACE(L"Loading %d-bit image...", bih->biBitCount);

	// copy palette
	TRACE(L"bitsOff = %08x", bitsOff);
	TRACE(L"palOff  = %08x", palOff);
    memset((BYTE*)&tex->palette, 0, 0x400);
    memcpy((BYTE*)&tex->palette, srcTex + palOff, palSize);
	// swap R and B
	for (int i=0; i<numColors; i++) 
	{
		BYTE blue = tex->palette[i].b;
		BYTE red = tex->palette[i].r;
		tex->palette[i].b = red;
		tex->palette[i].r = blue;
	}
	TRACE(L"Palette copied.");

	int k, m, j, w;
    int width = min(tex->header.width, bih->biWidth); // be safe
    int height = min(tex->header.height, bih->biHeight); // be safe

	// copy pixel data
    if (bih->biBitCount == 8)
    {
        for (k=0, m=bih->biHeight-1; k<height, m>=bih->biHeight - height; k++, m--)
        {
            memcpy(tex->data + k*tex->header.width, srcTex + bitsOff + m*width, width);
        }
    }
    else if (bih->biBitCount == 4)
    {
        for (k=0, m=bih->biHeight-1; k<tex->header.height, m>=bih->biHeight - height; k++, m--)
        {
            for (j=0; j<width/2; j+=1) {
                // expand ech nibble into full byte
                tex->data[k*(tex->header.width) + j*2] = srcTex[bitsOff + m*(bih->biWidth/2) + j] >> 4 & 0x0f;
                tex->data[k*(tex->header.width) + j*2 + 1] = srcTex[bitsOff + m*(bih->biWidth/2) + j] & 0x0f;
            }
        }
    }
	TRACE(L"Texture replaced.");
}

void FreePNGTexture(BITMAPINFO* bitmap) 
{
	if (bitmap != NULL) {
        pngdib_p2d_free_dib(NULL, (BITMAPINFOHEADER*)bitmap);
	}
}

void kservReadNumSlotsCallPoint1()
{
    __asm {
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push edi
        shr eax,2
        push eax // slot
        call kservReadNumSlots
        add esp,4 // pop parameters
        mov esi, eax
        pop edi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

void kservReadNumSlotsCallPoint4()
{
    __asm {
        pushfd 
        push ebp
        push ebx
        push ecx
        push edx
        push esi
        push edi
        sar edx,1
        push edx // slot
        call kservReadNumSlots
        add esp,4 // pop parameters
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop ebp
        popfd
        retn
    }
}

KEXPORT DWORD kservReadNumSlots(DWORD slot)
{
    DWORD* pNumSlots = (DWORD*)dta[NUM_SLOTS_PTR];
    if (slot >= XSLOT_FIRST)
        return XSLOT_LAST+1;
    return *pNumSlots;
}

/**
 * Apply attributes visible on uniform selection screen
 **/
void ApplySomeAttributes(map<wstring,Kit>::iterator kiter, KIT_CHOICE* kc)
{
    // model
    if (kiter->second.attDefined & MODEL)
        kc->model = kiter->second.model;

    // front number
    if (kiter->second.attDefined & FRONT_NUMBER_SHOW)
        kc->frontNumberShow = kiter->second.frontNumberShow;
    if (kiter->second.attDefined & FRONT_NUMBER_SIZE)
        kc->frontNumberSize = kiter->second.frontNumberSize;
    if (kiter->second.attDefined & FRONT_NUMBER_Y)
        kc->frontNumberY = kiter->second.frontNumberY;
    if (kiter->second.attDefined & FRONT_NUMBER_X)
        kc->frontNumberX = kiter->second.frontNumberX;

    // shorts number
    if (kiter->second.attDefined & SHORTS_NUMBER_LOCATION)
        kc->shortsNumberLocation = kiter->second.shortsNumberLocation;
    if (kiter->second.attDefined & SHORTS_NUMBER_SIZE)
        kc->shortsNumberSize = kiter->second.shortsNumberSize;
    if (kiter->second.attDefined & SHORTS_NUMBER_Y)
        kc->shortsNumberY = kiter->second.shortsNumberY;
    if (kiter->second.attDefined & SHORTS_NUMBER_X)
        kc->shortsNumberX = kiter->second.shortsNumberX;

    // radar and shorts colors
    //if (kiter->second.attDefined & MAIN_COLOR)
    //    RGBAColor2KCOLOR(kiter->second.mainColor,
    //            kc->mainColor);
    if (kiter->second.attDefined & SHORTS_MAIN_COLOR)
        RGBAColor2KCOLOR(kiter->second.shortsFirstColor,
                kc->shortsColor);

    // sleeve patch settings
    if (kiter->second.attDefined & SLEEVE_PATCH_RIGHT_POS_SHORT)
        kc->sleevePatchRightPosShort = kiter->second.sleevePatchRightPosShort;
    if (kiter->second.attDefined & SLEEVE_PATCH_RIGHT_POS_LONG)
        kc->sleevePatchRightPosLong = kiter->second.sleevePatchRightPosLong;
    if (kiter->second.attDefined & SLEEVE_PATCH_LEFT_POS_SHORT)
        kc->sleevePatchLeftPosShort = kiter->second.sleevePatchLeftPosShort;
    if (kiter->second.attDefined & SLEEVE_PATCH_LEFT_POS_LONG)
        kc->sleevePatchLeftPosLong = kiter->second.sleevePatchLeftPosLong;
}

bool SameTeams()
{
    if (_kitChoices.home1st && _kitChoices.away1st)
    {
        return _kitChoices.home1st->teamId == _kitChoices.away1st->teamId;
    }
    return false;
}

WORD AcquireXslot(kit_iter_t& iter, WORD teamIndex)
{
    unordered_map<WORD,KIT_PICK>::iterator kpit;
    kpit = _kitPicks.find(iter->second.slot);
    if (kpit == _kitPicks.end() 
            || kpit->second.teamIndex != teamIndex
            || kpit->second.iter != iter)
    {
        // not acquired: get a slot
        WORD slot = GetNextXslot();
        while (slot == _kitChoices.home1st->kitSlotIndex/2
                || slot == _kitChoices.home2nd->kitSlotIndex/2
                || slot == _kitChoices.away1st->kitSlotIndex/2
                || slot == _kitChoices.away2nd->kitSlotIndex/2)
            slot = GetNextXslot();
        // check if overwriting existing entry
        kpit = _kitPicks.find(slot);
        if (kpit != _kitPicks.end())
        {
            kpit->second.iter->second.slot = 0xffff;
            ResetTeamKitInfo(kpit->second.teamIndex, slot);
        }
        KIT_PICK kp;
        kp.kitKey = iter->first;
        kp.teamIndex = teamIndex;
        kp.iter = iter;
        _kitPicks[slot] = kp;
        iter->second.slot = slot;

        if (k_kserv.debug)
            LOG(L"took xslot: 0x%x", slot);
        return slot;
    }
    else
    {
        // already acquired
        return kpit->first;
    }
}

void SwitchKit(SWITCH_KIT& sw, bool advanceIter)
{
    KitCollection* kcol;
    WORD teamIndex = GetTeamIndex(sw.kc);
    if (!FindTeamInGDB(teamIndex, kcol))
        return;
    if (kcol->disabled)
        return;

    if (k_kserv.debug)
        LOG(L"teamIndex = %d", teamIndex);

    if (advanceIter)
    {
        if (sw.iter == sw.iter_end)
            sw.iter = sw.iter_begin;
        else
            sw.iter++;
    }

    WORD paSlot=0, pbSlot=0;
    TEAM_KIT_INFO* teamKitInfo = (TEAM_KIT_INFO*)(
            *(DWORD*)dta[PLAYERS_DATA] 
            + dta[TEAM_KIT_INFO_OFFSET]);
    map<wstring,Kit>::iterator kiter;

    if (sw.iter != sw.iter_end)
    {
        if (k_kserv.debug)
            LOG(L"iter: {%s}", sw.iter->first.c_str());

        // apply attributes and determine the slot
        if (sw._1st) 
        {
            ApplyKitAttributes(sw.iter, teamKitInfo[teamIndex].pa);
            paSlot = AcquireXslot(sw.iter, teamIndex);//sw.iter->second.slot;
        }
        else 
        {
            ApplyKitAttributes(sw.iter, teamKitInfo[teamIndex].pb);
            pbSlot = AcquireXslot(sw.iter, teamIndex);//sw.iter->second.slot;
        }
        kiter = sw.iter;
    }
    else
    {
        if (k_kserv.debug)
            LOG(L"iter: game-choice");

        // apply attributes and determine the slot
        if (sw._1st)
        {
            map<wstring,Kit>::iterator zit = kcol->pa;
            if (zit != kcol->players.end())
            {
                // pa kit exists
                ApplyKitAttributes(zit, teamKitInfo[teamIndex].pa);
                paSlot = zit->second.slot;
            }
            else
            {
                RestoreTeamKitInfo(teamIndex, PA);
            }
            kiter = zit;
        }
        else
        {
            map<wstring,Kit>::iterator zit = kcol->pb;
            if (zit != kcol->players.end())
            {
                // pb kit exists
                ApplyKitAttributes(zit, teamKitInfo[teamIndex].pb);
                pbSlot = zit->second.slot;
            }
            else
            {
                RestoreTeamKitInfo(teamIndex, PB);
            }
            kiter = zit;
        }
    }

    // change the slot
    if (sw._1st)
        teamKitInfo[teamIndex].pa.slot = paSlot;
    else
        teamKitInfo[teamIndex].pb.slot = pbSlot;

    // trigger kit reload
    if (k_kserv.debug)
        LOG(L"slot indices were: %04x,%04x", 
                sw.kc->kitSlotIndex,
                sw.kc->fontNumSlotIndex);

    if (kiter!=kcol->players.end())
        ApplySomeAttributes(kiter, sw.kc);

    if (sw._1st)
    {
        sw.kc->kitSlotIndex = paSlot*2+1;
        sw.kc->fontNumSlotIndex = paSlot*4+2;
    }
    else
    {
        sw.kc->kitSlotIndex = pbSlot*2+1;
        sw.kc->fontNumSlotIndex = pbSlot*4+3;
    }
    sw.kc->reloadFlag = 1;

    if (k_kserv.debug)
        LOG(L"slot indices are: %04x,%04x",
                sw.kc->kitSlotIndex,
                sw.kc->fontNumSlotIndex);
}

WORD GetTeamId(KIT_CHOICE* kc)
{
    // initialize to specified team ID
    WORD teamId = kc->teamId;
    // fine-tune based on slot
    WORD teamIndex = GetTeamIndex(kc);
    if (teamIndex != 0xffff)
        teamId = GetTeamIdByIndex(teamIndex);
    return teamId;
}

WORD GetTeamIndex(KIT_CHOICE* kc)
{
    WORD teamIndex = 0xffff;
    if (kc->isEditedKit)
        return teamIndex;

    unordered_map<WORD,WORD>::iterator it;
    it = _slotMap.find(kc->kitSlotIndex/2);
    if (it != _slotMap.end())
        teamIndex = it->second;
    else
    {
        // maybe an x-slot
        unordered_map<WORD,KIT_PICK>::iterator kpit;
        kpit = _kitPicks.find(kc->kitSlotIndex/2);
        if (kpit != _kitPicks.end())
            teamIndex = kpit->second.teamIndex;
    }
    return teamIndex;
}

void InitIterators()
{
    g_iterHomePA_begin = _gdb->dummyHome.players.end();
    g_iterHomePB_begin = _gdb->dummyHome.players.end();
    g_iterHomeGA_begin = _gdb->dummyHome.goalkeepers.end();
    g_iterHomeGB_begin = _gdb->dummyHome.goalkeepers.end();
    g_iterHomePA_end = _gdb->dummyHome.players.end();
    g_iterHomePB_end = _gdb->dummyHome.players.end();
    g_iterHomeGA_end = _gdb->dummyHome.goalkeepers.end();
    g_iterHomeGB_end = _gdb->dummyHome.goalkeepers.end();

    g_iterAwayPA_begin = _gdb->dummyAway.players.end();
    g_iterAwayPB_begin = _gdb->dummyAway.players.end();
    g_iterAwayGA_begin = _gdb->dummyAway.goalkeepers.end();
    g_iterAwayGB_begin = _gdb->dummyAway.goalkeepers.end();
    g_iterAwayPA_end = _gdb->dummyAway.players.end();
    g_iterAwayPB_end = _gdb->dummyAway.players.end();
    g_iterAwayGA_end = _gdb->dummyAway.goalkeepers.end();
    g_iterAwayGB_end = _gdb->dummyAway.goalkeepers.end();

    g_iterHomePA = g_iterHomePA_end;
    g_iterHomePB = g_iterHomePB_end;
    g_iterHomeGA = g_iterHomeGA_end;
    g_iterHomeGB = g_iterHomeGB_end;
    g_iterAwayPA = g_iterAwayPA_end;
    g_iterAwayPB = g_iterAwayPB_end;
    g_iterAwayGA = g_iterAwayGA_end;
    g_iterAwayGB = g_iterAwayGB_end;
}

void ResetKit(SWITCH_KIT& sw)
{
    // reset kit iterators
    TEAM_KIT_INFO* teamKitInfo = (TEAM_KIT_INFO*)(
            *(DWORD*)dta[PLAYERS_DATA] 
            + dta[TEAM_KIT_INFO_OFFSET]);

    // team index
    WORD teamIndex = GetTeamIndex(sw.kc);

    if (k_kserv.debug)
    {
        LOG(L"ResetKit: team: %d", teamIndex);
        //DumpData(sw.kc, sizeof(KIT_CHOICE));
    }
    TRACE(L"sw.kc = %08x", (DWORD)sw.kc);
    unordered_map<WORD,KitCollection>::iterator it = _gdb->uni.find(teamIndex);
    if (it != _gdb->uni.end())
    {
        if (!it->second.disabled 
                && it->second.players.begin() != it->second.players.end())
        {
            sw.iter_begin = it->second.players.begin();
            sw.iter_end = it->second.players.end();
            while (sw.iter_begin!=sw.iter_end 
                    && (sw.iter_begin->first==L"pa" 
                        || sw.iter_begin->first==L"pb"))
                sw.iter_begin++;

            map<wstring,Kit>::iterator zit;
            if (sw._1st)
            {
                zit = it->second.pa;
                if (zit != sw.iter_end)
                {
                    ApplyKitAttributes(zit, teamKitInfo[teamIndex].pa);
                    teamKitInfo[teamIndex].pa.slot = zit->second.slot;
                }
            }
            else 
            {
                zit = it->second.pb;
                if (zit != sw.iter_end)
                {
                    ApplyKitAttributes(zit, teamKitInfo[teamIndex].pb);
                    teamKitInfo[teamIndex].pb.slot = zit->second.slot;
                }
            }
        }


        // check for player vs. goalkeeper kit
        if (sw.kc->kitSlotIndex % 2 == 1)
        {
            if (sw._1st)
            {
                map<wstring,Kit>::iterator swit;
                swit = it->second.pa;
                if (swit != it->second.players.end())
                {
                    ApplySomeAttributes(swit, sw.kc);
                    sw.kc->kitSlotIndex = 
                            swit->second.slot*2+1;
                    sw.kc->fontNumSlotIndex = 
                            swit->second.slot*4+2;
                    sw.kc->reloadFlag = 1;
                }
            }
            else
            {
                map<wstring,Kit>::iterator swit;
                swit = it->second.pb;
                if (swit != it->second.players.end())
                {
                    ApplySomeAttributes(swit, sw.kc);
                    sw.kc->kitSlotIndex = 
                            swit->second.slot*2+1;
                    sw.kc->fontNumSlotIndex = 
                            swit->second.slot*4+3;
                    sw.kc->reloadFlag = 1;
                }
            }
        }
    }

    sw.iter = sw.iter_end;

    if (k_kserv.debug)
        LOG(L"Kit reset done");
}

void kservReadUniformPickCallPoint()
{
    __asm {
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        mov eax,ebp
        add eax,0x155
        push eax
        call kservReadUniformPick
        add esp,4 // pop arguments
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        mov dl,byte ptr ds:[esi+0x3ee4]  //execute replaced code
        retn
    }
}

KEXPORT void kservReadUniformPick(BYTE* selection)
{
    _home1st = (selection[0]==0);
    _away1st = (selection[4]==0);
}

void kservCheckKitReloadFlagCallPoint()
{
    __asm {
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebx
        call kservCheckKitReloadFlag
        add esp,4 // pop arguments
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        cmp byte ptr ds:[ebx+0x2ba],0  //execute replaced code
        retn
    }
}

KEXPORT void kservCheckKitReloadFlag(KIT_CHOICE* pKC)
{
    if (!pKC)
        return;

    // update our book-keeping structure
    switch (pKC->uniformSelection)
    {
        case 0: // a
            if (pKC->homeAway == 1)
                _kitChoices.home1st = pKC;
            else if (pKC->homeAway == 2)
                _kitChoices.away1st = pKC;
            break;
        case 1: // b
            if (pKC->homeAway == 1)
                _kitChoices.home2nd = pKC;
            else if (pKC->homeAway == 2)
                _kitChoices.away2nd = pKC;
            break;
    }

    // check for kit-reset
    if (pKC->homeAway==1)
    {
        bool _1st = (pKC->uniformSelection == 0);
        WORD teamIndex = GetTeamIndex(pKC);
        if (teamIndex != ((_1st)?_lastHomePA:_lastHomePB))
        {
            SWITCH_KIT sw(pKC,
                    (_1st)?g_iterHomePA:g_iterHomePB,
                    (_1st)?g_iterHomePA_begin:g_iterHomePB_begin,
                    (_1st)?g_iterHomePA_end:g_iterHomePB_end,
                    _1st);
            ResetKit(sw);
            _homeDone = true;
        }
        if (_1st)
            _lastHomePA = teamIndex;
        else
            _lastHomePB = teamIndex;
    }
    else if (pKC->homeAway==2)
    {
        bool _1st = (pKC->uniformSelection == 0);
        WORD teamIndex = GetTeamIndex(pKC);
        bool needsResync =  (_homeDone 
                && _kitChoices.home1st 
                && teamIndex == GetTeamIndex(_kitChoices.home1st));
        if (teamIndex != ((_1st)?_lastAwayPA:_lastAwayPB) || needsResync)
        {
            if (teamIndex != _lastHomePA || _homeDone)
            {
                SWITCH_KIT sw(pKC,
                        (_1st)?g_iterAwayPA:g_iterAwayPB,
                        (_1st)?g_iterAwayPA_begin:g_iterAwayPB_begin,
                        (_1st)?g_iterAwayPA_end:g_iterAwayPB_end,
                        _1st);
                ResetKit(sw);
            }
            else
            {
                // re-sync iterators
                g_iterAwayPA = g_iterHomePA;
                g_iterAwayPB = g_iterHomePB;
                g_iterAwayPA_begin = g_iterHomePA_begin;
                g_iterAwayPB_begin = g_iterHomePB_begin;
                g_iterAwayPA_end = g_iterHomePA_end;
                g_iterAwayPB_end = g_iterHomePB_end;
            }
        }
        if (_1st)
            _lastAwayPA = teamIndex;
        else
            _lastAwayPB = teamIndex;

        if (!_1st)
            _homeDone = false;
    }
}

