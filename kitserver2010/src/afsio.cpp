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
#define NAMELONG L"AFSIO Module 9.2.0.0"
#define NAMESHORT L"AFSIO"
#define DEFAULT_DEBUG 0

#include <windows.h>
#include <stdio.h>
#include <sys/stat.h>
#include "kload_exp.h"
#include "afsio.h"
#include "afsio_addr.h"
#include "dllinit.h"
#include "gdb.h"
#include "pngdib.h"
#include "utf8.h"
#include "names.h"

#define lang(s) getTransl("afsio",s)

#include <map>
#include <list>
#include <unordered_map>
#include <wchar.h>

#define SWAPBYTES(dw) \
    ((dw<<24 & 0xff000000) | (dw<<8  & 0x00ff0000) | \
    (dw>>8  & 0x0000ff00) | (dw>>24 & 0x000000ff))

//#define CREATE_FLAGS FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_NO_BUFFERING
#define CREATE_FLAGS 0

#define MAX_RELPATH 128
#define MAX_AFSID 18

// VARIABLES
HINSTANCE hInst = NULL;
KMOD k_afsio = {MODID, NAMELONG, NAMESHORT, DEFAULT_DEBUG};

// GLOBALS
unordered_map<DWORD,FILE_STRUCT> g_file_map;
unordered_map<DWORD,DWORD> g_offset_map;
unordered_map<DWORD,FILE_STRUCT> g_event_map;

int _num_slots_cv0 = 8094;

// FUNCTIONS
HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface);

void afsioAtGetBinBufferSizeCallPoint();
void afsioAtGetBinBufferSizeCallPoint2();
void afsioAfterGetOffsetPagesCallPoint();
void afsioAtGetSizeCallPoint();
void afsioAfterCreateEventCallPoint();
void afsioAtGetImgSizeCallPoint();
void afsioAtGetImgSize2CallPoint();
void afsioBeforeReadCallPoint();
void afsioAtCloseHandleCallPoint();

KEXPORT DWORD afsioAfterGetBinBufferSize(GET_BIN_SIZE_STRUCT* gbss, DWORD orgSize);
KEXPORT void afsioAfterGetOffsetPages(DWORD offsetPages, DWORD afsId, DWORD binId);
KEXPORT void afsioAtGetSize(DWORD afsId, DWORD binId, DWORD* pSizeBytes, DWORD* pSizePages);
KEXPORT void afsioAfterCreateEvent(DWORD eventId, READ_EVENT_STRUCT* res, char* pathName);
KEXPORT void afsioBeforeRead(READ_STRUCT* rs);
KEXPORT void afsioAtCloseHandle(DWORD eventId);

void afsioConfig(char* pName, const void* pValue, DWORD a);

// callbacks chain
static list<CLBK_GET_FILE_INFO> g_afsio_callbacks;


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
			return true;
		}

        CHECK_KLOAD(MAKELONG(3,7));

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

HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface) {

    getConfig("afsio", "debug", DT_DWORD, 1, afsioConfig);
    LOG1N(L"debug = %d", k_afsio.debug);

	unhookFunction(hk_D3D_CreateDevice, initModule);

    if (getPesInfo()->gameVersion >= gvPES2009demo)
        HookCallPoint(code[C_AT_GET_BINBUFFERSIZE], afsioAtGetBinBufferSizeCallPoint2, 6, 1);
    else
        HookCallPoint(code[C_AT_GET_BINBUFFERSIZE], afsioAtGetBinBufferSizeCallPoint, 6, 1);
    HookCallPoint(code[C_AT_GET_SIZE], afsioAtGetSizeCallPoint, 6, 1);
    HookCallPoint(code[C_AFTER_CREATE_EVENT], afsioAfterCreateEventCallPoint, 6, 3);
    HookCallPoint(code[C_AT_GET_IMG_SIZE], afsioAtGetImgSizeCallPoint, 6, 0);
    HookCallPoint(code[C_AT_GET_IMG_SIZE2], afsioAtGetImgSize2CallPoint, 3, 1);
    HookCallPoint(code[C_AT_CLOSE_HANDLE], afsioAtCloseHandleCallPoint, 6, 3);
    HookCallPoint(code[C_AFTER_GET_OFFSET_PAGES], afsioAfterGetOffsetPagesCallPoint, 6, 1);
    HookCallPoint(code[C_BEFORE_READ], afsioBeforeReadCallPoint, 6, 1);
    //HookCallPoint(code[C_BEFORE_READ2], afsioBeforeReadCallPoint, 6, 1);

	TRACE(L"Hooking done.");

    //__asm { int 3 }          // uncomment this for debugging as needed
    return D3D_OK;
}

