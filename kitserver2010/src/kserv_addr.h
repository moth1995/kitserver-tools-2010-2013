// ADDRESSES for kserv.cpp
BYTE allowedGames[] = {
    gvPES2010,
    gvPES2010v11,
    gvPES2010v12,
    gvPES2010v13,
};

#define CODELEN 9
enum {
	C_AFTER_READ_NAMES,
    C_READ_NUM_SLOTS1, C_READ_NUM_SLOTS2, C_READ_NUM_SLOTS3,
    C_AFTER_CREATE_TEXTURE, C_KIT_VTABLE,
    C_AFTER_APPLY_CONTENT, //C_UNIFORM_SELECT, C_TEAMS_RESET,

    C_READ_UNIFORM_PICK, C_CHECK_KIT_RELOAD_FLAG,
};

#define NOCODEADDR {0,0,0,0,0,0,0,0,0},
DWORD codeArray[][CODELEN] = { 
	// PES2008
    NOCODEADDR
    NOCODEADDR
    NOCODEADDR
    NOCODEADDR
    NOCODEADDR
    NOCODEADDR
    NOCODEADDR
	NOCODEADDR
	// PES2009 demo
	NOCODEADDR
    // PES2009
	NOCODEADDR
    // PES2009 v1.10
	NOCODEADDR
    // PES2009 v1.20
	NOCODEADDR
    // [Settings] PES2009 demo
	NOCODEADDR
    // [Settings] PES2009
	NOCODEADDR
    // PES2009 v1.30
	NOCODEADDR
    // PES2009 v1.40
	NOCODEADDR
    // PES2010 demo
    NOCODEADDR
    // PES2010 
    {
        0xed9ca5,
        0xe8e7cb, 
        0xe8f05b, 
        0x10406d1, //eax (sar edx,1)
        0xea1f1e, 0x1211f5c,
        0xed9c35, //0xf7ac60, 0xf7d530,

        0xf7a10e, 0xb3792f,
    },
    // PES2010 v1.1
    {
        0xf394a5, 
        0xeea7db, 
        0xeeb65b, 
        0x109f951,
        0xefdb5e, 0x127ccf8,
        0xf39435, 

        0xfddece, 0xb7fcff,
    },
    // [Settings] PES2010 demo
	NOCODEADDR
    // [Settings] PES2010
	NOCODEADDR
    // PES2010 v1.2
    {
        0xf3b165, 
        0xeec35b, 
        0xeecbeb, 
        0x10a1421,
        0xeff6ee, 0x127f7ec,
        0xf3b0f5, 

        0xfd90fe, 0xb8149f,
    },
    // PES2010 v1.3
    {
        0xf417b5, 
        0xef1a2b, 
        0xef28ab, 
        0x10a8a41,
        0xf05dee, 0x12838d8,
        0xf41745, 

        0xfdf79e, 0xb82aaf,
    },
};

#define DATALEN 6 
enum {
	NEXT_MATCH_DATA_PTR, PLAYERS_DATA,
    TEAM_NAMES, TEAM_KIT_INFO_OFFSET,
    NUM_SLOTS_PTR,
    WIDESCREEN_FLAG,
};

#define NODATAADDR {0,0,0,0,0,0},
DWORD dataArray[][DATALEN] = {
	// PES2008
    NODATAADDR
    NODATAADDR
    NODATAADDR
    NODATAADDR
    NODATAADDR
    NODATAADDR
    NODATAADDR
	NODATAADDR
	// PES2009 demo
	NODATAADDR
    // PES2009
	NODATAADDR
    // PES2009 v1.10
	NODATAADDR
    // PES2009 v1.20
	NODATAADDR
    // [Settings] PES2009 demo
    NODATAADDR
    // [Settings] PES2009
    NODATAADDR
    // PES2009 v1.30
    NODATAADDR
    // PES2009 v1.40
    NODATAADDR
    // PES2010 demo
    NODATAADDR
    // PES2010 
    {
        0x18bd398, 0x18bd3a8,
        0x18bd3a4, 0x1c64c8,
        0x15ce72c,
        0x18bcc40,
    },
    // PES2010 v1.1
    {
        0x193caf8, 0x193cb08,
        0x193cb04, 0x1c64c8,
        0x163dfbc,
        0x193c3a0,
    },
    // [Settings] PES2010 demo
    NODATAADDR
    // [Settings] PES2010
    NODATAADDR
    // PES2010 v1.2
    {
        0x193eb18, 0x193eb28,
        0x193eb24, 0x1c64c8,
        0x163ffdc,
        0x193e3c0,
    },
    // PES2010 v1.3
    {
        0x1942b48, 0x1942b50,
        0x1942b4c, 0x1c64c8,
        0x1644004,
        0x19423e8,
    },
};

DWORD code[CODELEN];
DWORD data[DATALEN];
