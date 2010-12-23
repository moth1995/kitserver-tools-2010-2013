// ADDRESSES for afsio.cpp
BYTE allowedGames[] = {
    gvPES2011demo,
    gvPES2011,
    gvPES2011v101,
    gvPES2011v102,
    gvPES2011v103,
};

#define CODELEN 9
enum {
    C_AT_GET_BUFFERSIZE, C_BEFORE_READ,
    C_AFTER_CREATE_EVENT, C_AT_CLOSE_HANDLE, C_AFTER_GET_OFFSET_PAGES, 
    C_AT_GET_SIZE1, C_AT_GET_SIZE2,
    C_AT_GET_IMG_SIZE1, C_AT_GET_IMG_SIZE2,
};

#define NOCODEADDR {0,0,0,0,0,0,0,0,0},
DWORD codeArray[][CODELEN] = { 
    // PES2011 demo
    {
        0xc349a1, 0x4dd7a9,
        0x4d19e3, 0x4d1c05, 0x4cf58b, 
        0x4cf4f9, 0x4cf510,
        0x4d1a59, 0x4d10fa,
    },
    // PES2011
    {
        0xca54e1, 0x4de6a9,
        0x4d28f3, 0x4d2b15, 0x4d049b, 
        0x4d0409, 0x4d0420,
        0x4d2969, 0x4d200a,
    },
    // PES2011 v1.01
    {
        0xca59f1, 0x4de699,
        0x4d28c3, 0x4d2ae5, 0x4d046b, 
        0x4d03d9, 0x4d03f0,
        0x4d2939, 0x4d1fda,
    },
    // PES2011 v1.02
    {
        0xca5a81, 0x4de6c9,
        0x4d28f3, 0x4d2b15, 0x4d049b, 
        0x4d0409, 0x4d0420,
        0x4d2969, 0x4d200a,
    },
    // PES2011 v1.03
    {
        0xca5a91, 0x4de6c9,
        0x4d28f3, 0x4d2b15, 0x4d049b, 
        0x4d0409, 0x4d0420,
        0x4d2969, 0x4d200a,
    },
};

#define DATALEN 1 
enum {
    BIN_SIZES_TABLE
};

#define NODATAADDR {0},
DWORD dataArray[][DATALEN] = {
    // PES2011 demo
    {
        0x1910e80,
    },
    // PES2011
    {
        0x199f220,
    },
    // PES2011 v1.01
    {
        0x199f280,
    },
    // PES2011 v1.02
    {
        0x199f280,
    },
    // PES2011 v1.03
    {
        0x199f280,
    },
};

DWORD code[CODELEN];
DWORD data[DATALEN];