void afsioAtGetBinBufferSizeCallPoint()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd, because Windows
        // apparently checks for stack alignment and bad things happen, if it's not
        // DWORD-aligned. (For example, all file operations fail!)
        pushfd 
        push ebp
        push ebx
        push ecx
        push edx
        push esi
        push edi
        mov edx,dword ptr ss:[ebp+0x10]  // execute replaced code
        mov eax,dword ptr ds:[eax+edx*4] // ...
        push eax // org-size
        push ebp 
        call afsioAfterGetBinBufferSize
        add esp,0x08     // pop parameters
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

void afsioAtGetBinBufferSizeCallPoint2()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd, because Windows
        // apparently checks for stack alignment and bad things happen, if it's not
        // DWORD-aligned. (For example, all file operations fail!)
        pushfd 
        push ebp
        push ebx
        push ecx
        push edx
        push esi
        push edi
        mov edx,dword ptr ss:[ebx+0x10]  // execute replaced code
        mov eax,dword ptr ds:[eax+edx*4] // ...
        push eax // org-size
        push ebx 
        call afsioAfterGetBinBufferSize
        add esp,0x08     // pop parameters
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

KEXPORT DWORD afsioAfterGetBinBufferSize(GET_BIN_SIZE_STRUCT* gbss, DWORD orgSize)
{
    DWORD result = orgSize;
    TRACE2N(L"afsioAfterGBBS:: afsId=%d, binId=%d", gbss->afsId, gbss->binId);

    HANDLE hfile = NULL;
    DWORD fsize = 0;

    // execute callbacks
    for (list<CLBK_GET_FILE_INFO>::iterator it = g_afsio_callbacks.begin();
            it != g_afsio_callbacks.end(); 
            it++)
    {
        if ((*it)(gbss->afsId, gbss->binId, hfile, fsize))
            break;  // first successful callback stops chain processing
    }

    if (hfile && fsize)
    {
        DWORD binKey = (gbss->afsId << 16) + gbss->binId;

        // make new entry
        FILE_STRUCT fs;
        fs.hfile = hfile;
        fs.fsize = fsize;
        fs.offset = 0;
        fs.binKey = binKey;

        pair<unordered_map<DWORD,FILE_STRUCT>::iterator,bool> ires =
            g_file_map.insert(pair<DWORD,FILE_STRUCT>(binKey,fs));
        if (!ires.second)
        {
            // replace existing entry
            LOG2N(L"afsioAfterGBBS:: NOTICE: updating existing entry (not a bug): Handles: (new)%04x vs (old)%04x", (DWORD)hfile, (DWORD)ires.first->second.hfile);
            // update existing entry (close old file handle, if different)
            if (hfile != ires.first->second.hfile)
                CloseHandle(ires.first->second.hfile);
            ires.first->second.hfile = hfile;
            ires.first->second.fsize = fsize;
            ires.first->second.offset = 0;
        }

        // modify buffer size
        result = (fsize + 0x7ff) & 0xfffff800;
        TRACE4N(L"afsioAfterGBBS:: afsId=%d, binId=%d, orgSize=%0x, newSize=%0x", gbss->afsId, gbss->binId, orgSize, fsize);
    }
    return result;
}

void afsioAtGetSizeCallPoint()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd, because Windows
        // apparently checks for stack alignment and bad things happen, if it's not
        // DWORD-aligned. (For example, all file operations fail!)
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        mov eax,dword ptr ss:[esp+0x24+0x24] // execute replaced code
        mov dword ptr ds:[eax],edx           // ...
        mov ebx,[esp+0x24+0x08]
        push ecx // pSizePages
        push eax // pSizeBytes
        push edi // binId
        push ebx // afsId
        call afsioAtGetSize
        add esp,0x10     // pop parameters
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

