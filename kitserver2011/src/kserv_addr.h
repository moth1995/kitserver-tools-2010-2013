// ADDRESSES for kserv.cpp
BYTE allowedGames[] = {
    gvPES2011,
    gvPES2011v101,
    gvPES2011v102,
    gvPES2011v103,
};

#define CODELEN 9
enum {
	C_AFTER_READ_NAMES, C_AFTER_APPLY_CONTENT,
    C_READ_NUM_SLOTS1, C_READ_NUM_SLOTS2, C_READ_NUM_SLOTS3,
    C_AFTER_CREATE_TEXTURE, C_KIT_VTABLE, //nice-to-have
    C_READ_UNIFORM_PICK, C_CHECK_KIT_RELOAD_FLAG, //nice-to-have
};

#define NOCODEADDR {0,0,0,0,0,0,0,0,0}
DWORD codeArray[][CODELEN] = { 
    // PES2011 demo
    NOCODEADDR,
    // PES2011 
    {
        0x787b5c, 0x109dc74,
        0x10cf60c, 0x10d00bc, 0x119d7ea,
        0, 0,
        0, 0,
    },
    // PES2011 v1.01
    {
        0x78816c, 0x109e1d4,
        0x10cfb6c, 0x10d061c, 0x119dd3a,
        0, 0,
        0, 0,
    },
    // PES2011 v1.02
    {
        0x78805c, 0x109e334,
        0x10cfccc, 0x10d077c, 0x119de9a,
        0, 0,
        0, 0,
    },
    // PES2011 v1.03
    {
        0x78806c, 0x109e344,
        0x10cfcdc, 0x10d078c, 0x119deda,
        0, 0,//**
        0, 0,//**
    },
};

#define DATALEN 5 
enum {
	NEXT_MATCH_DATA_PTR, PLAYERS_DATA,
    TEAM_NAMES_OFFSET, TEAM_KIT_INFO_OFFSET,
    NUM_SLOTS_PTR,
};

#define NODATAADDR {0,0,0,0,0}
DWORD dtaArray[][DATALEN] = {
    // PES2011 demo
    NODATAADDR,
    // PES2011 
    {
        0, 0x1940b08,
        0x3ac8c8, 0x249918,
        0x15a2c50,
    },
    // PES2011 v1.01
    {
        0, 0x1940b60,
        0x3ac8c8, 0x249918,
        0x15a2c50,
    },
    // PES2011 v1.02
    {
        0, 0x1940b58,
        0x3ac8c8, 0x249918,
        0x15a2c50,
    },
    // PES2011 v1.03
    {
        0, 0x1940b5c,
        0x3ac8c8, 0x249918,
        0x15a2c50,
    },
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
