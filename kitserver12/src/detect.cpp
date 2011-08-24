// detect.cpp

#include "windows.h"
#include <stdio.h>
#include "detect.h"
#include "imageutil.h"
#include "shared.h"
#include "manage.h"

wchar_t* GAME[] = {
    L"PES2011 demo",
    L"PES2011",
    L"PES2011",
    L"PES2011 v1.01",
    L"PES2011 v1.01",
    L"PES2011 v1.02",
    L"PES2011 v1.03",
    L"PES2012 demo",
};

char* GAME_GUID[] = {
    "Pro Evolution Soccer 2011 DEMO",
    "Pro Evolution Soccer 2011",
    "Pro Evolution Soccer 2011",
    "Pro Evolution Soccer 2011",
    "Pro Evolution Soccer 2011",
    "1.02",
    "1.03",
    "Pro Evolution Soccer 2012 DEMO",
};

DWORD GAME_GUID_OFFSETS[] = { 
    0xfa2f68, 
    0xf23264, 
    0xf23a64, 
    0xf237bc,
    0xf22bbc,
    0x1027ae0,
    0x1027ae0,
    0x10d0140,
};

DWORD GAME_GUID_MEMORY[] = { 
    0x13a4968, 
    0x1323a64, 
    0x1323a64, 
    0x13239bc,
    0x13239bc,
    0x1427ae0,
    0x1427ae0,
    0x14d1140,
};

BYTE BASE_GAME[] = {
    gvPES2011demo,
    gvPES2011,
    gvPES2011,
    gvPES2011v101,
    gvPES2011v101,
    gvPES2011v102,
    gvPES2011v103,
    gvPES2012demo,
};

bool ISGAME[] = {
    true,   // PES2011 demo
    true,   // PES2011
    true,   // PES2011 nodvd
    true,   // PES2011 v1.01 
    true,   // PES2011 v1.01 nodvd
    true,   // PES2011 v1.02
    true,   // PES2011 v1.03
    true,   // PES2012 demo
};

// Returns the game real version id
int GetRealGameVersion(void)
{
    for (int i=0; i<sizeof(GAME_GUID)/sizeof(char*); i++)
    {
        if (!IsBadReadPtr((char*)GAME_GUID_MEMORY[i], 
                    strlen(GAME_GUID[i]))) 
        {
            char* guid = (char*)(GAME_GUID_MEMORY[i]);
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