KEXPORT void afsioAtGetSize(DWORD afsId, DWORD binId, DWORD* pSizeBytes, DWORD* pSizePages)
{
    TRACE4N(L"afsioAtGetSize: afsId=%d, binId=%d, sizeBytes=%08x, sizePages=%08x",
            afsId, binId, *pSizeBytes, *pSizePages);
    DWORD binKey = (afsId << 16) + binId;
    unordered_map<DWORD,FILE_STRUCT>::iterator it = g_file_map.find(binKey);
    if (it != g_file_map.end())
    {
        // modify size
        DWORD orgSize = *pSizeBytes;
        //if (it->second.binKey == 0x00f07e2)
        //{
        //    __asm int 3;
        //}
        *pSizeBytes = it->second.fsize;
        *pSizePages = (it->second.fsize + 0x7ff) / 0x800;
        TRACE4N(L"afsioAtGetSize:: (FILE_MAP FOUND) afsId=%d, binId=%d, orgSize=%0x, newSize=%0x", afsId, binId, orgSize, it->second.fsize);
    }
    else
    {
        // This might be a case, where the buffer-size wasn't checked in advance,
        // but we may still have the file to replace. So check for the existence here.
        
        HANDLE hfile = NULL;
        DWORD fsize = 0;

        // execute callbacks
        for (list<CLBK_GET_FILE_INFO>::iterator it = g_afsio_callbacks.begin();
                it != g_afsio_callbacks.end(); 
                it++)
        {
            if ((*it)(afsId, binId, hfile, fsize))
                break;  // first successful callback stops chain processing
        }

        if (hfile && fsize)
        {
            DWORD binKey = (afsId << 16) + binId;

            // make new entry
            FILE_STRUCT fs;
            fs.hfile = hfile;
            fs.fsize = fsize;
            fs.offset = 0;
            fs.binKey = binKey;

            pair<unordered_map<DWORD,FILE_STRUCT>::iterator,bool> ires =
                g_file_map.insert(pair<DWORD,FILE_STRUCT>(binKey,fs));
            if (!ires.second)
            {
                // replace existing entry
                LOG2N(L"afsioAtGetSize:: NOTICE: updating existing entry (not a bug): Handles: (new)%04x vs (old)%04x", (DWORD)hfile, (DWORD)ires.first->second.hfile);
                // update existing entry (close old file handle, if different)
                if (hfile != ires.first->second.hfile)
                    CloseHandle(ires.first->second.hfile);
                ires.first->second.hfile = hfile;
                ires.first->second.fsize = fsize;
                ires.first->second.offset = 0;
            }

            // modify size
            DWORD orgSize = *pSizeBytes;
            *pSizeBytes = fsize;
            *pSizePages = (fsize + 0x7ff) / 0x800;
            TRACE4N(L"afsioAtGetSize:: afsId=%d, binId=%d, orgSize=%0x, newSize=%0x", afsId, binId, orgSize, fsize);
        }
    }
}

DWORD GetAfsIdByPathName(char* pathName)
{
    BIN_SIZE_INFO** ppBST = (BIN_SIZE_INFO**)dta[BIN_SIZES_TABLE];
    for (DWORD afsId=0; afsId<=MAX_AFSID; afsId++)
    {
        if (ppBST[afsId]==0) continue;
        if (strncmp(ppBST[afsId]->relativePathName,pathName,MAX_RELPATH)==0)
            return afsId;
    }
    return 0xffffffff;
}

void afsioAfterGetOffsetPagesCallPoint()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd, because Windows
        // apparently checks for stack alignment and bad things happen, if it's not
        // DWORD-aligned. (For example, all file operations fail!)
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        mov ecx, [esp+0x24+0x08]
        push edi // binId
        push ecx // afsId
        push ebp // offset pages
        call afsioAfterGetOffsetPages
        add esp,0x0c     // pop parameters
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        mov ecx, dword ptr ds:[esi+0x114]
        retn
    }
}

KEXPORT void afsioAfterGetOffsetPages(DWORD offsetPages, DWORD afsId, DWORD binId)
{
    TRACE3N(L"afsAfterGetOffsetPages:: offsetPages=%08x, afsId=%d, binId=%d",
            offsetPages, afsId, binId);

    DWORD binKey = ((offsetPages << 0x0b)&0xfffff800) + afsId;
    g_offset_map[binKey] = binId;  // update existing entry, or insert new
}

