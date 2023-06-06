// ADDRESSES for fserv.cpp
BYTE allowedGames[] = {
    gvPES2010demo,
    gvPES2010,
    gvPES2010v11,
    gvPES2010v12,
    gvPES2010v13,
};

#define CODELEN 10
enum { 
    C_CHECK_FACE_AND_HAIR_ID,
    C_GET_FACE_BIN, C_GOT_FACE_BIN,
    C_GET_HAIR_BIN, C_GOT_HAIR_BIN,
    C_RESET_HAIR, C_SQUAD_LIST, C_SET_DEFAULT_PLAYER, //squadlist seems to be not needed
    C_FACEHAIR_READ, C_ONLINE_ENTER,
};

#define NOCODEADDR {0,0,0,0,0,0,0,0,0,0}
DWORD codeArray[][CODELEN] = { 
    // PES2010 demo
    NOCODEADDR,
    // PES2010 
    {
        0, //pes2010.exe+A950BA 
        0, 0,
        0, 0,
        0, 0, 0,
        0, 0,
    },
    // PES2010 v1.01
	NOCODEADDR,
    // PES2010 v1.02
	NOCODEADDR,
    // PES2010 v1.03
	NOCODEADDR,
};

#define DATALEN 2 
enum {
    EDIT_DATA_PTR, MENU_MODE_IDX, // menu mode idx seems to be not needed
};

#define NODATAADDR {0,0}
DWORD dtaArray[][DATALEN] = {
    // PES2010 demo
    NODATAADDR,
    // PES2010 
    {
        0, 0,
    },
    // PES2010 v1.01
    NODATAADDR,

    // PES2010 v1.02
    NODATAADDR,

    // PES2010 v1.03
    NODATAADDR,

};

DWORD code[CODELEN];
DWORD dta[DATALEN];
