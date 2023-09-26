// ADDRESSES for afsio.cpp
BYTE allowedGames[] = {
    gvPES2013demo1,
    gvPES2013v104,
};

#define CODELEN 10
enum {
    C_AT_GET_BUFFERSIZE, C_BEFORE_READ, C_BEFORE_READ2,
    C_AFTER_CREATE_EVENT, C_AT_CLOSE_HANDLE, C_AFTER_GET_OFFSET_PAGES, 
    C_AT_GET_SIZE1, C_AT_GET_SIZE2,
    C_AT_GET_IMG_SIZE1, C_AT_GET_IMG_SIZE2,
};

#define NOCODEADDR {0,0,0,0,0,0,0,0,0,0},
DWORD codeArray[][CODELEN] = { 
    // PES2013 demo 1
    {
        0xcba06f, 0x502A09, 0x502809,//0xcba06a
        0x4f51a3, 0x4f53cb, 0x4f2b6f,
        0x4f2a95, 0x4f2ac5,
        0x4f521c, 0x4f4aca
    }, 
    // PES2013 v1.04
    {
        0, 0, 0,
        0, 0, 0,
        0, 0,
        0, 0
    },
};

#define DATALEN 1 
enum {
    BIN_SIZES_TABLE
};

#define NODATAADDR {0},
DWORD dtaArray[][DATALEN] = {
    // PES2013 demo 1
    {
        0x19972E0,
    }, 
    // PES2013 v1.04
    {
        0,
    },
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
