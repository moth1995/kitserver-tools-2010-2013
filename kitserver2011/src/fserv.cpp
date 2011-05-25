/* <fserv>
 *
 */
#define UNICODE
#define THISMOD &k_fserv

#include <windows.h>
#include <stdio.h>
#include <sys/stat.h>
#include "kload_exp.h"
#include "afsio.h"
#include "fserv.h"
#include "fserv_addr.h"
#include "dllinit.h"
#include "configs.h"
#include "configs.hpp"
#include "utf8.h"
#include "player.h"
#include "replay.h"

#define lang(s) getTransl("fserv",s)

#include <map>
#include <list>
#include <hash_map>
#include <wchar.h>

//#define CREATE_FLAGS FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_NO_BUFFERING
#define CREATE_FLAGS 0

#define FIRST_FACE_SLOT 13000
#define NUM_SLOTS 65536
#define FIRST_XFACE_ID 3000
#define FIRST_XHAIR_ID 3001

#define NETWORK_MODE 4

#define MAX_FACE_ID 1500
#define MAX_HAIR_ID 1500
#define MANEKEN_ID 948

// VARIABLES
HINSTANCE hInst = NULL;
KMOD k_fserv = {MODID, NAMELONG, NAMESHORT, DEFAULT_DEBUG};

class fserv_config_t
{
public:
    bool _enable_online;
    fserv_config_t() : _enable_online(false) {}
};

fserv_config_t _fserv_config;

class player_facehair_t
{
public:
    BYTE specialFace;
    BYTE specialHair;
    DWORD faceHairBits;

    player_facehair_t(PLAYER_INFO* p) :
        specialFace(p->specialFace),
        specialHair(p->specialHair),
        faceHairBits(p->faceHairBits)
    {}
};

// GLOBALS
hash_map<DWORD,wstring> _player_face;
hash_map<DWORD,wstring> _player_hair;
hash_map<DWORD,WORD> _player_face_slot;
hash_map<DWORD,WORD> _player_hair_slot;

hash_map<DWORD,player_facehair_t> _saved_facehair;

wstring* _fast_bin_table[NUM_SLOTS-FIRST_FACE_SLOT];

bool _struct_replaced = false;
int _num_slots = 8094;

DWORD _gotFaceBin = 0;
DWORD _gotHairBin = 0;

// FUNCTIONS
HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface);

void fservAtFaceHairCallPoint();
void fservAtCopyEditDataCallPoint();
KEXPORT void fservAtFaceHair(DWORD dest, DWORD src);

void fservAtGetFaceBinCallPoint();
void fservAtGetHairBinCallPoint();
KEXPORT DWORD fservGetFaceBin(DWORD faceId);
KEXPORT DWORD fservGetHairBin(DWORD hairId);

void fservAtResetHairCallPoint();

void fservConfig(char* pName, const void* pValue, DWORD a);
bool fservGetFileInfo(DWORD afsId, DWORD binId, HANDLE& hfile, DWORD& fsize);
bool OpenFileIfExists(const wchar_t* filename, HANDLE& handle, DWORD& size);
void InitMaps();
void fservCopyPlayerData(PLAYER_INFO* players, int place, bool writeList);

void fservWriteEditData(LPCVOID data, DWORD size);
void fservReadReplayData(LPCVOID data, DWORD size);
void fservWriteReplayData(LPCVOID data, DWORD size);

static void string_strip(wstring& s)
{
    static const wchar_t* empties = L" \t\n\r";
    int e = s.find_last_not_of(empties);
    s.erase(e + 1);
    int b = s.find_first_not_of(empties);
    s.erase(0,b);
}

static void string_strip_quotes(wstring& s)
{
    static const wchar_t* empties = L" \t\n\r";
    if (s[s.length()-1]=='"')
        s.erase(s.length()-1);
    if (s[0]=='"')
        s.erase(0,1);
}


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

        //CHECK_KLOAD(MAKELONG(0,11));

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
	switch (a) {
		case 1: // debug
			k_fserv.debug = *(DWORD*)pValue;
			break;
        case 2: // disable-online
            _fserv_config._enable_online = *(DWORD*)pValue == 1;
            break;
	}
	return;
}

