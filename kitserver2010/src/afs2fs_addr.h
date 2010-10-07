// ADDRESSES for afs2fs.cpp
BYTE allowedGames[] = {
    gvPES2010demo,
    gvPES2010,
    gvPES2010v11,
    gvPES2010v12,
    gvPES2010v13,
};

#define CODELEN 1
enum { 
    C_READ_BALLS 
};

#define NOCODEADDR {0}
DWORD codeArray[][CODELEN] = { 
    // PES2008 DEMO
    NOCODEADDR,
    // [Settings] PES2008 PC DEMO
    NOCODEADDR,
    // PES2008
    NOCODEADDR,
    // [Settings] PES2008 PC
    NOCODEADDR,
    NOCODEADDR,
    // PES2008 1.10
    NOCODEADDR,
    NOCODEADDR,
    // PES2008 1.20
    NOCODEADDR,
    // PES2009 DEMO
    NOCODEADDR,
    // PES2009
    NOCODEADDR,
    // PES2009 1.10
    NOCODEADDR,
    // PES2009 1.20
    NOCODEADDR,
    // [Settings] PES2009 demo
    NOCODEADDR,
    // [Settings] PES2009
    NOCODEADDR,
    // PES2009 1.30
    NOCODEADDR,
    // PES2009 1.40
    NOCODEADDR,
    // PES2010 demo
    {
        0x43759a,
    },
    // PES2010
    {
        0x44670a,
    },
    // PES2010 1.1
    {
        0x445cea,
    },
    // [Settings] PES2010 demo
    NOCODEADDR,
    // [Settings] PES2010
    NOCODEADDR,
    // PES2010 1.2
    {
        0x445afa,
    },
    // PES2010 1.3
    {
        0x446c0a,
    },
};

#define DATALEN 4 
enum {
    BIN_SIZES_TABLE, SONGS_INFO_TABLE, NUM_SONGS, PES_DIR,
};

#define NODATAADDR {0,0,0,0},
DWORD dataArray[][DATALEN] = {
    // PES2008 DEMO
    NODATAADDR
    // [Settings] PES2008 PC DEMO
    NODATAADDR
    // PES2008
    NODATAADDR
    // [Settings] PES2008 PC
    NODATAADDR
    NODATAADDR
    // PES2008 1.10
    NODATAADDR
    NODATAADDR
    // PES2008 1.20
    NODATAADDR
    // PES2009 DEMO
    NODATAADDR
    // PES2009 
    NODATAADDR
    // PES2009 1.10
    NODATAADDR
    // PES2009 1.20
    NODATAADDR
    // [Settings] PES2009 PC demo
    NODATAADDR
    // [Settings] PES2009 PC
    NODATAADDR
    // PES2009 1.30
    NODATAADDR
    // PES2009 1.40
    NODATAADDR
    // PES2010 demo
    {
        0x11e30c0, 0xdd5d98, 5, 0x11dce4c,
    },
    // PES2010
    {
        0x1980760, 0x1208bc0, 47, 0x197a4cc,
    },
    // PES2010 1.1
    {
        0x1a1b860, 0x1272828, 47, 0x19f9c6c,
    },
    // [Settings] PES2010 PC demo
    NODATAADDR
    // [Settings] PES2010 PC
    NODATAADDR
    // PES2010 1.2
    {
        0x1a01f00, 0x12749a8, 47, 0x19fbc5c,
    },
    // PES2010 1.3
    {
        0x1a21860, 0x12787e8, 47, 0x19ffc94,
    },
};

DWORD code[CODELEN];
DWORD data[DATALEN];
