// ADDRESSES for fserv.cpp
BYTE allowedGames[] = {
    gvPES2011,
    gvPES2011v101,
    gvPES2011v102,
    gvPES2011v103,
};

#define CODELEN 3
enum { 
    C_CHECK_FACE_AND_HAIR_ID, C_CHECK_FACE_ID, C_CHECK_HAIR_ID,
};

#define NOCODEADDR {0,0,0}
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
    },
};

#define DATALEN 2 
enum {
    EDIT_DATA_PTR, MENU_MODE_IDX,
};

#define NODATAADDR {0,0},
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
    },
};

DWORD code[CODELEN];
DWORD data[DATALEN];
