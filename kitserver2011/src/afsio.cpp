/* <afsio>
 *
 * This module is the foundation for any AFS-replacement activities.
 * It provides hooks to various points in time, where BINs are being
 * read from the AFS. 
 *
 * (For example, AFS2FS module is a client of this module.)
 */
#define UNICODE
#define THISMOD &k_afsio

#define MODID 123
#ifdef DEBUG
#define NAMELONG L"AFSIO Module 10.0.2.0 (DEBUG)"
#else
#define NAMELONG L"AFSIO Module 10.0.2.0"
#endif
#define NAMESHORT L"AFSIO"
#define DEFAULT_DEBUG 0

#include <windows.h>
#include <stdio.h>
#include <sys/stat.h>
#include "kload_exp.h"
#include "afsio.h"
#include "afsio_addr.h"
#include "dllinit.h"
#include "pngdib.h"
#include "utf8.h"
#include "names.h"
#include "apihijack.h"

#define lang(s) getTransl("afsio",s)

#include <map>
#include <list>
#include <hash_map>
#include <wchar.h>

//#define CREATE_FLAGS FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_NO_BUFFERING
#define CREATE_FLAGS 0

#define MAX_RELPATH 0x108
#define MAX_AFSID 31

struct NEXT_LIKELY_READ
{
    DWORD afsId;
    DWORD offset;
    DWORD localOffset;
    DWORD fileId;
};

class WstringHolder
{
public:
    WstringHolder(const char* lpFileName) : _us(NULL)
    {
        _us = Utf8::utf8ToUnicode((BYTE*)lpFileName);
    }
    ~WstringHolder()
    {
        Utf8::free(_us);
    }
    wchar_t* c_str() { return _us; }
    wchar_t* _us;
};

// VARIABLES
HINSTANCE hInst = NULL;
KMOD k_afsio = {MODID, NAMELONG, NAMESHORT, DEFAULT_DEBUG};

// GLOBALS
hash_map<HANDLE,struct NEXT_LIKELY_READ> _next_likely_reads;
hash_map<DWORD,FILE_STRUCT> g_file_map;
hash_map<DWORD,DWORD> g_offset_map;
hash_map<DWORD,FILE_STRUCT> g_event_map;
hash_map<HANDLE,DWORD> _afsHandles;

DWORD _afsSizes[MAX_AFSID+1];
bool _allHooked(false);
bool _initialized(false);

template <typename T1,typename T2> 
void replace(hash_map<T1,T2>& hm, T1 key, T2 value)
{
    pair<hash_map<T1,T2>::iterator,bool> result = hm.insert(
            pair<T1,T2>(key, value));
    if (!result.second) {
        hm.erase(result.first);
        hm.insert(pair<T1,T2>(key, value));
    }
}

// FUNCTIONS
HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface);

void afsioAtGetBinSizeCallPoint1();
void afsioAtGetBinSizeCallPoint2();
void afsioAtGetBufferSizeCallPoint();
void afsioAtGetImgSizeCallPoint();
void afsioAtGetImgSize2CallPoint();
KEXPORT DWORD afsioAtGetBinSize(DWORD base, DWORD fileId, DWORD orgSize);
KEXPORT DWORD afsioAtGetBufferSize(DWORD afsId, DWORD fileId, DWORD orgSize);
DWORD GetBinSize(DWORD afsId, DWORD fileId, DWORD orgSize);

KEXPORT HANDLE WINAPI Override_CreateFileA(
  LPCSTR lpFileName,
  DWORD dwDesiredAccess,
  DWORD dwShareMode,
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
  DWORD dwCreationDisposition,
  DWORD dwFlagsAndAttributes,
  HANDLE hTemplateFile);

KEXPORT HANDLE WINAPI Override_CreateFileW(
  LPCWSTR lpFileName,
  DWORD dwDesiredAccess,
  DWORD dwShareMode,
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
  DWORD dwCreationDisposition,
  DWORD dwFlagsAndAttributes,
  HANDLE hTemplateFile);

KEXPORT BOOL WINAPI Override_CloseHandle(
  HANDLE hObject);
void afsioConfig(char* pName, const void* pValue, DWORD a);

KEXPORT BOOL STDMETHODCALLTYPE Override_ReadFile(
  HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
  LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);

// callbacks chain
static list<CLBK_GET_FILE_INFO> g_afsio_callbacks;