void PatchCode(DWORD addr, char* patch)
{
    DWORD newProtection = PAGE_EXECUTE_READWRITE;
    DWORD savedProtection;
    if (VirtualProtect(
            (BYTE*)addr, strlen(patch)*2, newProtection, &savedProtection)) {
        memcpy((BYTE*)addr, patch, strlen(patch));
    }
}

HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface) {

	unhookFunction(hk_D3D_CreateDevice, initModule);

    LOG(L"Initializing Faceserver");

    getConfig("fserv", "debug", DT_DWORD, 1, fservConfig);
    getConfig("fserv", "online.enabled", DT_DWORD, 2, fservConfig);

    _gotFaceBin = code[C_GOT_FACE_BIN];
    _gotHairBin = code[C_GOT_HAIR_BIN];

    HookCallPoint(code[C_CHECK_FACE_AND_HAIR_ID], 
            fservAtFaceHairCallPoint, 3, 27);
            //fservAtFaceHairCallPoint, 6, 27);

    //PatchCode(code[C_CHECK_HAIR_ID], "\x8b\xfe\x90\x90\x90");
    //PatchCode(code[C_CHECK_FACE_ID], "\x8b\xc8\x90\x90\x90");
    HookCallPoint(code[C_GET_FACE_BIN],
            fservAtGetFaceBinCallPoint, 6, 4);
    HookCallPoint(code[C_GET_HAIR_BIN],
            fservAtGetHairBinCallPoint, 6, 0);
    HookCallPoint(code[C_RESET_HAIR],
            fservAtResetHairCallPoint, 6, 2);

    // register callbacks
    afsioAddCallback(fservGetFileInfo);
    addCopyPlayerDataCallback(fservCopyPlayerData);

    addReadReplayDataCallback(fservReadReplayData);
    addWriteReplayDataCallback(fservWriteReplayData);
    addWriteEditDataCallback(fservWriteEditData);

    // initialize face/hair map
    InitMaps();

	TRACE(L"Hooking done.");
    return D3D_OK;
}

