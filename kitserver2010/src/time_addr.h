// ADDRESSES for time module
BYTE allowedGames[] = {
    gvPES2010,
    gvPES2010v11,
    gvPES2010v12,
    gvPES2010v13,
};

#define CODELEN 5
enum { 
    C_TIME_READ1, C_TIME_READ2, C_READ_SETTINGS, C_RESTORE_SETTINGS,
    C_SET_CUP_ENDURANCE,
};

#define NOCODEADDR {0,0,0,0,0}
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
    // PES2009 Demo
    NOCODEADDR,
    // PES2009 
    NOCODEADDR,
    // PES2009 1.10
    NOCODEADDR,
    // PES2009 1.20
    NOCODEADDR,
    // [Settings] PES2009 PC DEMO
    NOCODEADDR,
    // [Settings] PES2009 PC
    NOCODEADDR,
    // PES2009 PC 1.30
    NOCODEADDR,
    // PES2009 PC 1.40
    NOCODEADDR,
    // PES2010 PC demo
    NOCODEADDR,
    // PES2010 
    { 
        0xb0ed98, 0xb165e3, 
        0xb1ac4a, 0xf825ee,
        0x4f6ca1,
    },
    // PES2010 1.1 
    { 
        0xb55908, 0xb5cfbd, 
        0xb6151a, 0xfe23ce,
        0x4f2a21,
    },
    // [Settings] PES2010 PC DEMO
    NOCODEADDR,
    // [Settings] PES2010 PC
    NOCODEADDR,
    // PES2010 1.2 
    { 
        0xb56eb8, 0xb5e56d, 
        0xb62aca, 0xfe306e,
        0x4f2b91,
    },
    // PES2010 1.3 
    { 
        0xb586c8, 0xb5fe8d, 
        0xb63b9a, 0xfe94de,
        0x4f5341,
    },
};

#define DATALEN 1 
enum {
    SETTINGS_ADDR
};

#define NODATAADDR {0}
DWORD dtaArray[][DATALEN] = {
    // PES2008 DEMO
    NODATAADDR,
    // [Settings] PES2008 PC DEMO
    NODATAADDR,
    // PES2008
    NODATAADDR,
    // [Settings] PES2008 PC
    NODATAADDR,
    NODATAADDR,
    // PES2008 1.10
    NODATAADDR,
    NODATAADDR,
    // PES2008 1.20
    NODATAADDR,
    // PES2009 Demo
    NODATAADDR,
    // PES2009 
    NODATAADDR,
    // PES2009 1.10
    NODATAADDR,
    // PES2009 1.20
    NODATAADDR,
    // [Settings] PES2009 PC DEMO
    NODATAADDR,
    // [Settings] PES2009 PC 
    NODATAADDR,
    // PES2009 PC 1.30
    NODATAADDR,
    // PES2009 PC 1.40
    NODATAADDR,
    // PES2010 demo
    NODATAADDR,
    // PES2010 
    {
        0x19593b8,
    },
    // PES2010 1.1
    {
        0x19d8b30,
    },
    // [Settings] PES2010 PC DEMO
    NODATAADDR,
    // [Settings] PES2010 PC 
    NODATAADDR,
    // PES2010 1.2
    {
        0x19dab48,
    },
    // PES2010 1.3
    {
        0x19deb78,
    },
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
