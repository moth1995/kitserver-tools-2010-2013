/* <camera>
 *
 */
#define UNICODE
#define THISMOD &k_leagues

#include <windows.h>
#include <stdio.h>
#include <sys/stat.h>
#include "kload_exp.h"
#include "leagues.h"
#include "leagues_addr.h"
#include "dllinit.h"
#include "configs.h"
#include "configs.hpp"
#include "utf8.h"

#define lang(s) getTransl("leagues",s)

#include <map>
#include <list>
#include <unordered_map>
#include <string>

#define SWAPBYTES(dw) \
    ((dw<<24 & 0xff000000) | (dw<<8  & 0x00ff0000) | \
    (dw>>8  & 0x0000ff00) | (dw>>24 & 0x000000ff))


// VARIABLES
HINSTANCE hInst = NULL;
KMOD k_leagues = {MODID, NAMELONG, NAMESHORT, DEFAULT_DEBUG};

// GLOBALS
DWORD *Relinks;
DWORD _codeMakeLeague = 0;
DWORD EsiTmp;
DWORD _offset = 0;
DWORD _offsetn = 0;
DWORD _offsetl = 0;
DWORD _relink = 0;
unordered_map<WORD,wstring> mapFile;
char* _thistex;
_league *nationals, *clubs;
DWORD customleague;

// FUNCTIONS
HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface);

KEXPORT DWORD leaguesRead(_league* theleagues);
void nationalsReadCallPoint();
void clubsReadCallPoint();
KEXPORT DWORD nationsRead();
KEXPORT DWORD clubsRead();
void changerelink();
void changenames();
bool readfile(const wchar_t* cfgFile, _league* &theleagues);

static void string_strip(wstring& s)
{
    static const wchar_t* empties = L" \t\n\r";
    int e = s.find_last_not_of(empties);
    s.erase(e + 1);
    int b = s.find_first_not_of(empties);
    s.erase(0,b);
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

		copyAdresses();
		hookFunction(hk_D3D_CreateDevice, initModule);
	}
	
	else if (dwReason == DLL_PROCESS_DETACH)
	{
	}
	
	return true;
}

HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface) {

	unhookFunction(hk_D3D_CreateDevice, initModule);
	
	bool found = false;

    LOG(L"Initializing Leagues Module");

	customleague=0xff;

    //Load leagues' list
	wstring filename(getPesInfo()->myDir);
	filename += L"national.txt";
	if(readfile(filename.c_str(), nationals))
	{
		found = true;

		HookCallPoint(code[C_READ_NATIONALS],
                nationalsReadCallPoint, 6, 2);

		if(nationals != NULL)
			LOG1N(L"Found %d national leagues", _msize(nationals)/sizeof(nationals[0]));
		else
			LOG(L"No national leagues");
	}
	else
	{
		LOG(L"No national.txt file found.");
	}

	filename = getPesInfo()->myDir;
	filename += L"club.txt";
	if(readfile(filename.c_str(), clubs))
	{
		found = true;

		HookCallPoint(code[C_READ_CLUBS],
				clubsReadCallPoint, 6, 2);
		if(clubs != NULL)
			LOG1N(L"Found %d club leagues", _msize(clubs)/sizeof(clubs[0]));
		else
			LOG(L"No club leagues");
	}
	else
	{
		LOG(L"No club.txt file found.");
	}

    if (found)
    {
        _codeMakeLeague = code[C_MAKE_LEAGUE];
		_offset = dta[D_RELINKS_OFFSET];
		_offsetl = dta[D_LOGOS_OFFSET];
		_offsetn = dta[D_NAMES_OFFSET];
		HookCallPoint(code[C_READ_RELINK],
			changerelink, 3, 2);
		HookCallPoint(code[C_READ_NAMES],
			changenames, 3, 9);
        LOG(L"Function hooked");
    }
    else
        LOG(L"Using game-set leagues");

	TRACE(L"Initialization complete.");
    return D3D_OK;
}

void nationalsReadCallPoint()
{
    __asm {
		mov edi, ecx
        pushfd
        push eax
        push ebx
        push ecx
		push edx
        push edi
        push esi
        push ebp
        call nationsRead
        pop ebp
        pop esi
        pop edi
        pop edx
        pop ecx
        pop ebx
        pop eax
        popfd
        add esp,4 // pop our return address - we won't need it
        pop edi   // execute replaced code
        pop esi   // ...
        pop ecx   // ...
        retn      // ... */
    }
}

void clubsReadCallPoint()
{
    __asm {
		mov edi, ecx
        pushfd
        push eax
        push ebx
        push ecx
		push edx
        push edi
        push esi
        push ebp
        call clubsRead
        pop ebp
        pop esi
        pop edi
        pop edx
        pop ecx
        pop ebx
        pop eax
        popfd
        add esp,4 // pop our return address - we won't need it
        pop edi   // execute replaced code
        pop esi   // ...
		pop ebx   // ...
        pop ecx   // ...
        retn 4    // ... */
    }
}

void changerelink()
{
	if(customleague!=0xff)
	{
		__asm{
			mov ecx, _relink
			retn
		}
	}
	else
	{
		__asm{
			push eax
			mov eax, _offset
			MOV ECX,DWORD PTR DS:[ESI*4+eax]
			pop eax
			retn
		}
	}
}