void afsioAfterCreateEventCallPoint()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd, because Windows
        // apparently checks for stack alignment and bad things happen, if it's not
        // DWORD-aligned. (For example, all file operations fail!)
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        mov edx, [esp+0x24+0x08]
        mov ecx, [esp+0x24+0x28]
        push edx  // relative img-file pathname
        push ecx  // pointer to READ_EVENT_STRUCT
        push eax  // event id
        call afsioAfterCreateEvent
        add esp,0x0c     // pop parameters
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        test eax,eax                      // execute replaced code
        mov dword ptr ds:[esi+0x27c],eax  // ...
        retn
    }
}

KEXPORT void afsioAfterCreateEvent(DWORD eventId, READ_EVENT_STRUCT* res, char* pathName)
{
    // The task here is to link the eventId with afsId/binId 
    // of the file that is going to be read later.
    //
#ifndef MYDLL_RELEASE_BUILD
    wchar_t* path = Utf8::utf8ToUnicode((BYTE*)pathName);
    TRACE1N1S(L"afsAfterCreateEvent:: eventId=%08x, pathName=%s",eventId,path);
    Utf8::free(path);
#endif

    DWORD afsId = GetAfsIdByPathName(pathName);
    TRACE1N(L"afsAfterCreateEvent:: afsId=%d",afsId);
    DWORD binKey = ((res->offsetPages << 0x0b)&0xfffff800) + afsId;
    TRACE1N(L"afsAfterCreateEvent:: binKey=%08x",binKey);

    unordered_map<DWORD,DWORD>::iterator it = g_offset_map.find(binKey);
    if (it != g_offset_map.end())
    {
        DWORD binId = it->second;

        // lookup FILE_STRUCT
        DWORD binKey1 = (afsId << 16) + binId;
        TRACE3N(L"afsAfterCreateEvent:: looking for binKey1=%08x (afsId=%d, binId=%d)",
                binKey1, afsId, binId);
        unordered_map<DWORD,FILE_STRUCT>::iterator fit = g_file_map.find(binKey1);
        if (fit != g_file_map.end())
        {
            // remember offset for later usage
            fit->second.offset = (res->offsetPages << 0x0b)&0xfffff800;

            // Found: so we have a replacement file handle
            // Now associate the eventId with that struct
            g_event_map.insert(pair<DWORD,FILE_STRUCT>(eventId,fit->second));

            TRACE3N(L"afsAfterCreateEvent:: eventId=%08x (afsId=%d, binId=%d)",eventId, afsId, binId);
        }

        // keep offset map small
        g_offset_map.erase(it);
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
        mov eax,0x7ffff800 // maxed-out size effectively
        mov [edx+8],eax    // execute replaced code
        mov dword ptr ds:[edi],eax // ...
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

void afsioBeforeReadCallPoint()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd, because Windows
        // apparently checks for stack alignment and bad things happen, if it's not
        // DWORD-aligned. (For example, all file operations fail!)
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push esi  // pointer to READ_STRUCT
        call afsioBeforeRead
        add esp,4  // pop parameters
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        sub esp,4                       // execute replaced code
        mov ecx,[esp+4]                 // (note: need to swap the values
        mov [esp],ecx                   // on top two slots on the stack)
        mov ecx,dword ptr ds:[esi+0x34] // ...
        mov [esp+4],ecx                 // ...
        jmp eax                         // ...
    }
}

KEXPORT void afsioBeforeRead(READ_STRUCT* rs)
{
    TRACE1N(L"afsBeforeRead::(%08x) ...", (DWORD)rs->pfrs->eventId); 
    if (rs->pfrs->eventId == 0)
        return;

    unordered_map<DWORD,FILE_STRUCT>::iterator it = g_event_map.find(rs->pfrs->eventId);
    if (it != g_event_map.end())
    {
        TRACE3N(L"afsBeforeRead::(%08x) WAS: hfile=%08x, offset=%08x", 
                (DWORD)rs->pfrs->eventId, (DWORD)rs->pfrs->hfile, rs->pfrs->offset); 
        FILE_STRUCT& fs = it->second;
        rs->pfrs->hfile = fs.hfile;
        rs->pfrs->offset -= fs.offset;
        rs->pfrs->offset_again -= fs.offset;
        TRACE3N(L"afsBeforeRead::(%08x) NOW: hfile=%08x, offset=%08x", 
                (DWORD)rs->pfrs->eventId, (DWORD)rs->pfrs->hfile, rs->pfrs->offset); 

        //if (fs.binKey == 0x00f07e2)
        //{
        //    _asm int 3;
        //}
    }
}