/*******************/
/* DLL Entry Point */
/*******************/
EXTERN_C BOOL WINAPI DllMain(
        HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		hInst = hInstance;

		RegisterKModule(THISMOD);
		
		if (!checkGameVersion()) {
			LOG(L"Sorry, your game version isn't supported!");
			return true;
		}

        CHECK_KLOAD(MAKELONG(0,10));

		copyAdresses();
		hookFunction(hk_D3D_CreateDevice, initModule);
	}
	
	else if (dwReason == DLL_PROCESS_DETACH)
	{
	}
	
	return true;
}

KEXPORT bool afsioAddCallback(const CLBK_GET_FILE_INFO callback)
{
    g_afsio_callbacks.push_back(callback);
    LOG(L"callback registered.");
    return true;
}

KEXPORT bool afsioRemoveCallback(const CLBK_GET_FILE_INFO callback)
{
    LOG(L"removing callbacks is currently unimplemented.");
    return true;
}

void afsioConfig(char* pName, const void* pValue, DWORD a)
{
	switch (a) {
		case 1: // debug
			k_afsio.debug = *(DWORD*)pValue;
			break;
	}
	return;
}

KEXPORT DWORD GetAfsIdByBase(DWORD base)
{
    DWORD* binSizesTable = (DWORD*)data[BIN_SIZES_TABLE];
    for (DWORD i=0; i<MAX_AFSID+1; i++)
        if (binSizesTable[i]==base)
            return i;
    return 0xffffffff;
}

KEXPORT DWORD GetFileIdByOffset(DWORD afsId, DWORD offset)
{
    DWORD* binSizesTable = (DWORD*)data[BIN_SIZES_TABLE];
    AFS_INFO* afsInfo = (AFS_INFO*)binSizesTable[afsId];
    if (!afsInfo)
        return 0xffffffff; // afs bin-sizes table not available

    DWORD fileId = 0xffffffff;
    DWORD offsetSoFar = afsInfo->startingOffset;
    for (DWORD i=0; i<afsInfo->numItems; i++) {
        DWORD pages = (afsInfo->sizes[i]+0x7ff)>>0x0b;
        if (pages)  // account for 0-size bins
            fileId = i;
        offsetSoFar += pages*0x800;
        if (offsetSoFar > offset)
            return fileId;
    }
    return 0xffffffff;
}

KEXPORT DWORD GetOffsetByFileId(DWORD afsId, DWORD fileId)
{
    DWORD* binSizesTable = (DWORD*)data[BIN_SIZES_TABLE];
    AFS_INFO* afsInfo = (AFS_INFO*)binSizesTable[afsId];
    if (!afsInfo) {
        return 0xffffffff;
    }

    DWORD offset = afsInfo->startingOffset;
    for (DWORD i=0; i<fileId; i++) {
        DWORD pages = (afsInfo->sizes[i]+0x7ff)>>0x0b;
        offset += pages*0x800;
    }
    return offset;
}

KEXPORT bool GetProbableInfoForHandle(DWORD afsId, 
        HANDLE hFile, DWORD offset, DWORD& localOffset, DWORD& fileId)
{
    hash_map<HANDLE,struct NEXT_LIKELY_READ>::iterator nit;
    nit = _next_likely_reads.find(hFile);
    if (nit != _next_likely_reads.end()) {
        if (nit->second.afsId == afsId && nit->second.offset == offset) {
            fileId = nit->second.fileId;
            localOffset = nit->second.localOffset;
            TRACE(
                L"Probable fileId: %d (afsId:%d, offset:0x%08x, hFile:0x%08x)",
                fileId, afsId, offset, hFile);
            _next_likely_reads.erase(nit);
            return true;
        }
        _next_likely_reads.erase(nit);
    }
    fileId = GetFileIdByOffset(afsId, offset);
    TRACE(L"GetProbableInfoForHandle: fileId=%d", fileId);
    if (fileId != -1) {
        localOffset = offset - GetOffsetByFileId(afsId, fileId);
        TRACE(L"GetProbableInfoForHandle: localOffset=%08x", localOffset);
    }
    return false;
}

