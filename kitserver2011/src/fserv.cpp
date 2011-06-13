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
#include "bal.h"

#define lang(s) getTransl("fserv",s)

#include <map>
#include <list>
#include <hash_map>
#include <wchar.h>

//#define CREATE_FLAGS FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_NO_BUFFERING
#define CREATE_FLAGS 0

#define CHECKED_OK 1
#define CHECKED_FAIL 2

#define FIRST_FACE_SLOT 13000
#define NUM_SLOTS 65536
#define FIRST_XFACE_ID 3000
#define FIRST_XHAIR_ID 3001

#define NETWORK_MODE 4

#define FIRST_BAL_ID 0x010000a0
#define NUM_BAL_PLAYERS 20

#define MAX_FACE_ID 1500
#define MAX_HAIR_ID 1500
#define MANEKEN_ID 948

#define REPLAY_SIG "ks11"

// VARIABLES
HINSTANCE hInst = NULL;
KMOD k_fserv = {MODID, NAMELONG, NAMESHORT, DEFAULT_DEBUG};

class wbuffer_t
{
public:
    wbuffer_t(size_t num_chars) 
    {
        _buf = new wchar_t[num_chars];
    }
    ~wbuffer_t() { delete _buf; }
    wchar_t* _buf;
};


class fserv_config_t
{
public:
    bool _check_map_on_load;
    fserv_config_t() : _check_map_on_load(true) {}
};

fserv_config_t _fserv_config;

/*
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
*/

// GLOBALS
hash_map<DWORD,wstring> _player_face;
hash_map<DWORD,wstring> _player_hair;
hash_map<DWORD,WORD> _player_face_slot;
hash_map<DWORD,WORD> _player_hair_slot;

//hash_map<DWORD,player_facehair_t> _saved_facehair;

hash_map<DWORD,BYTE> _saved_facebit;
hash_map<DWORD,BYTE> _saved_hairbit;

wstring* _fast_bin_table[NUM_SLOTS-FIRST_FACE_SLOT];
BYTE _fast_bin_exists[NUM_SLOTS-FIRST_FACE_SLOT];

DWORD _gotFaceBin = 0;
DWORD _gotHairBin = 0;

// FUNCTIONS
HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface);

bool IsSpecialHair(PLAYER_INFO* p);
bool IsSpecialFace(PLAYER_INFO* p);
bool IsSpecialHairByte(BYTE b);
bool IsSpecialFaceByte(BYTE b);

void fservAtFaceHairCallPoint();
void fservAtCopyEditDataCallPoint();
KEXPORT void fservAtFaceHair(DWORD dest, PLAYER_DETAILS* src);

void fservAtUseFaceHairCallPoint();
KEXPORT void fservAtUseFaceHair(DWORD src, PLAYER_DETAILS* dest);
void fservAtOnlineEnterCallPoint();
KEXPORT void fservAtOnlineEnter(DWORD src, DWORD dest, DWORD size);

void fservAtGetFaceBinCallPoint();
void fservAtGetHairBinCallPoint();
KEXPORT DWORD fservGetFaceBin(DWORD faceId);
KEXPORT DWORD fservGetHairBin(DWORD hairId);

void fservAtResetHairCallPoint();
void fservAtResetHair(PLAYER_DETAILS* src, PLAYER_DETAILS* dest, 
                      DWORD numDwords);
void fservAtSquadListCallPoint();
void fservAtSquadList(PLAYER_DETAILS* src, PLAYER_DETAILS* dest, 
                      DWORD numDwords);
void fservAtSetDefaultCallPoint();
void fservAtSetDefault(DWORD src, DWORD dest, DWORD numDwords);

void fservConfig(char* pName, const void* pValue, DWORD a);
bool fservGetFileInfo(DWORD afsId, DWORD binId, HANDLE& hfile, DWORD& fsize);
bool OpenFileIfExists(const wchar_t* filename, HANDLE& handle, DWORD& size);
bool FileExists(const wstring& filename);
bool BinExists(DWORD binId);
void InitMaps();
void fservCopyPlayerData(
    PLAYER_INFO* players, DWORD numBytes, int place, bool writeList);