void InitMaps()
{
    ZeroMemory(_fast_bin_table, sizeof(_fast_bin_table));

    // process face/hair map file
    hash_map<DWORD,wstring> mapFile;
    wstring mpath(getPesInfo()->gdbDir);
    mpath += L"GDB\\faces\\map.txt";
    if (!readMap(mpath.c_str(), mapFile))
    {
        LOG(L"Couldn't open face/hair-map for reading: {%s}",mpath.c_str());
    }
    else
    {
        for (hash_map<DWORD,wstring>::iterator it = mapFile.begin(); it != mapFile.end(); it++)
        {
            wstring& line = it->second;
            int comma = line.find(',');

            wstring face(line.substr(0,comma));
            string_strip(face);
            if (!face.empty())
                string_strip_quotes(face);

            wstring hair;
            if (comma != string::npos) // hair can be omitted
            {
                hair = line.substr(comma+1);
                string_strip(hair);
                if (!hair.empty())
                    string_strip_quotes(hair);
            }

            //LOG(L"{%d}:",it->first);
            //LOG(L"{%s}/{%s}",face.c_str(),hair.c_str());

            if (!face.empty())
            {
                // check that the file exists, so that we don't crash
                // later, when it's attempted to replace a face.
                wstring filename(getPesInfo()->gdbDir);
                filename += L"GDB\\faces\\" + face;
                HANDLE handle;
                DWORD size;
                if (OpenFileIfExists(filename.c_str(), handle, size))
                {
                    CloseHandle(handle);
                    _player_face.insert(pair<DWORD,wstring>(it->first,face));
                }
                else
                    LOG(L"ERROR in faceserver map for ID = %d: FAILED to open face BIN \"%s\". Skipping", it->first, face.c_str());
            }
            if (!hair.empty())
            {
                // check that the file exists, so that we don't crash
                // later, when it's attempted to replace a hair.
                wstring filename(getPesInfo()->gdbDir);
                filename += L"GDB\\faces\\" + hair;
                HANDLE handle;
                DWORD size;
                if (OpenFileIfExists(filename.c_str(), handle, size))
                {
                    CloseHandle(handle);
                    _player_hair.insert(pair<DWORD,wstring>(it->first,hair));
                }
                else
                    LOG(L"ERROR in faceserver map for ID = %d: FAILED to open hair BIN \"%s\". Skipping", it->first, hair.c_str());
            }
        }
    }

    /*
    DWORD nextSlotPair = FIRST_FACE_SLOT/2;

    // assign slots
    for (hash_map<DWORD,wstring>::iterator it = _player_face.begin();
            it != _player_face.end();
            it++)
    {
        hash_map<DWORD,WORD>::iterator sit = _player_face_slot.find(it->first);
        if (sit != _player_face_slot.end())
            continue; // already has slot
        sit = _player_hair_slot.find(it->first);
        WORD slotId;
        if (sit != _player_hair_slot.end())
            slotId = sit->second-1; // slot already known
        else
        {
            slotId = nextSlotPair*2; // assign new slot
            nextSlotPair++;
        }

        // check whether we ran out of slots
        if (nextSlotPair*2 >= NUM_SLOTS)
        {
            LOG(L"No more slots for faces: ID = %d skipped.", it->first);
            continue;
        }

        _fast_bin_table[slotId - FIRST_FACE_SLOT] = &it->second;
        _player_face_slot.insert(pair<DWORD,WORD>(it->first,slotId));
    }
    for (hash_map<DWORD,wstring>::iterator it = _player_hair.begin();
            it != _player_hair.end();
            it++)
    {
        hash_map<DWORD,WORD>::iterator sit = _player_hair_slot.find(it->first);
        if (sit != _player_hair_slot.end())
            continue; // already has slot
        sit = _player_face_slot.find(it->first);
        WORD slotId;
        if (sit != _player_face_slot.end())
            slotId = sit->second+1; // slot already known
        else
        {
            slotId = nextSlotPair*2+1; // assign new slot
            nextSlotPair++;
        }

        // check whether we ran out of slots
        if (nextSlotPair*2 >= NUM_SLOTS)
        {
            LOG(L"No more slots for hairs: ID = %d skipped.", it->first);
            continue;
        }

        _fast_bin_table[slotId - FIRST_FACE_SLOT] = &it->second;
        _player_hair_slot.insert(pair<DWORD,WORD>(it->first,slotId));
    }

    // initialize total number of BINs
    _num_slots = nextSlotPair*2;
    LOG(L"_num_slots = %d",_num_slots);
    */
}

int GetHairIdRaw(DWORD faceHairBits)
{
    //return faceHairBits & 0x000007ff;
    return faceHairBits & 0x00007fff;
}

int GetFaceIdRaw(DWORD faceHairBits)
{
    //return (faceHairBits >> 15) & 0x000007ff;
    return (faceHairBits >> 15) & 0x00007fff;
}

int GetHairId(PLAYER_INFO* p)
{
    if (IsSpecialHair(p)) {
        // 11 bits [0-10]
        //return p->faceHairBits & 0x000007ff;
        // use 15 bits [0-15]
        return p->faceHairBits & 0x00007fff;
    }
    return -1;
}

int GetFaceId(PLAYER_INFO* p)
{
    if (IsSpecialFace(p)) {
        // 11 bits: [15-25]
        //return (p->faceHairBits >> 15) & 0x000007ff;
        // use 15 bits: [15-30]
        return (p->faceHairBits >> 15) & 0x00007fff;
    }
    return -1;
}

int GetHairBin(int hairId)
{
    if (hairId == -1) {
        return -1;
    }

    DWORD binId = fservGetHairBin(hairId);
    if (binId >= FIRST_FACE_SLOT) {
        return binId;
    }
    return hairId + FIRST_HAIR_BIN - 1;
}

int GetFaceBin(int faceId)
{
    if (faceId == -1) {
        return -1;
    }

    DWORD binId = fservGetFaceBin(faceId);
    if (binId >= FIRST_FACE_SLOT) {
        return binId;
    }
    return faceId + FIRST_FACE_BIN - 1;
}

