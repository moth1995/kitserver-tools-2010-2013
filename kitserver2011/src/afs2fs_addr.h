// ADDRESSES for afs2fs.cpp
BYTE allowedGames[] = {
    gvPES2011demo,
    gvPES2011,
    gvPES2011v101,
    gvPES2011v102,
};

#define CODELEN 1
enum { 
    C_READ_BALLS 
};

#define NOCODEADDR {0}
DWORD codeArray[][CODELEN] = { 
    // PES2011 demo
    {
        0x42d36a,
    },
    // PES2011
    {
        0x42d40a,
    },
    // PES2011 v1.01
    {
        0x42d28a,
    },
    // PES2011 v1.02
    {
        0x42d30a,
    },
};

#define DATALEN 5 
enum {
    BIN_SIZES_TABLE, SONGS_INFO_TABLE, NUM_SONGS, NUM_BALLS, PES_DIR,
};

#define NODATAADDR {0,0,0,0,0},
DWORD dataArray[][DATALEN] = {
    // PES2011 demo
    {
        0x1910e80, 0x12a03c0, 50, 28, 0,
    },
    // PES2011
    {
        0x199f220, 0x13219c0, 50, 28, 0
    },
    // PES2011 v1.01
    {
        0x199f280, 0x1321918, 50, 28, 0
    },
    // PES2011 v1.02
    {
        0x199f280, 0x13218f8, 50, 28, 0
    },
};

DWORD code[CODELEN];
DWORD data[DATALEN];