void fservReadEditData(LPCVOID data, DWORD size);
void fservWriteEditData(LPCVOID data, DWORD size);
void fservReadReplayData(LPCVOID data, DWORD size);
void fservWriteReplayData(LPCVOID data, DWORD size);
void fservReadBalData(LPCVOID data, DWORD size);
void fservWriteBalData(LPCVOID data, DWORD size);

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
            _fserv_config._check_map_on_load = *(DWORD*)pValue == 1;
            LOG(L"map-check: %d", _fserv_config._check_map_on_load);
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
    getConfig("fserv", "map-check.enabled", DT_DWORD, 2, fservConfig);

    _gotFaceBin = code[C_GOT_FACE_BIN];
    _gotHairBin = code[C_GOT_HAIR_BIN];

    HookCallPoint(code[C_CHECK_FACE_AND_HAIR_ID], 
            fservAtFaceHairCallPoint, 6, 1);

    //PatchCode(code[C_CHECK_HAIR_ID], "\x8b\xfe\x90\x90\x90");
    //PatchCode(code[C_CHECK_FACE_ID], "\x8b\xc8\x90\x90\x90");
    HookCallPoint(code[C_GET_FACE_BIN],
            fservAtGetFaceBinCallPoint, 6, 4);
    HookCallPoint(code[C_GET_HAIR_BIN],
            fservAtGetHairBinCallPoint, 6, 0);
    HookCallPoint(code[C_RESET_HAIR],
            fservAtResetHairCallPoint, 6, 2);
    //HookCallPoint(code[C_SQUAD_LIST],
    //        fservAtSquadListCallPoint, 6, 2);
    //HookCallPoint(code[C_SET_DEFAULT_PLAYER],
    //        fservAtSetDefaultCallPoint, 6, 2);
    HookCallPoint(code[C_FACEHAIR_READ],
            fservAtUseFaceHairCallPoint, 6, 2);
    HookCallPoint(code[C_ONLINE_ENTER],
            fservAtOnlineEnterCallPoint, 6, 2);

    // register callbacks
    afsioAddCallback(fservGetFileInfo);
    addCopyPlayerDataCallback(fservCopyPlayerData);

    addReadReplayDataCallback(fservReadReplayData);
    addReadBalDataCallback(fservReadBalData);
    addReadEditDataCallback(fservReadEditData);
    addWriteReplayDataCallback(fservWriteReplayData);
    addWriteEditDataCallback(fservWriteEditData);
    addWriteBalDataCallback(fservWriteBalData);

    // initialize face/hair map
    InitMaps();

	TRACE(L"Hooking done.");
    return D3D_OK;
}

void InitMaps()
{
    ZeroMemory(_fast_bin_exists, sizeof(_fast_bin_exists));
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
                wstring filename(getPesInfo()->gdbDir);
                filename += L"GDB\\faces\\" + face;
                if (_fserv_config._check_map_on_load) {
                    if (FileExists(filename)) {
                        _player_face.insert(
                            pair<DWORD,wstring>(it->first,face));
                    }
                    else {
                        LOG(L"ERROR in faceserver map for ID = %d: "
                            L"FAILED to open face BIN \"%s\". Skipping", 
                            it->first, face.c_str());
                    }
                }
                else {
                    // insert blindly: will check later, when
                    // BIN is loaded
                    _player_face.insert(
                        pair<DWORD,wstring>(it->first,face));
                }
            }
            if (!hair.empty())
            {
                // check that the file exists, so that we don't crash
                // later, when it's attempted to replace a hair.
                wstring filename(getPesInfo()->gdbDir);
                filename += L"GDB\\faces\\" + hair;
                if (_fserv_config._check_map_on_load) {
                    if (FileExists(filename)) {
                        _player_hair.insert(
                            pair<DWORD,wstring>(it->first,hair));
                    }
                    else {
                        LOG(L"ERROR in faceserver map for ID = %d: "
                            L"FAILED to open hair BIN \"%s\". Skipping", 
                            it->first, hair.c_str()); 
                    }
                }
                else {
                    // insert blindly: will check later, when
                    // BIN is loaded
                    _player_hair.insert(
                        pair<DWORD,wstring>(it->first,hair));
                }
            }
        }
    }
}

WORD GetHairId(DWORD faceHairBits)
{
    return faceHairBits & 0x7ff;
}

WORD GetFaceId(DWORD faceHairBits)
{
    return (faceHairBits >> 15) & 0x7ff;
}

int GetHairBin(PLAYER_INFO* p)
{
    if (!IsSpecialHair(p)) {
        return -1;
    }
    return GetHairId(p->faceHairBits);
}

int GetFaceBin(PLAYER_INFO* p)
{
    if (!IsSpecialFace(p)) {
        return -1;
    }
    return GetFaceId(p->faceHairBits);
}

bool IsSpecialHair(PLAYER_INFO* p)
{
    return (p->specialHair & SPECIAL_HAIR) != 0;
}

bool IsSpecialFace(PLAYER_INFO* p)
{
    return (p->specialFace & SPECIAL_FACE) != 0;
}

bool IsSpecialHairByte(BYTE b)
{
    return (b & SPECIAL_HAIR) != 0;
}

bool IsSpecialFaceByte(BYTE b)
{
    return (b & SPECIAL_FACE) != 0;
}

void SetSpecialHair(PLAYER_INFO* p)
{
    p->specialHair |= SPECIAL_HAIR;
}

