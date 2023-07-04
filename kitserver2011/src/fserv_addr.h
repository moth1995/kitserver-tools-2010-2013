// ADDRESSES for fserv.cpp
BYTE allowedGames[] = {
    gvPES2011,
    gvPES2011v101,
    gvPES2011v102,
    gvPES2011v103,
};

#define CODELEN 10
enum { 
    C_CHECK_FACE_AND_HAIR_ID, //C_CHECK_FACE_ID, C_CHECK_HAIR_ID,
    C_GET_FACE_BIN, C_GOT_FACE_BIN,
    C_GET_HAIR_BIN, C_GOT_HAIR_BIN,
    C_RESET_HAIR, C_SQUAD_LIST, C_SET_DEFAULT_PLAYER,
    C_FACEHAIR_READ, C_ONLINE_ENTER,
};

#define NOCODEADDR {0,0,0,0,0,0,0,0,0,0}
DWORD codeArray[][CODELEN] = { 
    // PES2011 demo
    NOCODEADDR,
    // PES2011 
    {
        0x10dc296,
        0x10d4a13, 0x10d4a6e,
        0x119d945, 0x119d987,
        0x6d9d71, 0, 0x7a5eaf,
        0x10dbc4b, 0xc9e393,
    },
    // PES2011 v1.01
    {
        0x10dc7f6,
        0x10d4f73, 0x10d4fce,
        0x119de95, 0x119ded7,
        0x6d9bf1, 0, 0x7a66bf,
        0x10dc1ab, 0xc9e8a3,
    },
    // PES2011 v1.02
    {
        0x10dc956,
        0x10d50d3, 0x10d512e,
        0x119dff5, 0x119e037,
        0x6d9c61, 0, 0x7a642f,
        0x10dc30b, 0xc9e933,
    },
    // PES2011 v1.03
    { 
        0x10dc966, //0x10d50e7, 0x119e035,
        0x10d50e3, 0x10d513e,
        0x119e035, 0x119e077,
        0x6d9c71, 0x6d9cb1, 0x7a644f,
        0x10dc31b, 0xc9e943,
    },
};

#define DATALEN 2 
enum {
    EDIT_DATA_PTR, MENU_MODE_IDX,
    //PFIRST_FACE, PFIRST_HAIR,
};

#define NODATAADDR {0,0}
DWORD dtaArray[][DATALEN] = {
    // PES2011 demo
    NODATAADDR,
    // PES2011 
    {
        0x1940b08, 0,
    },
    // PES2011 v1.01
    {
        0x1940b60, 0,
    },
    // PES2011 v1.02
    {
        0x1940b58, 0,
    },
    // PES2011 v1.03
    {
        0x1940b5c, 0,
        //0x15a4e2c, 0x15a2cbc,
    },
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