bool IsSpecialHair(PLAYER_INFO* p)
{
    return (p->specialHair & SPECIAL_HAIR) != 0;
}

bool IsSpecialFace(PLAYER_INFO* p)
{
    return (p->specialFace & SPECIAL_FACE) != 0;
}

void SetSpecialHairBits(DWORD* pFaceHairBits, int hairId)
{
    *pFaceHairBits &= CLEAR_HAIR_MASK;
    *pFaceHairBits |= (hairId & 0x00007fff);
}

void SetSpecialFaceBits(DWORD* pFaceHairBits, int faceId)
{
    *pFaceHairBits &= CLEAR_FACE_MASK;
    *pFaceHairBits |= ((faceId << 15) & 0x3fff0000);
}

void SetSpecialHair(PLAYER_INFO* p, int hairId)
{
    SetSpecialHairBits(&p->faceHairBits, hairId);
    p->specialHair |= SPECIAL_HAIR;
}

void SetSpecialFace(PLAYER_INFO* p, int faceId)
{
    SetSpecialFaceBits(&p->faceHairBits, faceId);
    p->specialFace |= SPECIAL_FACE;
}

void fservCopyPlayerData(PLAYER_INFO* players, int place, bool writeList)
{
    afsioExtendSlots(0x0c, 45000);

    if (place==2)
    {
        //DWORD menuMode = *(DWORD*)data[MENU_MODE_IDX];
        //if (menuMode==NETWORK_MODE && !_fserv_config._enable_online)
        //    return;
    }

    int minFaceId = 2048;
    int maxFaceId = 0;
    int minHairId = 2048;
    int maxHairId = 0;
    hash_map<int,bool> hairsUsed;
    hash_map<int,bool> facesUsed;

    _saved_facehair.clear();
    
    _player_face_slot.clear();
    _player_hair_slot.clear();
    ZeroMemory(_fast_bin_table, sizeof(_fast_bin_table));

    multimap<string,DWORD> mm;
    for (WORD i=0; i<MAX_PLAYERS; i++)
    {
        if (players[i].id == 0) {
            continue;  // no player at this player slot
        }
        if (players[i].name[0] == '\0') {
            continue;  // no player at this player slot
        }

        // assign slots
        hash_map<DWORD,wstring>::iterator sit;
        sit = _player_face.find(players[i].id);
        if (sit != _player_face.end()) {
            DWORD slot = FIRST_FACE_SLOT + i*2;
            _fast_bin_table[slot - FIRST_FACE_SLOT] = &sit->second;
            _player_face_slot.insert(pair<DWORD,WORD>(sit->first,slot));
        }
        sit = _player_hair.find(players[i].id);
        if (sit != _player_hair.end()) {
            DWORD slot = FIRST_FACE_SLOT + i*2 + 1;
            _fast_bin_table[slot - FIRST_FACE_SLOT] = &sit->second;
            _player_hair_slot.insert(pair<DWORD,WORD>(sit->first,slot));
        }

        // save original info
        player_facehair_t facehair(&players[i]);
        _saved_facehair.insert(
            pair<DWORD,player_facehair_t>(i, facehair));

        // clear out extra bits
        players[i].faceHairBits &= CLEAR_EXTENDED_BITS;
        
        hash_map<DWORD,WORD>::iterator it = 
            _player_face_slot.find(players[i].id);
        if (it != _player_face_slot.end())
        {
            LOG(L"player #%d assigned slot (face) #%d",
                    players[i].id,it->second);
            // if not unique face, remember that for later restoring
            //if (!(IsSpecialFace(&players[i]))) {
            //    _non_unique_face.push_back(i);
            //}
            // set new face
            SetSpecialFace(&players[i], 
                it->second - FIRST_FACE_SLOT + FIRST_XFACE_ID);
                //it->second - FIRST_FACE_BIN + 1);
        }
        it = _player_hair_slot.find(players[i].id);
        if (it != _player_hair_slot.end())
        {
            LOG(L"player #%d assigned slot (hair) #%d",
                    players[i].id,it->second);
            // if not unique hair, remember that for later restoring
            //if (!(IsSpecialHair(&players[i]))) {
            //    _non_unique_hair.push_back(i);
            //}
            // set new hair
            SetSpecialHair(&players[i], 
                it->second - FIRST_FACE_SLOT + FIRST_XHAIR_ID);
                //it->second - FIRST_HAIR_BIN + 1);
        }

        /*
        // TEST: assign Drogba's face/hair to second edited player
        if (players[i].id == 0x100000) {
            //SetSpecialFace(&players[i], 248 - FIRST_FACE_BIN + 1);
            SetSpecialHair(&players[i], 3822 - FIRST_HAIR_BIN + 1);

            //players[i].faceHairBits &= CLEAR_FACE_FLAGS;
            //players[i].faceHairBits |= ~CLEAR_FACE_FLAGS;
            //players[i].faceHairBits &= CLEAR_HAIR_FLAGS;
            //players[i].faceHairBits |= ~CLEAR_HAIR_FLAGS;
        }

        // TEST: assign Arshavin's face/hair to second edited player
        if (players[i].id == 0x100001) {
            SetSpecialFace(&players[i], 1084 - FIRST_FACE_BIN + 1);
            SetSpecialHair(&players[i], 4658 - FIRST_HAIR_BIN + 1);

            //players[i].faceHairBits &= CLEAR_FACE_FLAGS;
            //players[i].faceHairBits |= ~CLEAR_FACE_FLAGS;
            //players[i].faceHairBits &= CLEAR_HAIR_FLAGS;
            //players[i].faceHairBits |= ~CLEAR_HAIR_FLAGS;
        }
        */

        if (writeList && players[i].name[0]!='\0') {
            string name(players[i].name);
            mm.insert(pair<string,DWORD>(name,players[i].id));
        }

        int faceId = GetFaceId(&players[i]);
        int hairId = GetHairId(&players[i]);
        if (faceId >= 0) {
            if (faceId < minFaceId) minFaceId = faceId;
            if (faceId > maxFaceId) maxFaceId = faceId;
            facesUsed.insert(pair<int,bool>(faceId,true));
        }
        if (hairId >= 0) {
            if (hairId < minHairId) minHairId = hairId;
            if (hairId > maxHairId) maxHairId = hairId;
            hairsUsed.insert(pair<int,bool>(hairId,true));
        }
        
        faceId = GetFaceIdRaw(players[i].faceHairBits);
        hairId = GetHairIdRaw(players[i].faceHairBits);
        LOG(L"Player (%d): faceHairBits: %08x", i, players[i].faceHairBits);

        int faceBin = GetFaceBin(GetFaceId(&players[i]));
        int hairBin = GetHairBin(GetHairId(&players[i]));
        wchar_t* nameBuf = Utf8::utf8ToUnicode((BYTE*)players[i].name);
        LOG(L"Player (%d): id=%d (id_again=%d): %s (f:%d, h:%d)", 
                i, players[i].id, players[i].id_again, nameBuf,
                faceBin, hairBin);
        Utf8::free(nameBuf);
    }

    LOG(L"face id range: [%d,%d]", minFaceId, maxFaceId);
    LOG(L"hair id range: [%d,%d]", minHairId, maxHairId);
    LOG(L"number of faces used: %d", facesUsed.size());
    LOG(L"number of hairs used: %d", hairsUsed.size());

    if (writeList)
    {
        // write out playerlist.txt
        wstring plist(getPesInfo()->myDir);
        plist += L"\\playerlist.txt";
        FILE* f = _wfopen(plist.c_str(),L"wt");
        if (f)
        {
            for (multimap<string,DWORD>::iterator it = mm.begin();
                    it != mm.end();
                    it++)
                fprintf(f,"%7d : %s\n",it->second,it->first.c_str());
            fclose(f);
        }
    }

    LOG(L"fservCopyPlayerData() done: players updated.");
}

