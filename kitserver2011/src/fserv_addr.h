// ADDRESSES for fserv.cpp
BYTE allowedGames[] = {
    gvPES2011,
    gvPES2011v101,
    gvPES2011v102,
    gvPES2011v103,
};

#define CODELEN 3
enum { 
    C_CHECK_FACE_AND_HAIR_ID, C_COPY_DATA, C_COPY_DATA2,
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
    //NOCODEADDR, // 0x109e2b3
    0, 0x4b974a, 0, //0x4b97a8,

    /*
    // PES2008 DEMO
    NOCODEADDR,
    // [Settings] PES2008 PC DEMO
    NOCODEADDR,
    // PES2008
    {
        0xb6efcf, 0xc96248, 0x41433f,
    },
    // [Settings] PES2008 PC
    NOCODEADDR,
    NOCODEADDR,
    // PES2008 1.10
    {
        0xb6edef, 0xc97bd8, 0x413e8f,
    },
    NOCODEADDR,
    // PES2008 1.20
    { 
        0xb734cf, 0xc99b28, 0x414caf,
    },
    */
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
