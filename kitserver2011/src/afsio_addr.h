// ADDRESSES for afsio.cpp
BYTE allowedGames[] = {
    gvPES2011demo,
    gvPES2011,
    gvPES2011v101,
};

#define CODELEN 5
enum {
    C_AT_GET_SIZE1, C_AT_GET_SIZE2, C_AT_GET_BUFFER_SIZE,
    C_AT_GET_IMG_SIZE, C_AT_GET_IMG_SIZE2,
};

#define NOCODEADDR {0,0,0,0,0},
DWORD codeArray[][CODELEN] = { 
    // PES2011 demo
    {
        0x4cf4f9, 0x4cf510, 0xc349a1,
        0x4d1a59, 0x4d10fa,
    },
    // PES2011 
    {
        0x4d0409, 0x4d0420, 0xca54e1,
        0x4d2969, 0x4d200a,
    },
    // PES2011 v1.01
    {
        0x4d03d9, 0x4d03f0, 0xca59f1,
        0x4d2939, 0x4d1fda,
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
};

DWORD code[CODELEN];
DWORD data[DATALEN];
