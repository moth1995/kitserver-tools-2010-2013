// ADDRESSES for fserv.cpp
BYTE allowedGames[] = {
    gvPES2011,
    gvPES2011v101,
    gvPES2011v102,
    gvPES2011v103,
};

#define CODELEN 7
enum { 
    C_CHECK_FACE_AND_HAIR_ID, C_CHECK_FACE_ID, C_CHECK_HAIR_ID,
    C_GET_FACE_BIN, C_GOT_FACE_BIN,
    C_GET_HAIR_BIN, C_GOT_HAIR_BIN,
};

#define NOCODEADDR {0,0,0,0,0,0,0}
DWORD codeArray[][CODELEN] = { 
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
        0x10dc946, 0x10d50e7, 0x119e035,
        0x10d50e3, 0x10d513e,
        0x119e034, 0x119e077,
    },
};

#define DATALEN 4 
enum {
    EDIT_DATA_PTR, MENU_MODE_IDX,
    PFIRST_FACE, PFIRST_HAIR,
};

#define NODATAADDR {0,0,0,0},
DWORD dataArray[][DATALEN] = {
    // PES2011 demo
    NODATAADDR
    // PES2011 
    NODATAADDR
    // PES2011 v1.01
    NODATAADDR
    // PES2011 v1.02
    NODATAADDR
    // PES2011 v1.03
    {
        0x1940b5c, 0,
        0x15a4e2c, 0x15a2cbc,
    },
};

DWORD code[CODELEN];
DWORD data[DATALEN];