KEXPORT void SetNextProbableInfoForHandle(DWORD afsId, 
        DWORD offset, DWORD localOffset, DWORD fileId, HANDLE hFile)
{
    struct NEXT_LIKELY_READ nlr;
    nlr.afsId = afsId;
    nlr.offset = offset;
    nlr.localOffset = localOffset;
    nlr.fileId = fileId;
    _next_likely_reads.insert(
        pair<HANDLE,struct NEXT_LIKELY_READ>(hFile, nlr));
}

HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
        D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
        D3DPRESENT_PARAMETERS *pPresentationParameters, 
        IDirect3DDevice9** ppReturnedDeviceInterface) 
{
    if (_initialized) {
        return D3D_OK;
    }
    ZeroMemory(_afsSizes, sizeof(_afsSizes));

    getConfig("afsio", "debug", DT_DWORD, 1, afsioConfig);
    LOG(L"debug = %d", k_afsio.debug);

	//unhookFunction(hk_D3D_CreateDevice, initModule);

    HookCallPoint(code[C_AT_GET_SIZE1], afsioAtGetBinSizeCallPoint1, 6, 2);
    HookCallPoint(code[C_AT_GET_SIZE2], afsioAtGetBinSizeCallPoint2, 6, 2);
    HookCallPoint(code[C_AT_GET_BUFFER_SIZE], 
            afsioAtGetBufferSizeCallPoint, 6, 1);
    HookCallPoint(code[C_AT_GET_IMG_SIZE], 
            afsioAtGetImgSizeCallPoint, 6, 0);
    HookCallPoint(code[C_AT_GET_IMG_SIZE2], 
            afsioAtGetImgSize2CallPoint, 3, 1);

    // api-hooks
    SDLLHook Kernel32Hook = 
    {
        "KERNEL32.DLL",
        false, NULL,		// Default hook disabled, NULL function pointer.
        {
            { "CreateFileA", Override_CreateFileA },
            { "CreateFileW", Override_CreateFileW },
            { "CloseHandle", Override_CloseHandle },
            { "ReadFile", Override_ReadFile },
            { NULL, NULL }
        }
    };
    HookAPICalls( &Kernel32Hook );

	TRACE(L"Hooking done.");

    //__asm { int 3 }          // uncomment this for debugging as needed
    _initialized = true;
    return D3D_OK;
}

KEXPORT HANDLE WINAPI Override_CreateFileA(
  __in      LPCSTR lpFileName,
  __in      DWORD dwDesiredAccess,
  __in      DWORD dwShareMode,
  __in_opt  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
  __in      DWORD dwCreationDisposition,
  __in      DWORD dwFlagsAndAttributes,
  __in_opt  HANDLE hTemplateFile)
{
    HANDLE handle = INVALID_HANDLE_VALUE;

    /*
    if (_strnicmp(lpFileName+strlen(lpFileName)-3,"OPT",3)==0)
    {
        //LOG("CreateFileA: OPTION FILE");
        PFNCREATEOPTION NextCall=NULL;
        for (int i=0;i<(l_CreateOption.num);i++)
        if (l_CreateOption.addr[i]!=0) {
            NextCall=(PFNCREATEOPTION)l_CreateOption.addr[i];
            HANDLE newHandle = NextCall(dwDesiredAccess, 
                                        dwShareMode,
                                        lpSecurityAttributes,
                                        dwCreationDisposition,
                                        dwFlagsAndAttributes,
                                        hTemplateFile);
            if (newHandle != INVALID_HANDLE_VALUE)
            {
                handle = newHandle;
                break;
            }
        } // end-if
    }
    */

    if (handle == INVALID_HANDLE_VALUE) {
        handle = CreateFileA(lpFileName,
                             dwDesiredAccess,
                             dwShareMode,
                             lpSecurityAttributes,
                             dwCreationDisposition,
                             dwFlagsAndAttributes,
                             hTemplateFile);

        const char* shortName = strrchr(lpFileName,'\\');
        if (shortName && lpFileName!=shortName) shortName++;
        size_t nameLen = strlen(shortName);
        bool isImgFile = (nameLen>=4 && 
                _strnicmp(shortName+nameLen-4,".img",4)==0);
        if (isImgFile) {
            DWORD afsId = 0xffffffff;
            if (sscanf(shortName+2,"%02x",&afsId)==1) {
                if (afsId < 0 || afsId > MAX_AFSID) {
                    LOG(L"CreateFile: afsId=%02x out of range. Ignoring",
                            afsId);
                }
                else {
                    replace(_afsHandles, handle, afsId);
                }
            }
        }
    }

    TRACE(L"CreateFileA: {%s} --> %04x",
            WstringHolder(lpFileName).c_str(), 
            (DWORD)handle);

    // api-hooks
    if (!_allHooked) {
        _allHooked = true;
        SDLLHook Kernel32Hook = 
        {
            "KERNEL32.DLL",
            false, NULL,		// Default hook disabled, NULL function pointer.
            {
                { "CloseHandle", Override_CloseHandle },
                { "ReadFile", Override_ReadFile },
                { NULL, NULL }
            }
        };
        HookAPICalls( &Kernel32Hook );
        LOG(L"All api functions hooked");
    }

    return handle;
}