void SetSpecialFace(PLAYER_INFO* p)
{
    p->specialFace |= SPECIAL_FACE;
}

void fservCopyPlayerData(
    PLAYER_INFO* players, DWORD numBytes, int place, bool writeList)
{
    afsioExtendSlots(0x0c, 45000);
    //LOG(L"COPY-DATA: # players = %d", numBytes/sizeof(PLAYER_INFO));

    if (place==2) {
        return;
    }

    int minFaceId = 2048;
    int maxFaceId = 0;
    int minHairId = 2048;
    int maxHairId = 0;
    hash_map<int,bool> hairsUsed;
    hash_map<int,bool> facesUsed;

    //_saved_facehair.clear();
    _saved_facebit.clear();
    _saved_hairbit.clear();
    
    _player_face_slot.clear();
    _player_hair_slot.clear();
    ZeroMemory(_fast_bin_exists, sizeof(_fast_bin_exists));
    ZeroMemory(_fast_bin_table, sizeof(_fast_bin_table));

    bool indexTaken(false);
    WORD numPlayers = (numBytes)?(numBytes/sizeof(PLAYER_INFO)):MAX_PLAYERS;

    multimap<string,DWORD> mm;
    for (WORD i=0; i<numPlayers; i++)
    {
        // assign index
        players[i].index = i;

        if (players[i].id == 0) {
            continue;  // no player at this player slot
        }
        if (players[i].name[0] == '\0') {
            continue;  // no player at this player slot
        }

        bool modified(false);

        // assign slots
        hash_map<DWORD,wstring>::iterator sit;
        sit = _player_face.find(players[i].id);
        if (sit != _player_face.end()) {
            DWORD slot = FIRST_FACE_SLOT + i*2;
            _fast_bin_table[slot - FIRST_FACE_SLOT] = &sit->second;
            _player_face_slot.insert(pair<DWORD,WORD>(sit->first,slot));

            // save original face bit
            _saved_facebit.insert(pair<DWORD,BYTE>(
                i, players[i].specialFace & SPECIAL_FACE));
            
        }
        sit = _player_hair.find(players[i].id);
        if (sit != _player_hair.end()) {
            DWORD slot = FIRST_FACE_SLOT + i*2 + 1;
            _fast_bin_table[slot - FIRST_FACE_SLOT] = &sit->second;
            _player_hair_slot.insert(pair<DWORD,WORD>(sit->first,slot));

            // save original hair bit
            _saved_hairbit.insert(pair<DWORD,BYTE>(
                i, players[i].specialHair & SPECIAL_HAIR));
        }

        hash_map<DWORD,WORD>::iterator it = 
            _player_face_slot.find(players[i].id);
        if (it != _player_face_slot.end())
        {
            LOG(L"player %d assigned slot (face) #%d (index=0x%x)",
                    players[i].id,it->second, i);
            // if not unique face, remember that for later restoring
            //if (!(IsSpecialFace(&players[i]))) {
            //    _non_unique_face.push_back(i);
            //}
            // set new face
            SetSpecialFace(&players[i]);
        }
        it = _player_hair_slot.find(players[i].id);
        if (it != _player_hair_slot.end())
        {
            LOG(L"player %d assigned slot (hair) #%d (index=0x%x)",
                    players[i].id,it->second, i);
            // if not unique hair, remember that for later restoring
            //if (!(IsSpecialHair(&players[i]))) {
            //    _non_unique_hair.push_back(i);
            //}
            // set new hair
            SetSpecialHair(&players[i]);
        }

        if (writeList && players[i].name[0]!='\0') {
            string name(players[i].name);
            mm.insert(pair<string,DWORD>(name,players[i].id));
        }

        /*
        WORD faceId = GetFaceId(players[i].faceHairBits);
        WORD hairId = GetHairId(players[i].faceHairBits);
        if (IsSpecialFace(&players[i])) {
            if (faceId < minFaceId) minFaceId = faceId;
            if (faceId > maxFaceId) maxFaceId = faceId;
            facesUsed.insert(pair<WORD,bool>(faceId,true));
        }
        if (IsSpecialHair(&players[i])) {
            if (hairId < minHairId) minHairId = hairId;
            if (hairId > maxHairId) maxHairId = hairId;
            hairsUsed.insert(pair<WORD,bool>(hairId,true));
        }
        */
        
        //LOG(L"Player (%d): faceHairBits: %08x", i, players[i].faceHairBits);

        int faceBin = GetFaceBin(&players[i]);
        int hairBin = GetHairBin(&players[i]);
        //wchar_t* nameBuf = Utf8::utf8ToUnicode((BYTE*)players[i].name);
        //LOG(L"Player (%d): id=%d (id_again=%d): %s (f:%d, h:%d), index=%04x", 
        //        i, players[i].id, players[i].id_again, nameBuf,
        //        faceBin, hairBin, players[i].index);
        //Utf8::free(nameBuf);

        //if (players[i].index != 0) {
        //    indexTaken = true;
        //}
    }

    // BAL players
    for (DWORD id=FIRST_BAL_ID; id<FIRST_BAL_ID+NUM_BAL_PLAYERS; id++) {
        // assign slots
        hash_map<DWORD,wstring>::iterator sit;
        sit = _player_face.find(id);
        if (sit != _player_face.end()) {
            DWORD slot = FIRST_FACE_SLOT + (MAX_PLAYERS+id-FIRST_BAL_ID)*2;
            _fast_bin_table[slot - FIRST_FACE_SLOT] = &sit->second;
            _player_face_slot.insert(pair<DWORD,WORD>(sit->first,slot));
            LOG(L"BAL player %d assigned slot (face) #%d (index=0x%x)",
                    id, slot, (slot-FIRST_FACE_SLOT)/2);
        }
        sit = _player_hair.find(id);
        if (sit != _player_hair.end()) {
            DWORD slot = FIRST_FACE_SLOT + (MAX_PLAYERS+id-FIRST_BAL_ID)*2 + 1;
            _fast_bin_table[slot - FIRST_FACE_SLOT] = &sit->second;
            _player_hair_slot.insert(pair<DWORD,WORD>(sit->first,slot));
            LOG(L"BAL player %d assigned slot (hair) #%d (index=0x%x)",
                    id, slot, (slot-FIRST_FACE_SLOT)/2);
        }
    }

    //LOG(L"indexTaken=%d", indexTaken);
    //LOG(L"face id range: [%d,%d]", minFaceId, maxFaceId);
    //LOG(L"hair id range: [%d,%d]", minHairId, maxHairId);
    //LOG(L"number of faces used: %d", facesUsed.size());
    //LOG(L"number of hairs used: %d", hairsUsed.size());
    LOG(L"number of GDB faces used: %d", _player_face_slot.size());
    LOG(L"number of GDB hairs used: %d", _player_hair_slot.size());

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

    LOG(L"fservCopyPlayerData() done: players (%p) updated.",
        players);
}