void afsioAtCloseHandleCallPoint()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd, because Windows
        // apparently checks for stack alignment and bad things happen, if it's not
        // DWORD-aligned. (For example, all file operations fail!)
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push eax // handle
        call afsioAtCloseHandle
        add esp,0x04     // pop parameters
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        push edx                          // execute replaced code
        mov edx,[esp+4]                   // (need to swap two numbers on
        mov [esp+4],eax                   // top of the stack, before doing
        mov [esp+8],edx                   // the call to CloseHandle)
        pop edx                           // ...
        call ebx                          // ...
        mov dword ptr ds:[edi+0x27c],esi  // ...
        retn
    }
}

KEXPORT void afsioAtCloseHandle(DWORD eventId)
{
    // reading event is complete. Handle is being closed
    // Do necessary house-keeping tasks, such as removing the eventId
    // from the event map
    TRACE1N(L"afsAtCloseHandle:: eventId=%08x",eventId);

    unordered_map<DWORD,FILE_STRUCT>::iterator it = g_event_map.find(eventId);
    if (it != g_event_map.end())
    {
        FILE_STRUCT& fs = it->second;
        HANDLE hfile = fs.hfile;

        // delete entry in file_map
        unordered_map<DWORD,FILE_STRUCT>::iterator fit = g_file_map.find(fs.binKey);
        if (fit != g_file_map.end())
        {
            if (k_afsio.debug)
                LOG2N(L"Finished read-event for afsId=0x%02x, binId=%d",
                        (fs.binKey >> 16)&0xffff, fs.binKey&0xffff);
            g_file_map.erase(fit);
        }

        // close file handle, and delete from event map
        CloseHandle(hfile);
        g_event_map.erase(it);

    }
}

KEXPORT bool afsioExtendSlots_cv0(int num_slots)
{
    if (num_slots <= _num_slots_cv0)
        return true;

    // extend BIN-sizes table
    BIN_SIZE_INFO** tabArray = (BIN_SIZE_INFO**)dta[BIN_SIZES_TABLE];
    if (!tabArray)
        return false;
    BIN_SIZE_INFO* table = tabArray[0];
    if (!table)
        return false;

    int newSize = sizeof(DWORD)*(num_slots)+0x120;
    BIN_SIZE_INFO* newTable = (BIN_SIZE_INFO*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, newSize);
    memcpy(newTable, table, table->structSize);
    for (int i=table->numItems; i<num_slots; i++)
        newTable->sizes[i] = 0x800; // back-fill with 1 page defaults

    newTable->structSize = newSize;
    newTable->numItems = num_slots;
    newTable->numItems2 = num_slots;
    tabArray[0] = newTable; // point to new structure

    _num_slots_cv0 = num_slots;
    return true;
}

KEXPORT bool afsioExtendSlots(int afsId, int num_slots)
{
    // extend BIN-sizes table
    BIN_SIZE_INFO** tabArray = (BIN_SIZE_INFO**)dta[BIN_SIZES_TABLE];
    if (!tabArray)
        return false;
    BIN_SIZE_INFO* table = tabArray[afsId];
    if (!table)
        return false;

    // check if already enough slots
    if (num_slots <= table->numItems)
        return true;

    int newSize = sizeof(DWORD)*(num_slots)+0x120;
    BIN_SIZE_INFO* newTable = (BIN_SIZE_INFO*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, newSize);
    memcpy(newTable, table, table->structSize);
    for (int i=table->numItems; i<num_slots; i++)
        newTable->sizes[i] = 0x800; // back-fill with 1 page defaults

    newTable->structSize = newSize;
    newTable->numItems = num_slots;
    newTable->numItems2 = num_slots;
    tabArray[afsId] = newTable; // point to new structure
    return true;
}

