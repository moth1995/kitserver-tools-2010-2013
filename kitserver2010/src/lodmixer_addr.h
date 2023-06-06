// ADDRESSES for lodmixer.cpp
BYTE allowedGames[] = {
    gvPES2010demo,
    gvPES2010,
    gvPES2010v11,
    gvPES2010v12,
    gvPES2010v13,
};

#define CODELEN 7
enum {
	C_SETTINGS_CHECK, C_MODE_CHECK,
    C_SETTINGS_READ, C_SETTINGS_RESET, C_VIDEO_CHECK1, C_VIDEO_CHECK2, 
    C_LODCHECK_1,
};

#define NOCODEADDR {0,0,0,0,0,0,0},
DWORD codeArray[][CODELEN] = { 
    // PES2008 DEMO
    NOCODEADDR
	// [Settings] PES2008 PC DEMO
    NOCODEADDR
    // PES2008
    NOCODEADDR
	// [Settings] PES2008 PC
    NOCODEADDR
    NOCODEADDR
    // PES2008 1.10
    NOCODEADDR
    NOCODEADDR
    // PES2008 1.20
    NOCODEADDR
    // PES2009 Demo
    NOCODEADDR
    // PES2009 
    NOCODEADDR
    // PES2009 1.10
    NOCODEADDR
    // PES2009 1.20
    NOCODEADDR
	// [Settings] PES2009 PC DEMO
	NOCODEADDR
	// [Settings] PES2009 PC
	NOCODEADDR
	// PES2009 PC 1.30
	NOCODEADDR
	// PES2009 PC 1.40
	NOCODEADDR
    // PES2010 Demo
    {
        0, 0,
        0xbfd260, 0xbfccf4, 0xbfff90, 0xc002b0,
        0,
    },
    // PES2010 
    {
        0, 0xf7313e,
        0xfb5570, 0xfb5004, 0xfb82c0, 0xfb85e0,
        0,
    },
    // PES2010 1.1 
    {
        0, 0xfcc79e,
        0x10156a0, 0x1015134, 0x10183f0, 0x1018710,
        0,
    },
	// [Settings] PES2010 PC DEMO
	NOCODEADDR
	// [Settings] PES2010 PC
	NOCODEADDR
    // PES2010 1.2 
    {
        0, 0xfce1ee,
        0x10170d0, 0x1016b64, 0x1019ec0, 0x101a1e0,
        0,
    },
    // PES2010 1.3 
    {
        0, 0xfd5cce,
        0x1020000, 0x101fa94, 0x1020ee0, 0x1021200,
        0,
    },
};

#define DATALEN 11
enum {
    SCREEN_WIDTH, SCREEN_HEIGHT, WIDESCREEN_FLAG,
    RATIO_4on3, RATIO_16on9,
    LOD_SWITCH1, LOD_SWITCH2,
    LOD_PLAYERS_TABLE1, LOD_PLAYERS_TABLE2, 
    LOD_REF_TABLE1, LOD_REF_TABLE2,
};

#define NODATAADDR {0,0,0,0,0,0,0,0,0,0,0},
DWORD dtaArray[][DATALEN] = {
    // PES2008 DEMO
	NODATAADDR
	// [Settings] PES2008 PC DEMO
	NODATAADDR
    // PES2008
	NODATAADDR
	// [Settings] PES2008 PC
	NODATAADDR
	NODATAADDR
    // PES2008 1.10
	NODATAADDR
    NODATAADDR
    // PES2008 1.20
	NODATAADDR
    // PES2009 DEMO
	NODATAADDR
    // PES2009 
	NODATAADDR
    // PES2009 1.10
	NODATAADDR
    // PES2009 1.20
	NODATAADDR
	// [Settings] PES2009 PC DEMO
	NODATAADDR
	// [Settings] PES2009 PC
	NODATAADDR
	// PES2009 PC 1.30
	NODATAADDR
	// PES2009 PC 1.40
	NODATAADDR
    // PES2010 DEMO
    {
        0x1137558, 0x113755c, 0x1137560,
        0xdf232c, 0xdf5078,
        0, 0,
        0xeb3340, 0xeb3388, 0xe52d3c, 0xe52d64,
    },
    // PES2010 
    {
        0x18bcc38, 0x18bcc3c, 0x18bcc40,
        0x122861c, 0x122b5e8,
        0, 0,
        0x15ce6b8, 0x15ce700, 0x156bc04, 0x156bc2c,
    },
    // PES2010 1.1
    {
        0x193c398, 0x193c39c, 0x193c3a0,
        0x129426c, 0x1297248,
        0, 0,
        0x163df48, 0x163df90, 0x15db32c, 0x15db354,
    },
	// [Settings] PES2010 PC DEMO
	NODATAADDR
	// [Settings] PES2010 PC
	NODATAADDR
    // PES2010 1.2
    {
        0x193e3b8, 0x193e3bc, 0x193e3c0,
        0x1296cd4, 0x1299c94,
        0, 0,
        0x163ff68, 0x163ffb0, 0x15dd34c, 0x15dd374,
    },
    // PES2010 1.3
    {
        0x19423e0, 0x19423e4, 0x19423e8,
        0x129adcc, 0x129dd84,
        0, 0,
        0x1643f90, 0x1643fd8, 0x15e1360, 0x15e1388,
    },
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
