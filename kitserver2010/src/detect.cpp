// detect.cpp

#include "windows.h"
#include <stdio.h>
#include "detect.h"
#include "imageutil.h"
#include "shared.h"
#include "manage.h"

wchar_t* GAME[] = {
    L"PES2008 PC DEMO",
    L"[Settings] PES2008 PC DEMO",
    L"PES2008 PC",
    L"[Settings] PES2008 PC",
    L"PES2008 PC FLT-NODVD",
    L"PES2008 PC 1.10",
    L"PES2008 PC 1.10 NODVD",
    L"PES2008 PC 1.20",
    L"PES2008 PC 1.20 ViTALiTY NODVD",
    L"PES2009 PC Demo",
    L"PES2009 PC",
    L"PES2009 PC 1.10",
    L"PES2009 PC 1.20",
    L"[Settings] PES2009 PC DEMO",
    L"[Settings] PES2009 PC",
    L"PES2009 PC 1.30",
    L"PES2009 PC 1.40",
    L"PES2010 PC Demo",
    L"PES2010 PC",
    L"PES2010 PC 1.1",
    L"[Settings] PES2010 PC DEMO",
    L"[Settings] PES2010 PC",
    L"PES2010 PC 1.2",
    L"PES2010 PC 1.3",
};

char* GAME_GUID[] = {
    "Pro Evolution Soccer 2008 DEMO",
    "Pro Evolution Soccer 2008 DEMO",
    "Pro Evolution Soccer 2008",
    "Pro Evolution Soccer 2008",
    "rr0\"\x0d\x09\x08",
    "PC  1.10",
    "PC  1.10",
    "PC  1.20",
    "+V!TAL!TY+",
    "Pro Evolution Soccer 2009 DEMO",
    "Pro Evolution Soccer 2009",
    "Pro Evolution Soccer 2009",
    "Pro Evolution Soccer 2009",
    "Pro Evolution Soccer 2009 DEMO",
    "Pro Evolution Soccer 2009",
    "Pro Evolution Soccer 2009",
    "Pro Evolution Soccer 2009",
    "Pro Evolution Soccer 2010 DEMO",
    "Pro Evolution Soccer 2010",
    "Pro Evolution Soccer 2010",
    "Pro Evolution Soccer 2010 DEMO",
    "Pro Evolution Soccer 2010",
    "Pro Evolution Soccer 2010",
    "Pro Evolution Soccer 2010",
};

DWORD GAME_GUID_OFFSETS[] = { 
    0x67aca8, 0x5b5c4, 0x994e74, 0x5ec34, 
    0x3e0, 0x977c50, 0x977c50, 0x978be8, 0x16e, 0xa5db00, 
    0xbd0b80, 0xbd0b70, 0xb80874,  0xa74a3, 0xaa757, 
    0xb84c6c, 0xb84c6c, 
    0x9d717c, 0xd865ec, 0xdf0224, 0xa84d3, 0xab787, 
    0xdf23b8, 0xdf63a4,
};

/**
Version offsets (UNICODE):
PES 2010:      0x1264e44
PES 2010 v1.1: 0x12d3e44
PES 2010 v1.2: 0x12d5e44
PES 2010 v1.3: 0x12d9e44
**/

BYTE BASE_GAME[] = {
    gvPES2008demo,
    gvPES2008demoSet,
    gvPES2008,
    gvPES2008Set,
    gvPES2008,
    gvPES2008v110,
    gvPES2008v110,
    gvPES2008v120,
    gvPES2008v120,
    gvPES2009demo,
    gvPES2009,
    gvPES2009v110,
    gvPES2009v120,
    gvPES2009demoSet,
    gvPES2009Set,
    gvPES2009v130,
    gvPES2009v140,
    gvPES2010demo,
    gvPES2010,
    gvPES2010v11,
    gvPES2010demoSet,
    gvPES2010Set,
    gvPES2010v12,
    gvPES2010v13,
};

bool ISGAME[] = {
    true,   // PES2008 PC DEMO
    false,  // PES2008 PC DEMO (Settings)
    true,   // PES2008 PC
    false,  // PES2008 PC (Settings)
    true,   // PES2008 PC FLT-NODVD
    true,   // PES2008 PC 1.10
    true,   // PES2008 PC 1.10 NODVD
    true,   // PES2008 PC 1.20
    true,   // PES2009 PC DEMO
    true,   // PES2009 PC
    true,   // PES2009 PC 1.10
    true,   // PES2009 PC 1.20
    false,  // PES2009 PC DEMO (Settings)
    false,  // PES2009 PC (Settings)
    true,   // PES2009 PC 1.30
    true,   // PES2009 PC 1.40
    true,   // PES2010 PC DEMO
    true,   // PES2010 PC
    true,   // PES2010 PC 1.1
    false,  // PES2010 PC DEMO (Settings)
    false,  // PES2010 PC (Settings)
    true,   // PES2010 PC 1.2
    true,   // PES2010 PC 1.3
};

// Returns the game real version id
int GetRealGameVersion(void)
{
    HMODULE hMod = GetModuleHandle(NULL);
    for (int i=0; i<sizeof(GAME_GUID)/sizeof(char*); i++)
    {
        if (!IsBadReadPtr((BYTE*)hMod + GAME_GUID_OFFSETS[i], 
                    strlen(GAME_GUID[i]))) 
        {
            char* guid = (char*)((DWORD)hMod + GAME_GUID_OFFSETS[i]);
            if (memcmp(guid, GAME_GUID[i], strlen(GAME_GUID[i]))==0)
                    return i;
        }
    }
    return -1;
}

// Returns the game version id
int GetRealGameVersion(wchar_t* filename)
{
    char guid[512];
    memset(guid,0,sizeof(guid));

    FILE* f = _wfopen(filename, L"rb");
    if (f == NULL)
        return -1;

    // check for regular exes
    for (int i=0; i<sizeof(GAME_GUID)/sizeof(char*); i++)
    {
        fseek(f, GAME_GUID_OFFSETS[i], SEEK_SET);
        if (fread(guid, strlen(GAME_GUID[i]), 1, f)==1)
        {
            if (memcmp(guid, GAME_GUID[i], strlen(GAME_GUID[i]))==0)
            {
                fclose(f);
                return i;
            }
        }
    }

    // unrecognized.
    return -1;
}

bool isGame(int gameVersion)
{
    if (gameVersion == -1) return false;
    return ISGAME[gameVersion];
}

bool isRealGame(int realGameVersion)
{
    if (realGameVersion == -1) return false;
    return ISGAME[GetGameVersion(realGameVersion)];
}

int GetGameVersion()
{
    int v = GetRealGameVersion();
    if (v == -1) return -1;
    return BASE_GAME[v];
}

int GetGameVersion(int realGameVersion)
{
    if (realGameVersion == -1) return -1;
    return BASE_GAME[realGameVersion];
}