void fservAtResetHairCallPoint()
{
    __asm {
        pushfd
        push eax
        push ebx
        push edx
        lea ebx,[edi]
        mov edx,[edi+4]
        mov ecx,0x10
        rep movs dword ptr es:[edi], dword ptr ds:[esi]
        mov ecx,0xffff87ff
        mov al,byte ptr [ebx+3]
        and al,0x50
        cmp al,0x40
        je done
        cmp al,0x50
        je hback
clear:
        and [ebx+4],ecx
        jmp done
hback:   
        mov ax,dx
        and ax,0x7fff
        mov edx,0xffff8000
        and dword ptr [ebx+4],edx
        or word ptr [ebx+4],ax 
done:
        pop edx
        pop ebx
        pop eax
        popfd
        retn
    }
}

void fservAtGetFaceBinCallPoint()
{
    __asm {
        pushfd
        push ebp
        push ebx
        push ecx
        push edx
        push esi
        push edi
        movzx eax, word ptr ds:[ebx+0x0c]
        push eax // param: face id
        call fservGetFaceBin
        add esp,0x04     // pop parameters
        cmp eax,FIRST_FACE_SLOT
        jae fsface
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop ebp
        popfd
        mov ecx,0x5dc
        retn
fsface: pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop ebp
        popfd
        add eax,0x0c000000
        push eax
        mov eax,_gotFaceBin
        mov [esp+4],eax
        pop eax
        retn
    }
}