void fservAtOnlineEnterCallPoint()
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
        mov edi,[esp+0x24]
        mov esi,[esp+0x28]
        mov ecx,[esp+0x2c]
        push ecx // param: size
        push edi // param: dest
        push esi // param: src
        call fservAtOnlineEnter
        add esp,0x0c // pop params
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        push eax
        mov eax,[esp+4]
        mov [esp+0x10],eax
        pop eax
        add esp,0x0c
        add dword ptr ss:[esp+0x28],esi
        retn
    }
}

void fservAtUseFaceHairCallPoint()
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
        push eax  // param: dest
        push ecx  // param: src
        call fservAtUseFaceHair
        add esp,0x08 // pop params
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        xor word ptr ds:[eax+0x3c],dx  // modified code
        movzx edx,byte ptr ds:[ecx+8]   // ...
        retn
    }
}

void fservAtResetHairCallPoint()
{
    __asm {
        pushfd
        push ebp
        push eax
        push ebx
        push edx
        push esi
        push edi
        mov ecx,0x10
        push ecx  // param: dword count
        push edi  // param: dest
        push esi  // param: src
        call fservAtResetHair
        add esp,0x0c // pop params
        pop edi
        add edi,0x40
        pop esi
        add esi,0x40
        mov ecx,0
        pop edx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

void fservAtSquadListCallPoint()
{
    __asm {
        pushfd
        push ebp
        push eax
        push ebx
        push edx
        push esi
        push edi
        mov ecx,0x10
        push ecx  // param: dword count
        push edi  // param: dest
        push esi  // param: src
        call fservAtSquadList
        add esp,0x0c // pop params
        pop edi
        add edi,0x40
        pop esi
        add esi,0x40
        mov ecx,0
        pop edx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

void fservAtSetDefaultCallPoint()
{
    __asm {
        pushfd
        push ebp
        push eax
        push ebx
        push edx
        push esi
        push edi
        mov ecx,0x1b
        push ecx  // param: dword count
        push edi  // param: dest
        push esi  // param: src
        call fservAtSetDefault
        add esp,0x0c // pop params
        pop edi
        add edi,0x6c
        pop esi
        add esi,0x6c
        mov ecx,0
        pop edx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

void fservAtOnlineEnter(DWORD src, DWORD dest, DWORD size)
{
    //LOG(L"size = %x", size);
    if (!*(DWORD*)data[EDIT_DATA_PTR]) {
        return;
    }

    DWORD playerBase = *(DWORD*)data[EDIT_DATA_PTR]+8;
    PLAYER_INFO* players = (PLAYER_INFO*)playerBase;

    if (playerBase <= src && 
            src < playerBase+MAX_PLAYERS*sizeof(PLAYER_INFO)) {
        // inside player data
        WORD index = (src - playerBase)/sizeof(PLAYER_INFO);
        PLAYER_INFO* pSrc = &players[index];
        PLAYER_INFO* pDest = (PLAYER_INFO*)(dest - (src - (DWORD)pSrc));
        //LOG(L"player #%d (src=%p, dest=%p)", index, src, dest);

        if (pSrc->index != 0) {
            //LOG(L"pSrc->index = %d", pSrc->index);
            //LOG(L"online reset for player index %d", pSrc->index);
            //
            DWORD addr = (DWORD)&(pDest->index);
            if (dest <= addr && addr < dest+size) {
                pDest->index = 0;
            }

            hash_map<DWORD,BYTE>::iterator it;
            it = _saved_facebit.find(pSrc->index);
            if (it != _saved_facebit.end()) {
                addr = (DWORD)&(pDest->specialFace);
                if (dest <= addr && addr < dest+size) {
                    pDest->specialFace &= ~SPECIAL_FACE;
                    pDest->specialFace |= it->second;
                }
            }
            it = _saved_hairbit.find(pSrc->index);
            if (it != _saved_hairbit.end()) {
                addr = (DWORD)&(pDest->specialHair);
                if (dest <= addr && addr < dest+size) {
                    pDest->specialHair &= ~SPECIAL_HAIR;
                    pDest->specialHair |= it->second;
                }
            }

            /*
            hash_map<DWORD,player_facehair_t>::iterator it;
            it = _saved_facehair.find(pSrc->index);
            if (it != _saved_facehair.end()) {
                //LOG(L"index/face/hair: %d/%02x/%02x --> %d/%02x/%02x",
                //        pSrc->index,
                //        pSrc->specialFace, pSrc->specialHair,
                //        0,
                //        it->second.specialFace,
                //        it->second.specialHair);

                DWORD addr;
                addr = (DWORD)&(pDest->specialFace);
                if (dest <= addr && addr < dest+size) {
                    pDest->specialFace = it->second.specialFace;
                }
                addr = (DWORD)&(pDest->specialHair);
                if (dest <= addr && addr < dest+size) {
                    pDest->specialHair = it->second.specialHair;
                }
                addr = (DWORD)&(pDest->index);
                if (dest <= addr && addr < dest+size) {
                    pDest->index = 0;
                }
            }
            */
        }
    }
}

void fservAtUseFaceHair(DWORD src, PLAYER_DETAILS* dest)
{
    WORD faceId = *(WORD*)(src+0x0c);
    WORD hairId = *(WORD*)(src+4);

    if (faceId >= FIRST_XFACE_ID) {
        dest->index = (faceId - FIRST_XFACE_ID)/2;
    }
    else if (hairId >= FIRST_XHAIR_ID) {
        dest->index = (hairId - FIRST_XHAIR_ID)/2;
    }
}

void fservAtResetHair(PLAYER_DETAILS* src, PLAYER_DETAILS* dest, 
                      DWORD numDwords)
{
    //LOG(L"src/dest: (%02x,%02x,%08x)/(%02x,%02x,%08x)", 
    //    src->specialHair, src->specialFace,
    //    src->faceHairBits, dest->specialHair,
    //    dest->specialFace, dest->faceHairBits);

    // enforce special hair bit, if needed
    if (src->index != 0) {
        if (_fast_bin_table[src->index*2 + 1]) {
            //LOG(L"setting special hair bit");
            src->specialHair |= SPECIAL_HAIR;
        }
        if (_fast_bin_table[src->index*2]) {
            //LOG(L"setting special face bit");
            src->specialFace |= SPECIAL_FACE;
        }
    }

    // execute game logic
    memcpy(dest, src, numDwords*sizeof(DWORD));


    //LOG(L"dest now: (%02x,%02x,%08x)",
    //    dest->specialHair, dest->specialFace, dest->faceHairBits);
}

void fservAtSquadList(PLAYER_DETAILS* src, PLAYER_DETAILS* dest, 
                      DWORD numDwords)
{
    LOG(L"SQUAD src/dest: (%02x,%02x,%08x)/(%02x,%02x,%08x)", 
        src->specialHair, src->specialFace,
        src->faceHairBits, dest->specialHair,
        dest->specialFace, dest->faceHairBits);

    // execute game logic
    memcpy(dest, src, numDwords*sizeof(DWORD));
}

void fservAtSetDefault(DWORD srcAddr, DWORD destAddr, DWORD numDwords)
{
    PLAYER_DETAILS* src = (PLAYER_DETAILS*)(srcAddr + 0x2c);
    PLAYER_DETAILS* dest = (PLAYER_DETAILS*)(destAddr + 0x2c);

    LOG(L"SDEF: src/dest: (%02x,%02x,%08x)/(%02x,%02x,%08x)", 
        src->specialHair, src->specialFace,
        src->faceHairBits, dest->specialHair,
        dest->specialFace, dest->faceHairBits);

    // execute game logic
    memcpy(dest, src, numDwords*sizeof(DWORD));

    LOG(L"SDEF: dest now: (%02x,%02x,%08x)",
        dest->specialHair, dest->specialFace, dest->faceHairBits);
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
        push ebp
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
        movzx eax,byte ptr ds:[esi+8] // execute replaced code
        and al,0x7f                   // ...
        retn
    }
}

KEXPORT void fservAtFaceHair(DWORD dest, PLAYER_DETAILS* src)
{
    //static WORD lastSrcIndex = 0;
    WORD index = src->index;
    //if (index == 0) {
    //    index = lastSrcIndex;
    //} 
    //else {
    //    lastSrcIndex = index;
    //}

    WORD* pFace = (WORD*)(dest+0x0c);
    WORD* pHair = (WORD*)(dest+4);

    if (index == 0) {
        //LOG(L"(index=0) src->faceHairBits == %08x", src->faceHairBits);
        //LOG(L"(index=0) src->specialFace == %02x", src->specialFace);
        //LOG(L"(index=0) src->specialHair == %02x", src->specialHair);

        //if (src->faceHairBits == 0xc3920726) {
        //    __asm int 3;
        //}
    }

    if (index != 0) {
        // check corresponding slots
        WORD faceBin = FIRST_FACE_SLOT + index*2;
        WORD hairBin = faceBin + 1;

        if (IsSpecialFaceByte(src->specialFace)) {
            if (_fast_bin_table[faceBin - FIRST_FACE_SLOT]) {
                // face slot is assigned 
                *pFace = faceBin - FIRST_FACE_SLOT + FIRST_XFACE_ID;
                //LOG(L"index = %04x", index);
                //LOG(L"*pFace is now: %04x (bin=%d)", *pFace, faceBin);

                //DWORD id = faceBin - FIRST_FACE_SLOT + FIRST_XFACE_ID;
                //src->faceHairBits &= 0xc0007fff;
                //src->faceHairBits |= (id << 15) & 0x3fff8000;
            }
        }
        if (IsSpecialHairByte(src->specialHair)) {
            if (_fast_bin_table[hairBin - FIRST_FACE_SLOT]) {
                // hair slot is assigned
                *pHair = hairBin - FIRST_FACE_SLOT + FIRST_XHAIR_ID;
                //LOG(L"index = %04x (src=%08x, dest=%08x)", 
                //    index, (DWORD)src, (DWORD)dest);
                //LOG(L"*pHair is now: %04x (bin=%d)", *pHair, hairBin);

                //DWORD id = hairBin - FIRST_FACE_SLOT + FIRST_XHAIR_ID;
                //src->faceHairBits &= 0xffff8000;
                //src->faceHairBits |= id & 0x00007fff;

                //if (hairBin == 20021) {
                //    __asm int 3;
                //}
            }
        }
    }
}

/**
 */
bool BinExists(DWORD binId)
{
    // fast check first
    switch (_fast_bin_exists[binId - FIRST_FACE_SLOT]) {
        case CHECKED_OK:
            return true;
        case CHECKED_FAIL:
            return false;
        default:
            break;
    } 

    wstring* pws = _fast_bin_table[binId - FIRST_FACE_SLOT];
    if (!pws) {
        return false;
    }

    wbuffer_t wbuf(1024);
    _snwprintf(wbuf._buf, 1024, L"%sGDB\\faces\\%s", 
            getPesInfo()->gdbDir,
            pws->c_str());

    HANDLE handle = CreateFile(wbuf._buf,     // file to open
                       GENERIC_READ,          // open for reading
                       FILE_SHARE_READ,       // share for reading
                       NULL,                  // default security
                       OPEN_EXISTING,         // existing file only
                       FILE_ATTRIBUTE_NORMAL | CREATE_FLAGS, // normal file
                       NULL);                 // no attr. template

    if (handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
        _fast_bin_exists[binId - FIRST_FACE_SLOT] = CHECKED_OK;
        return true;
    }
    LOG(L"ERROR in faceserver map: FAILED to open file \"%s\".", 
        wbuf._buf);
    _fast_bin_exists[binId - FIRST_FACE_SLOT] = CHECKED_FAIL;
    return false;
}

/**
 */
bool FileExists(const wstring& filename)
{
    TRACE(L"FileExists:: %s", filename.c_str());
    HANDLE handle = CreateFile(filename.c_str(), // file to open
                       GENERIC_READ,          // open for reading
                       FILE_SHARE_READ,       // share for reading
                       NULL,                  // default security
                       OPEN_EXISTING,         // existing file only
                       FILE_ATTRIBUTE_NORMAL | CREATE_FLAGS, // normal file
                       NULL);                 // no attr. template

    if (handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(handle);
        return true;
    }
    return false;
}

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
 * read data callback
 */
void fservReadEditData(LPCVOID data, DWORD size)
{
    LOG(L"Setting face/hair settings");

    PLAYER_INFO* players = (PLAYER_INFO*)((BYTE*)data + 0x1a0);
    fservCopyPlayerData(players, 0, 3, true);
}

/**
 * write data callback
 */
void fservWriteEditData(LPCVOID data, DWORD size)
{
    LOG(L"Restoring face/hair settings");

    // restore face/hair settings
    PLAYER_INFO* players = (PLAYER_INFO*)((BYTE*)data + 0x1a0);
    /*
    hash_map<DWORD,player_facehair_t>::iterator it;
    for (it = _saved_facehair.begin(); it != _saved_facehair.end(); it++) {
        players[it->first].index = 0;
        players[it->first].specialFace &= ~SPECIAL_FACE;
        players[it->first].specialFace |= it->second.specialFace & SPECIAL_FACE;
        players[it->first].specialHair &= ~SPECIAL_HAIR;
        players[it->first].specialHair |= it->second.specialHair & SPECIAL_HAIR;
    }
    */
    hash_map<DWORD,BYTE>::iterator it;
    for (it = _saved_facebit.begin(); it != _saved_facebit.end(); it++) {
        players[it->first].index = 0;
        players[it->first].specialFace &= ~SPECIAL_FACE;
        players[it->first].specialFace |= it->second;
    }
    for (it = _saved_hairbit.begin(); it != _saved_hairbit.end(); it++) {
        players[it->first].index = 0;
        players[it->first].specialHair &= ~SPECIAL_HAIR;
        players[it->first].specialHair |= it->second;
    }
}

void fservReadReplayData(LPCVOID data, DWORD size)
{
    LOG(L"Reading replay data:");
    REPLAY_DATA* replay = (REPLAY_DATA*)data;
    if (memcmp(replay->ksSignature, REPLAY_SIG, 
                sizeof(replay->ksSignature))==0) {
        // check faceId/hairId for players
        // if we have the extended slots mapped, then ok
        // --> otherwise, clear the corresponding bits
        for (int i=0; i<22; i++) {
            // debug
            wchar_t* wname = Utf8::utf8ToUnicode(
                (BYTE*)(replay->payload.players[i].name));
            LOG(L"Player #%02d: %s", i, wname);
            Utf8::free(wname);
        }
    }
}

void fservWriteReplayData(LPCVOID data, DWORD size)
{
    LOG(L"Writing replay data:");
    REPLAY_DATA* replay = (REPLAY_DATA*)data;
    // set kitserver signature
    memcpy(replay->ksSignature, REPLAY_SIG, sizeof(replay->ksSignature));
}

KEXPORT DWORD fservGetFaceBin(DWORD faceId)
{
    if (faceId < FIRST_XFACE_ID) {
        //if (faceId == 0x724) {
        //    LOG(L"fservGetFaceBin: faceId=%d (hex:%04x) --> result=%d", 
        //        faceId, faceId, faceId);
        //}
        return faceId;
    }
    DWORD binId = faceId - FIRST_XFACE_ID + FIRST_FACE_SLOT;
    if (_fast_bin_table[binId - FIRST_FACE_SLOT]) {
        //LOG(L"fservGetFaceBin: faceId=%d (hex:%04x) --> result=%d", 
        //    faceId, faceId, binId);
        if (BinExists(binId)) {
            return binId;
        }
        return MANEKEN_ID;
    }
    //LOG(L"_fast_bin_table[%d - %d] is NULL!", binId, FIRST_FACE_SLOT);
    //LOG(L"faceId & 0x7ff = %d", (faceId & 0x7ff));
    return MANEKEN_ID; //faceId & 0x7ff;
}

KEXPORT DWORD fservGetHairBin(DWORD hairId)
{
    if (hairId < FIRST_XHAIR_ID) {
        //if (hairId == 0x726) {
        //    LOG(L"fservGetHairBin: hairId=%d (hex:%04x) --> result=%d", 
        //        hairId, hairId, hairId);
        //}
        return hairId;
    }
    DWORD binId = hairId - FIRST_XHAIR_ID + FIRST_FACE_SLOT;
    if (_fast_bin_table[binId - FIRST_FACE_SLOT]) {
        //LOG(L"fservGetHairBin: hairId=%d (hex:%04x) --> result=%d", 
        //    hairId, hairId, binId);
        if (BinExists(binId)) {
            return binId;
        }
        return MANEKEN_ID;
    }
    LOG(L"_fast_bin_table[%d - %d] is NULL!", binId, FIRST_FACE_SLOT);
    LOG(L"hairId & 0x7ff = %d", (hairId & 0x7ff));
    return MANEKEN_ID; //hairId & 0x7ff;
}

void fservReadBalData(LPCVOID data, DWORD size)
{
    BAL* bal = (BAL*)data;
    wchar_t* wideName = Utf8::utf8ToUnicode((BYTE*)bal->bal1.player.name);
    LOG(L"BAL player: id=%d, name={%s}, faceHairBits=%08x, "
        L"sHair=%02x, sFace=%02x",
            bal->bal1.player.id, wideName, 
            bal->bal2.playerDetails.faceHairBits,
            bal->bal2.playerDetails.specialHair, 
            bal->bal2.playerDetails.specialFace);
    Utf8::free(wideName);

    // Adjust face/hair bytes, if specified.
    hash_map<DWORD,WORD>::iterator it;
    it = _player_face_slot.find(bal->bal1.player.id);
    if (it != _player_face_slot.end())
    {
        WORD index = MAX_PLAYERS + (bal->bal1.player.id - FIRST_BAL_ID);
        LOG(L"BAL player %d has face slot=#%d (index=0x%x)",
                bal->bal1.player.id, it->second, index);

        bal->bal4.faceBit = bal->bal2.playerDetails.specialFace & SPECIAL_FACE;
        LOG(L"faceBit = %02x", bal->bal4.faceBit);

        bal->bal2.playerDetails.index = index;
        bal->bal2.playerDetails.specialFace |= SPECIAL_FACE;
        bal->bal3.player.index = index;
        bal->bal3.player.specialFace |= SPECIAL_FACE;
    }
    it = _player_hair_slot.find(bal->bal1.player.id);
    if (it != _player_hair_slot.end())
    {
        WORD index = MAX_PLAYERS + (bal->bal1.player.id - FIRST_BAL_ID);
        LOG(L"BAL player %d has hair slot=#%d (index=0x%x)",
                bal->bal1.player.id, it->second, index);

        bal->bal4.hairBit = bal->bal2.playerDetails.specialHair & SPECIAL_HAIR;
        LOG(L"hairBit = %02x", bal->bal4.hairBit);

        bal->bal2.playerDetails.index = index;
        bal->bal2.playerDetails.specialHair |= SPECIAL_HAIR;
        bal->bal3.player.index = index;
        bal->bal3.player.specialHair |= SPECIAL_HAIR;
    }
}

void fservWriteBalData(LPCVOID data, DWORD size)
{
    BAL* bal = (BAL*)data;
    wchar_t* wideName = Utf8::utf8ToUnicode((BYTE*)bal->bal1.player.name);
    LOG(L"BAL player: id=%d, name={%s}, faceHairBits=%08x, "
        L"sHair=%02x, sFace=%02x",
            bal->bal1.player.id, wideName, 
            bal->bal2.playerDetails.faceHairBits,
            bal->bal2.playerDetails.specialHair, 
            bal->bal2.playerDetails.specialFace);
    Utf8::free(wideName);

    // Restore face/hair bytes, if specified.
    hash_map<DWORD,WORD>::iterator it;
    it = _player_face_slot.find(bal->bal1.player.id);
    if (it != _player_face_slot.end())
    {
        WORD index = MAX_PLAYERS + (bal->bal1.player.id - FIRST_BAL_ID);
        LOG(L"BAL player %d has face slot=#%d (index=0x%x)",
                bal->bal1.player.id, it->second, index);
        bal->bal2.playerDetails.index = 0;
        bal->bal2.playerDetails.specialFace &= ~SPECIAL_FACE;
        bal->bal2.playerDetails.specialFace |= bal->bal4.faceBit;
        bal->bal3.player.index = 0;
        bal->bal3.player.specialFace &= ~SPECIAL_FACE;
        bal->bal3.player.specialFace |= bal->bal4.faceBit;
        LOG(L"specialFace now: %02x", bal->bal3.player.specialFace);
    }
    it = _player_hair_slot.find(bal->bal1.player.id);
    if (it != _player_hair_slot.end())
    {
        WORD index = MAX_PLAYERS + (bal->bal1.player.id - FIRST_BAL_ID);
        LOG(L"BAL player %d has hair slot=#%d (index=0x%x)",
                bal->bal1.player.id, it->second, index);
        bal->bal2.playerDetails.index = 0;
        bal->bal2.playerDetails.specialHair &= ~SPECIAL_HAIR;
        bal->bal2.playerDetails.specialHair |= bal->bal4.hairBit;
        bal->bal3.player.index = 0;
        bal->bal3.player.specialHair &= ~SPECIAL_HAIR;
        bal->bal3.player.specialHair |= bal->bal4.hairBit;
        LOG(L"specialHair now: %02x", bal->bal3.player.specialHair);
    }
}