KEXPORT HANDLE WINAPI Override_CreateFileW(
  __in      LPCWSTR lpFileName,
  __in      DWORD dwDesiredAccess,
  __in      DWORD dwShareMode,
  __in_opt  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
  __in      DWORD dwCreationDisposition,
  __in      DWORD dwFlagsAndAttributes,
  __in_opt  HANDLE hTemplateFile)
{
    HANDLE handle = INVALID_HANDLE_VALUE;

    /*
    if (_wcsnicmp(lpFileName+wcslen(lpFileName)-3,L"OPT",3)==0)
    {
        //LOG("CreateFileW: OPTION FILE");
        PFNCREATEOPTION NextCall=NULL;
        for (int i=0;i<(l_CreateOption.num);i++)
        if (l_CreateOption.addr[i]!=0) {
            NextCall=(PFNCREATEOPTION)l_CreateOption.addr[i];
            HANDLE newHandle = NextCall(dwDesiredAccess, 
                                        dwShareMode,
                                        lpSecurityAttributes,
                                        dwCreationDisposition,
                                        dwFlagsAndAttributes,
                                        hTemplateFile);
            if (newHandle != INVALID_HANDLE_VALUE)
            {
                handle = newHandle;
                break;
            }
        } // end-if
    }
    */

    if (handle == INVALID_HANDLE_VALUE) {
        handle = CreateFileW(lpFileName,
                             dwDesiredAccess,
                             dwShareMode,
                             lpSecurityAttributes,
                             dwCreationDisposition,
                             dwFlagsAndAttributes,
                             hTemplateFile);

        const wchar_t* shortName = wcsrchr(lpFileName,L'\\');
        if (shortName && lpFileName!=shortName) shortName++;
        size_t nameLen = wcslen(shortName);
        bool isImgFile = (nameLen>=4 && 
                _wcsnicmp(shortName+nameLen-4,L".img",4)==0);
        if (isImgFile) {
            DWORD afsId = 0xffffffff;
            if (swscanf(shortName+2,L"%02x",&afsId)==1) {
                if (afsId < 0 || afsId > MAX_AFSID) {
                    LOG(L"CreateFile: afsId=%02x out of range. Ignoring",
                            afsId);
                }
                else {
                    replace(_afsHandles, handle, afsId);
                }
            }
        }
    }

    TRACE(L"CreateFileW: {%s} --> %04x", lpFileName, (DWORD)handle);

    // api-hooks
    if (!_allHooked) {
        _allHooked = true;
        SDLLHook Kernel32Hook = 
        {
            "KERNEL32.DLL",
            false, NULL,		// Default hook disabled, NULL function pointer.
            {
                { "CloseHandle", Override_CloseHandle },
                { "ReadFile", Override_ReadFile },
                { NULL, NULL }
            }
        };
        HookAPICalls( &Kernel32Hook );
        LOG(L"All api functions hooked");
    }

    return handle;
}

KEXPORT BOOL WINAPI Override_CloseHandle(
  __in HANDLE hObject)
{
    BOOL result = CloseHandle(hObject);

    _afsHandles.erase(hObject);

    TRACE(L"CloseHandle: {%04x}", hObject);
    return result;
}

DWORD GetImgFileSize(DWORD afsId, HANDLE handle)
{
    if (afsId<0 || afsId>MAX_AFSID)
        return 0;
    if (!_afsSizes[afsId]) {
        _afsSizes[afsId] = GetFileSize(handle,NULL);
    }
    return _afsSizes[afsId];
}

