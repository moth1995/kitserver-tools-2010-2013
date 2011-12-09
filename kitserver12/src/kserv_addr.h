// ADDRESSES for kserv.cpp
BYTE allowedGames[] = {
    //gvPES2011,
    //gvPES2011v101,
    //gvPES2011v102,
    //gvPES2011v103,
    gvPES2012,
    gvPES2012v101,
    gvPES2012v102,
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
    // PES2012 demo
    {
        0, 0,
        0, 0, 0,
        0, 0,//**
        0, 0,//**
    },
    // PES2012 demo
    {
        0, 0,
        0, 0, 0,
        0, 0,//**
        0, 0,//**
    },
    // PES2012
    {
        0x119cf2c, 0x12c3d7c,
        0x11e7e4c, 0x11e88fc, 0x12ce53a,
        0, 0,//**
        0, 0,//**
    },
    // PES2012 v1.01
    {
        0x11a036c, 0x12c721c,
        0x11eb28c, 0x11ebd3c, 0x12d1a2a,
        0, 0,//**
        0, 0,//**
    },
    // PES2012 v1.02
    {
        0x11a5b7c, 0x12c8d9c,
        0x11f0a9c, 0x11f154c, 0x12d35aa,
        0, 0,//**
        0, 0xd9b76c,//**
    },
};

#define DATALEN 6 
enum {
	NEXT_MATCH_DATA_PTR, PLAYERS_DATA,
    TEAM_NAMES_OFFSET, TEAM_KIT_INFO_OFFSET,
    NUM_SLOTS_PTR, EURO_TEAM_KIT_INFO_PTR,
};

#define NODATAADDR {0,0,0,0,0,0}
DWORD dataArray[][DATALEN] = {
    // PES2011 demo
    NODATAADDR,
    // PES2011 
    {
        0, 0x1940b08,
        0x3ac8c8, 0x249918,
        0x15a2c50, 0,
    },
    // PES2011 v1.01
    {
        0, 0x1940b60,
        0x3ac8c8, 0x249918,
        0x15a2c50, 0,
    },
    // PES2011 v1.02
    {
        0, 0x1940b58,
        0x3ac8c8, 0x249918,
        0x15a2c50, 0,
    },
    // PES2011 v1.03
    {
        0, 0x1940b5c,
        0x3ac8c8, 0x249918,
        0x15a2c50, 0,
    },
    // PES2012 demo
    {
        0, 0,
        0, 0,
        0, 0,
    },
    // PES2012 demo2
    {
        0, 0,
        0, 0,
        0, 0,
    },
    // PES2012
    {
        0, 0x1a15874,
        0x42f2a0, 0x2b8e58,
        0x16d4d94, 0x1a1588c,
    },
    // PES2012 v1.01
    {
        0, 0x1a26a3c,
        0x42f2a0, 0x2b8e58,
        0x16e5eac, 0x1a26a54,
    },
    // PES2012 v1.02
    {
        0, 0x1a28a4c,
        0x42f2a0, 0x2b8e58,
        0x16e7eac, 0x1a28a64,
    },
};

DWORD code[CODELEN];
DWORD data[DATALEN];