void changenames()
{
	if(customleague!=0xff)
	{
		__asm{
			mov ecx, _thistex
			mov edx, 00
			retn
		}
	}
	else
	{
		__asm{
			push eax
			mov eax, _offsetl
			MOV ECX,DWORD PTR DS:[ESI*4+eax]
			mov eax, _offsetn
			MOV EDX,DWORD PTR DS:[ESI*4+eax]
			pop eax
			retn
		}
	}
}

KEXPORT DWORD nationsRead()
{
	LOG(L"Loading national teams");

	EsiTmp = 0;

	customleague = 0;

	leaguesRead(nationals);

	customleague = 0xff;

	return 0;
}

KEXPORT DWORD clubsRead()
{
	customleague = 0;

	leaguesRead(clubs);

	customleague = 0xff;

	return 0;
}


KEXPORT DWORD leaguesRead(_league* theleagues)
{
	LOG(L"Initializing leagues function...");

	char* _leaguename;
	wstring tempname;

	_leaguename = new char[255];

    // read our additional leagues
	if(theleagues == NULL)
		return 0;

    for (int i=0; i<_msize(theleagues)/sizeof(theleagues[0]); i++)
	{
        string_strip(tempname);
		_relink = theleagues[i].relink;
		_leaguename = theleagues[i].leaguename;
		_thistex = theleagues[i].leaguelogo;

        // we need to set esi, so best to use
        // assembly fragment to call the league reading function
    	__asm {
            mov esi, EsiTmp
            push _leaguename
            push 0
            push 1
            call _codeMakeLeague
        }

		EsiTmp++;
	}

    return 0;
}

bool readfile(const wchar_t* cfgFile, _league* &theleagues)
{
	int count=0;

	FILE* f = _wfopen(cfgFile, L"rb");
	if (!f) return false;

	DWORD firstDWORD;
	bool unicodeFile = false;
	fgets((char*)&firstDWORD, 4, f);
	if ((firstDWORD & 0xffffff) == 0xbfbbef) {
		// UTF8
		fseek(f, 3, SEEK_SET);
	} else if ((firstDWORD & 0xffff) == 0xfeff) {
		// Unicode Little Endian
		unicodeFile = true;
		fseek(f, 2, SEEK_SET);
	} else {
		// no supported BOM detected, asume UTF8
		fseek(f, 0, SEEK_SET);
	}
	
	char currSection[64] = {'\0'};
	
	char a_str[BUFLEN];
	wchar_t str[WBUFLEN];

	wchar_t *pComment = NULL, *pName = NULL, *pValue = NULL, *pValue1 = NULL, *pSpace = NULL, *pEq = NULL, *pTemp = NULL;

	while (!feof(f))
	{
		if (!unicodeFile) {
			ZeroMemory(str, WBUFLEN);
			ZeroMemory(a_str, BUFLEN);
			fgets(a_str, BUFLEN-1, f);
			Utf8::fUtf8ToUnicode(str, a_str);
		} else {
			ZeroMemory(str, WBUFLEN);
			fgetws(str, BUFLEN-1, f);
		}

        if (wcslen(str)==0)
            continue;
		
		if (str[0] == '[') {
			wchar_t* pFirst = str + 1;
			wchar_t* pSecond = wcsstr(pFirst, L"]");
			if (!pSecond) continue;
			*pSecond = '\0';
			
			Utf8::fUnicodeToAnsi(currSection, pFirst);
			continue;
		}
		
		// skip comments
		pComment = wcsstr(str, L"#");
		if (pComment != NULL) pComment[0] = '\0';
			
		// parse the line
		pName = pValue = NULL;
		pEq = wcsstr(str, L",");
		if (pEq == NULL || pEq[1] == '\0') continue;

		pEq[0] = '\0';
		pName = str;
		pValue = pEq + 1;
		
        WORD numKey;
		count++;
		if (swscanf(pName, L"%d", &numKey)==1)
        {
            while (*pValue == ' ')
                pValue++;

			pEq = wcsstr(pValue, L",");
			if (pEq == NULL || pEq[1] == '\0') continue;

			pEq[0] = '\0';
			pValue1 = pEq + 1;

			while (*pValue1 == ' ')
                pValue1++;

            pTemp = pValue1 + wcslen(pValue1) - 1;
            while (*pTemp == (wchar_t)13 || *pTemp == (wchar_t)10) {
                *pTemp = 0;
                pTemp--;
            }

			theleagues = (_league*)realloc(theleagues, count*sizeof(_league));

			theleagues[count-1].relink = (DWORD)numKey;

            wstring sVal = pValue;
			theleagues[count-1].leaguename = new char[255];
			sprintf(theleagues[count-1].leaguename,"%ls",sVal.c_str());

			sVal = pValue1;
			theleagues[count-1].leaguelogo = new char[255];
			sprintf(theleagues[count-1].leaguelogo,"%ls",sVal.c_str());
        }
	}
	
    fclose(f);
	if(count==0)
		theleagues = NULL;
	return true;
}