void fservAtGetHairBinCallPoint()
{
    __asm {
        pushfd
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push edi
        push esi // param: hair id
        call fservGetHairBin
        add esp,0x04     // pop parameters
        mov esi,eax
        cmp esi,FIRST_FACE_SLOT
        jae fshair
        pop edi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        mov edi,0x5dc
        retn
fshair: pop edi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        add esi,0x0c000000
        mov edi,[esp+4] 
        add esp,4
        push eax
        mov eax,_gotHairBin
        mov [esp+4],eax
        pop eax
        retn
    }
}

void fservAtFaceHairCallPoint()
{
    __asm {
        pushfd 
        //cmp eax,1
        //jae doit
        jmp doit
skip:   mov edx,dword ptr ds:[esi+4]
        shr edx,0x0f
        and edx,0x7ff
        mov word ptr ds:[ecx+0x0c],dx
        mov ax,word ptr ds:[esi+4]
        mov edx,0x7ff
        and ax,dx
        mov word ptr ds:[ecx+4],ax
        popfd
        retn
doit:
        // use 15 bits instead of 11
        // this gives us 32k faces and 32k hairs
        mov edx,dword ptr ds:[esi+4]
        shr edx,0x0f
        and edx,0x7fff
        mov word ptr ds:[ecx+0x0c],dx
        mov ax,word ptr ds:[esi+4]
        mov edx,0x7fff
        and ax,dx
        mov word ptr ds:[ecx+4],ax
        popfd
        retn
/*
doit:   push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push esi // param: src struct
        push ecx // param: dest struct
        call fservAtFaceHair
        add esp,0x08     // pop parameters
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
*/
    }
}

void GetSlotsByPlayerIndex(DWORD idx, DWORD& faceSlot, DWORD& hairSlot)
{
    if (idx >= MAX_PLAYERS)
        return;

    PLAYER_INFO* players = (PLAYER_INFO*)(*(DWORD**)data[EDIT_DATA_PTR] + 1);
    hash_map<DWORD,WORD>::iterator sit;
    sit = _player_face_slot.find(players[idx].id);
    if (sit != _player_face_slot.end())
        faceSlot = sit->second;
    sit = _player_hair_slot.find(players[idx].id);
    if (sit != _player_hair_slot.end())
        hairSlot = sit->second;
}

KEXPORT void fservAtFaceHair(DWORD dest, DWORD src)
{
    //WORD faceId = 20002 - FIRST_FACE_BIN + 1;
    //WORD hairId = 20003 - FIRST_HAIR_BIN + 1;
    
    //WORD faceId = *(WORD*)(src+6);
    //WORD hairId = *(WORD*)(src+4);

    // make sure we take 15 bits for face and 15 - for hair
    WORD faceId = ((*(DWORD*)(src+4))>>15) & 0x7fff;
    WORD hairId = (*(DWORD*)(src+4)) & 0x7fff;

    WORD* pFace = (WORD*)(dest+0x0c);
    WORD* pHair = (WORD*)(dest+4);
    *pFace = faceId;
    *pHair = hairId;
}

