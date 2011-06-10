// ADDRESSES for lodmixer.cpp
BYTE allowedGames[] = {
    gvPES2011,
    gvPES2011v101,
    gvPES2011v102,
    gvPES2011v103,
};

#define CODELEN 3
enum {
    C_SETTINGS_READ, C_QUALITY_CHECK, C_QUALITY_CHECK_2,
};

#define NOCODEADDR {0,0,0}
DWORD codeArray[][CODELEN] = { 
    // PES2011 demo
    NOCODEADDR,
    // PES2011 
    {
        0x1122e73, 0x112603a, 0x1125cf4,
    },
    // PES2011 v1.01 
    {
        0x11233d3, 0x112659a, 0x1126254,
    },
    // PES2011 v1.02 
    {
        0x1123533, 0x11266fa, 0x11263b4,
    },
    // PES2011 v1.03 
    {
        0x1123573, 0x112673a, 0x11263f4,
    },
};

#define DATALEN 11
enum {
    SCREEN_WIDTH, SCREEN_HEIGHT, WIDESCREEN_FLAG,
    RATIO_4on3, RATIO_16on9,
    LOD_PLAYERS_TABLE1, LOD_PLAYERS_TABLE2, 
    LOD_REF_TABLE1, LOD_REF_TABLE2,
};

#define NODATAADDR {0,0,0,0,0,0,0,0,0,0,0}
DWORD dataArray[][DATALEN] = {
    // PES2011 demo
    NODATAADDR,
    // PES2011 
    {
        0x1940370, 0x1940374, 0x1940378,
        0x143df84, 0x1440da0,
        0x15a2dd0, 0x15a2e18,
        0x1535760, 0x1535788,
    },
    // PES2011 v1.01
    {
        0x19403c8, 0x19403cc, 0x19403d0,
        0x143e304, 0x1441120,
        0x15a2dd0, 0x15a2e18,
        0x1535760, 0x1535788, 
    },
    // PES2011 v1.02
    {
        0x19403c0, 0x19403c4, 0x19403c8,
        0x143e30c, 0x1441128,
        0x15a2dd0, 0x15a2e18,
        0x1535760, 0x1535788, 
    },
    // PES2011 v1.03
    {
        0x19403c0, 0x19403c4, 0x19403c8,
        0x143e30c, 0x1441128,
        0x15a2dd0, 0x15a2e18,
        0x1535760, 0x1535788, 
    },
};

DWORD code[CODELEN];
DWORD data[DATALEN];