KEXPORT BOOL STDMETHODCALLTYPE Override_ReadFile(
        HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
        LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
    // determine current offset
    DWORD offset = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

    // original read
    TRACE(L"ReadFile: hFile=%04x, buffer=%p, offset=%08x, nb=%08x, %d",
            hFile, lpBuffer, offset, nNumberOfBytesToRead, lpOverlapped);

    // look up the handle
    hash_map<HANDLE,DWORD>::iterator hit = _afsHandles.find(hFile);
    if (hit != _afsHandles.end()) {
        DWORD afsId = hit->second;
        TRACE(L"Reading afsId=%02x", afsId);
        DWORD fileId=-1, localOffset=-1;
        GetProbableInfoForHandle(afsId, hFile, offset, localOffset, fileId);
        if (fileId == -1) {
            // read afs data
            return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead,
                    lpNumberOfBytesRead, lpOverlapped); 
        }

        TRACE(L"Reading afsId=%02x, fileId=%d", afsId, fileId);

        // execute callbacks
        HANDLE myFile = INVALID_HANDLE_VALUE;
        DWORD fsize = 0;
        for (list<CLBK_GET_FILE_INFO>::iterator it = g_afsio_callbacks.begin();
                it != g_afsio_callbacks.end(); 
                it++) {
            if ((*it)(afsId, fileId, myFile, fsize))
                break;  // first successful callback stops chain processing
        }

        if (fsize>0) {
            SetFilePointer(myFile, localOffset, NULL, FILE_BEGIN);
            DWORD bytesRead = 0;
            DWORD bytesToRead = nNumberOfBytesToRead;
            if (fsize < localOffset + bytesToRead) {
                bytesToRead = fsize - localOffset;
            }
            TRACE(L"localOffset=%08x, bytesToRead=%08x",
                    localOffset, bytesToRead);
            ReadFile(
                    myFile, lpBuffer, bytesToRead, &bytesRead, lpOverlapped);
            CloseHandle(myFile);
            if (lpNumberOfBytesRead) {
                *lpNumberOfBytesRead = nNumberOfBytesToRead;
            }

            // just advance the file pointer, we don't need to transfer data
            SetFilePointer(hFile, nNumberOfBytesToRead, NULL, FILE_CURRENT);

            // zero-out remaining bytes
            if (nNumberOfBytesToRead > bytesRead) {
                ZeroMemory(
                        (BYTE*)lpBuffer+bytesRead, 
                        nNumberOfBytesToRead-bytesRead);
            }

            DWORD nextOffset = offset + bytesRead;
            // check for end of AFS-file
            if (offset+*lpNumberOfBytesRead >= GetImgFileSize(afsId, hFile)) {
                TRACE(L"Reached THE END of AFS-file!");
                if (fsize > localOffset + bytesRead) {
                    // more data still to read: push the file pointer back
                    // for the AFS-handle
                    SetFilePointer(hFile, -0x12000, NULL, FILE_CURRENT);
                    nextOffset -= 0x12000;
                }
            }

            // set next likely read
            if (fsize > localOffset + bytesRead) {
                SetNextProbableInfoForHandle(
                    //afsId, nextOffset, localOffset+bytesRead,
                    afsId, offset+bytesRead, localOffset+bytesRead,
                    fileId, hFile);
            }
            return TRUE;
        } 
        else {
            //TRACE(L"No replacement file found.");
        }
    }

    // read afs data
    return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead,
            lpNumberOfBytesRead, lpOverlapped); 
}

void afsioAtGetBinSizeCallPoint1()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd for stack alignment
        pushfd 
        push ebp
        push ebx
        push ecx
        push edx
        push esi
        push edi
        mov eax, dword ptr ds:[esi+edi*4+0x11c] // original code
        push eax // params: org-size in bytes
        push edi // params: file id
        push esi // params: base
        call afsioAtGetBinSize
        add esp,0x0c     // pop parameters
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

void afsioAtGetBinSizeCallPoint2()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd for stack alignment
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push esi
        push edi
        mov edx, dword ptr ds:[esi+edi*4+0x11c] // original code
        push edx // params: org-size in bytes
        push edi // params: file id
        push esi // params: base
        call afsioAtGetBinSize
        add esp,0x0c     // pop parameters
        mov edx, eax
        pop edi
        pop esi
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