/*
KEXPORT void fservAtFaceHair(DWORD dest, DWORD src)
{
    if (!_struct_replaced)
        _struct_replaced = afsioExtendSlots(0, _num_slots);

    BYTE faceHairMask = *(BYTE*)(src+3);
    WORD playerIdx = *(WORD*)(src+0x3a);
    DWORD faceSlot = 0, hairSlot = 0;
    GetSlotsByPlayerIndex(playerIdx, faceSlot, hairSlot);

    WORD* from = (WORD*)(src+6); // face
    WORD* to = (WORD*)(dest+0xe);
    *to = *from & 0x7ff;
    if ((faceHairMask & UNIQUE_FACE) &&
            faceSlot >= FIRST_FACE_SLOT && faceSlot < NUM_SLOTS)
    {
        if (_fast_bin_table[faceSlot - FIRST_FACE_SLOT])
            *to = faceSlot-1408;
    }

    from = (WORD*)(src+4); // hair
    to = (WORD*)(dest+4);
    *to = *from & 0x7ff; 
    if ((faceHairMask & UNIQUE_HAIR) &&
            hairSlot >= FIRST_FACE_SLOT && hairSlot < NUM_SLOTS)
    {
        if (_fast_bin_table[hairSlot - FIRST_FACE_SLOT])
            *to = hairSlot-4449;
    }
}
*/

/**
 */
bool OpenFileIfExists(const wchar_t* filename, HANDLE& handle, DWORD& size)
{
    TRACE(L"OpenFileIfExists:: %s", filename);
    handle = CreateFile(filename,           // file to open
                       GENERIC_READ,          // open for reading
                       FILE_SHARE_READ,       // share for reading
                       NULL,                  // default security
                       OPEN_EXISTING,         // existing file only
                       FILE_ATTRIBUTE_NORMAL | CREATE_FLAGS, // normal file
                       NULL);                 // no attr. template

    if (handle != INVALID_HANDLE_VALUE)
    {
        size = GetFileSize(handle,NULL);
        return true;
    }
    return false;
}

/**
 * AFSIO callback
 */
bool fservGetFileInfo(DWORD afsId, DWORD binId, HANDLE& hfile, DWORD& fsize)
{
    //if (afsId == 0x0c)
    //    LOG(L"Handling BIN: (%02x,%d)", afsId, binId);
    if (afsId != 0x0c || binId < FIRST_FACE_SLOT || binId >= NUM_SLOTS)
        return false;

    wchar_t filename[1024] = {0};
    wstring* pws = _fast_bin_table[binId - FIRST_FACE_SLOT];
    if (pws) 
    {
        LOG(L"Loading face/hair BIN: %d", binId);
        swprintf(filename,L"%sGDB\\faces\\%s", getPesInfo()->gdbDir,
                pws->c_str());
        return OpenFileIfExists(filename, hfile, fsize);
    }
    return false;
}

/**
 * write data callback
 */
void fservWriteEditData(LPCVOID data, DWORD size)
{
    LOG(L"Restoring face/hair settings");

    // restore face/hair settings
    PLAYER_INFO* players = (PLAYER_INFO*)((BYTE*)data + 0x1a0);
    hash_map<DWORD,player_facehair_t>::iterator it;
    for (it = _saved_facehair.begin(); it != _saved_facehair.end(); it++) {
        players[it->first].specialHair = it->second.specialHair;
        players[it->first].specialFace = it->second.specialFace;
        if (!IsSpecialHair(&players[it->first])) {
            // if edited hair --> allow that to be saved
            DWORD justHair = (players[it->first].faceHairBits) & 0x7ff;
            players[it->first].faceHairBits = it->second.faceHairBits;
            players[it->first].faceHairBits &= 0xfffff800;
            players[it->first].faceHairBits |= justHair;
            players[it->first].specialHair &= ~SPECIAL_HAIR;
        }
        else {
            players[it->first].faceHairBits = it->second.faceHairBits;
        }
    }
}

