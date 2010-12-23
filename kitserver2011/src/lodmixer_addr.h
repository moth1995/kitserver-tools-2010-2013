// ADDRESSES for lodmixer.cpp
BYTE allowedGames[] = {
    gvPES2011v102,
    gvPES2011v103,
};

#define CODELEN 7
enum {
	C_SETTINGS_CHECK, C_MODE_CHECK,
    C_SETTINGS_READ, C_SETTINGS_RESET, C_VIDEO_CHECK1, C_VIDEO_CHECK2, 
    C_LODCHECK_1,
};

#define NOCODEADDR {0,0,0,0,0,0,0},
DWORD codeArray[][CODELEN] = { 
    // PES2011 demo
    NOCODEADDR
    // PES2011 
    NOCODEADDR
    // PES2011 v1.01 
    NOCODEADDR
    // PES2011 v1.02 
    NOCODEADDR
    // PES2011 v1.03 
    NOCODEADDR
};

#define DATALEN 11
enum {
    SCREEN_WIDTH, SCREEN_HEIGHT, WIDESCREEN_FLAG,
    RATIO_4on3, RATIO_16on9,
    LOD_PLAYERS_TABLE1, LOD_PLAYERS_TABLE2, 
    LOD_REF_TABLE1, LOD_REF_TABLE2,
};

#define NODATAADDR {0,0,0,0,0,0,0,0,0,0,0},
DWORD dataArray[][DATALEN] = {
    // PES2011 demo
    NODATAADDR
    // PES2011 
    NODATAADDR
    // PES2011 v1.01
    NODATAADDR
    /*// PES2010 1.3
    {
        0x19423e0, 0x19423e4, 0x19423e8,
        0x129adcc, 0x129dd84,
        0x1643f90, 0x1643fd8, 
        0x15e1360, 0x15e1388,
    },*/
    // PES2011 v1.02
    {
        0, 0, 0,
        0, 0,
        0x15a2dd0, 0x15a2e18,
        0x1535760, 0x1535788, 
    },
    // PES2011 v1.03
    {
        0, 0, 0,
        0, 0,
        0x15a2dd0, 0x15a2e18,
        0x1535760, 0x1535788, 
    },
};

DWORD code[CODELEN];
DWORD data[DATALEN];