KEXPORT DWORD afsioAtGetBinSize(DWORD base, DWORD fileId, DWORD orgSize)
{
    TRACE(L"afsioAtGetBinSize:: base=%p, fileId=%d, orgSize=%08x", 
            base, fileId, orgSize);
    DWORD result = orgSize;
    DWORD afsId = GetAfsIdByBase(base);
    if (afsId == 0xffffffff) {
        return orgSize;
    }

    return GetBinSize(afsId, fileId, orgSize);
}

KEXPORT DWORD afsioAtGetBufferSize(DWORD afsId, DWORD fileId, DWORD orgSize)
{
    TRACE(L"afsioAtGetBufferSize:: afsId=%02x, fileId=%d, orgSize=%08x", 
            afsId, fileId, orgSize);
    DWORD result = orgSize;
    HANDLE hfile = INVALID_HANDLE_VALUE;
    DWORD fsize = 0;

    return GetBinSize(afsId, fileId, orgSize);
}

DWORD GetBinSize(DWORD afsId, DWORD fileId, DWORD orgSize)
{
    DWORD result = orgSize;
    HANDLE hfile = INVALID_HANDLE_VALUE;
    DWORD fsize = 0;

    // execute callbacks
    for (list<CLBK_GET_FILE_INFO>::iterator it = g_afsio_callbacks.begin();
            it != g_afsio_callbacks.end(); 
            it++)
    {
        if ((*it)(afsId, fileId, hfile, fsize))
            break;  // first successful callback stops chain processing
    }

    if (fsize>0) {
        TRACE(L"GetBinSize:: fsize=%08x", fsize);
        result = fsize;
    }

    return result;
}

DWORD GetAfsIdByPathName(char* pathName)
{
    AFS_INFO** ppBST = (AFS_INFO**)data[BIN_SIZES_TABLE];
    for (DWORD afsId=0; afsId<=MAX_AFSID; afsId++)
    {
        if (ppBST[afsId]==0) continue;
        if (strncmp(ppBST[afsId]->filename,pathName,MAX_RELPATH)==0)
            return afsId;
    }
    return 0xffffffff;
}

KEXPORT bool afsioExtendSlots(int afsId, int num_slots)
{
    // extend BIN-sizes table
    AFS_INFO** tabArray = (AFS_INFO**)data[BIN_SIZES_TABLE];
    if (!tabArray)
        return false;
    AFS_INFO* table = tabArray[afsId];
    if (!table)
        return false;

    // check if already enough slots
    if (num_slots <= table->numItems)
        return true;

    int newSize = sizeof(DWORD)*(num_slots)+0x120;
    AFS_INFO* newTable = (AFS_INFO*)HeapAlloc(
            GetProcessHeap(), HEAP_ZERO_MEMORY, newSize);
    memcpy(newTable, table, table->structSize);
    for (int i=table->numItems; i<num_slots; i++)
        newTable->sizes[i] = 0x800; // back-fill with 1 page defaults

    newTable->structSize = newSize;
    newTable->numItems = num_slots;
    newTable->numItems2 = num_slots;
    tabArray[afsId] = newTable; // point to new structure
    return true;
}

void afsioAtGetBufferSizeCallPoint()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd for stack alignment
        pushfd 
        push ebp
        push ebx
        push ecx
        push edx
        push esi
        push edi
        mov edx, dword ptr ds:[ebx+0x10]  // original code
        mov eax, dword ptr ds:[eax+edx*4] // ...
        mov ecx, dword ptr ds:[ebx+0x0c]
        push eax // params: org-size in bytes
        push edx // params: file id
        push ecx // params: afs id
        call afsioAtGetBufferSize
        add esp,0x0c     // pop parameters
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

void afsioAtGetImgSizeCallPoint()
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
        mov edx,0x7ffff800 // maxed-out size effectively
        mov [eax+8],edx    // execute replaced code
        mov dword ptr ds:[edi],edx // ...
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

void afsioAtGetImgSize2CallPoint()
{
    __asm {
        pushfd
        push eax
        mov eax,0x7ffff800   // maxed-out size effectively
        mov dword ptr ds:[esi+0x250],eax
        shr eax,0x0b
        mov dword ptr ds:[esi+0x258],eax
        pop eax
        popfd
        mov ecx, dword ptr ds:[esi+0x24c]  // execute replaced code
        retn
    }
}

