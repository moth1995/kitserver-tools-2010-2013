// ADDRESSES for kserv.cpp
BYTE allowedGames[] = {
    //gvPES2010v13,
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
    /*
    // PES2010 v1.3
    {
        0xf417b5, 0xf41745, 
        0xef1a2b, 0xef28ab, 0x10a8a41,
        0xf05dee, 0x12838d8,
        0xfdf79e, 0xb82aaf,
    },
    */
    // PES2011 demo
    NOCODEADDR,
    // PES2011 
    NOCODEADDR,
    // PES2011 v1.01
    NOCODEADDR,
    // PES2011 v1.02
    NOCODEADDR,
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
DWORD dataArray[][DATALEN] = {
    /*
    // PES2010 v1.3
    {
        0x1942b48, 0x1942b50,
        ???, 0x1c64c8,
        0x1644004,
    },
    */
    // PES2011 demo
    NODATAADDR,
    // PES2011 
    NODATAADDR,
    // PES2011 v1.01
    NODATAADDR,
    // PES2011 v1.02
    NODATAADDR,
    // PES2011 v1.03
    {
        0, 0x1940b5c,
        0x3ac8c8, 0x249918,
        0x15a2c50,
    },
};

DWORD code[CODELEN];
DWORD data[DATALEN];