void fservReadReplayData(LPCVOID data, DWORD size)
{
    LOG(L"Reading replay data:");
    REPLAY_DATA* replay = (REPLAY_DATA*)data;
    // check faceId/hairId for players
    // if we have the extended slots mapped, then ok
    // --> otherwise, clear the corresponding bits
    for (int i=0; i<22; i++) {
        // debug
        wchar_t* wname = Utf8::utf8ToUnicode(
            (BYTE*)(replay->payload.players[i].name));
        LOG(L"Player #%02d: %s", i, wname);
        Utf8::free(wname);

        DWORD* pFaceHairBits = &replay->payload.players[i].faceHairBits;

        int faceId = GetFaceIdRaw(*pFaceHairBits);
        if (faceId >= FIRST_XFACE_ID) {
            if (!_fast_bin_table[faceId - FIRST_XFACE_ID]) {
                // do not currently have any players mapped
                // to that slot ==> clear extended bits, use maneken
                *pFaceHairBits = *pFaceHairBits & CLEAR_EXTENDED_FACE_BITS;
                SetSpecialFaceBits(pFaceHairBits, MANEKEN_ID);
                LOG(L"nothing mapped to slot: %d. Skipping", 
                    faceId + FIRST_FACE_SLOT - FIRST_XFACE_ID);
            }
        }
        else {
            // normal bin
            replay->payload.players[i].faceHairBits 
                &= CLEAR_EXTENDED_FACE_BITS;
        }
        int hairId = GetHairIdRaw(*pFaceHairBits);
        if (hairId >= FIRST_XHAIR_ID) {
            if (!_fast_bin_table[hairId - FIRST_XHAIR_ID]) {
                // do not currently have any players mapped
                // to that slot ==> clear extended bits, use maneken
                *pFaceHairBits = *pFaceHairBits & CLEAR_EXTENDED_HAIR_BITS;
                SetSpecialHairBits(pFaceHairBits, MANEKEN_ID);
                LOG(L"nothing mapped to slot: %d. Skipping", 
                    hairId + FIRST_FACE_SLOT - FIRST_XHAIR_ID);
            }
        }
        else {
            // normal bin
            replay->payload.players[i].faceHairBits 
                &= CLEAR_EXTENDED_HAIR_BITS;
        }
    }
}

void fservWriteReplayData(LPCVOID data, DWORD size)
{
    LOG(L"Writing replay data:");
    REPLAY_DATA* replay = (REPLAY_DATA*)data;
    // set kitserver signature
    memcpy(replay->ksSignature, "ks11", 4);
}

KEXPORT DWORD fservGetFaceBin(DWORD faceId)
{
    if (faceId < FIRST_XFACE_ID) {
        return faceId;
    }
    DWORD binId = faceId - FIRST_XFACE_ID + FIRST_FACE_SLOT;
    if (_fast_bin_table[binId - FIRST_FACE_SLOT]) {
        return binId;
    }
    LOG(L"_fast_bin_table[%d - %d] is NULL!", binId, FIRST_FACE_SLOT);
    LOG(L"faceId & 0x7ff = %d", (faceId & 0x7ff));
    return MANEKEN_ID; //faceId & 0x7ff;
}

KEXPORT DWORD fservGetHairBin(DWORD hairId)
{
    if (hairId < FIRST_XHAIR_ID) {
        return hairId;
    }
    DWORD binId = hairId - FIRST_XHAIR_ID + FIRST_FACE_SLOT;
    if (_fast_bin_table[binId - FIRST_FACE_SLOT]) {
        return binId;
    }
    LOG(L"_fast_bin_table[%d - %d] is NULL!", binId, FIRST_FACE_SLOT);
    LOG(L"hairId & 0x7ff = %d", (hairId & 0x7ff));
    return MANEKEN_ID; //hairId & 0x7ff;
}

