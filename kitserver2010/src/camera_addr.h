// ADDRESSES for camera module
BYTE allowedGames[] = {
    gvPES2010demo,
    gvPES2010,
    gvPES2010v11,
    gvPES2010v12,
    gvPES2010v13,
};

#define CODELEN 1
enum { 
    C_READ_CAMERA_ANGLE,
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
    // PES2009 Demo
    NOCODEADDR,
    // PES2009 
    NOCODEADDR,
    // PES2009 1.10
    NOCODEADDR,
    // PES2009 1.20
    NOCODEADDR,
    // [Settings] PES2009 PC demo
    NOCODEADDR,
    // [Settings] PES2009 PC
    NOCODEADDR,
    // PES2009 PC 1.30
    NOCODEADDR,
    // PES2009 PC 1.40
    NOCODEADDR,
    // PES2010 Demo
    {
        0xad4108,
    },
    // PES2010 
    {
        0xdcdcf8,
    },
    // PES2010 1.1
    {
        0xe29948,
    },
    // [Settings] PES2010 PC demo
    NOCODEADDR,
    // [Settings] PES2010 PC
    NOCODEADDR,
    // PES2010 1.2
    {
        0xe2b4d8,
    },
    // PES2010 1.3
    {
        0xe31468,
    },
};

#define DATALEN 1 
enum {
    DUMMY
};

#define NODATAADDR {0}
DWORD dataArray[][DATALEN] = {
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
    // PES2009 PC DEMO
    NODATAADDR,
    // PES2009 PC 
    NODATAADDR,
    // PES2009 PC 1.10
    NODATAADDR,
    // PES2009 PC 1.20
    NODATAADDR,
    // [Settings] PES2009 PC demo
    NODATAADDR,
    // [Settings] PES2009 PC 
    NODATAADDR,
    // PES2009 PC 1.30
    NODATAADDR,
    // PES2009 PC 1.40
    NODATAADDR,
    // PES2010 PC demo
    NODATAADDR,
    // PES2010 PC
    NODATAADDR,
    // PES2010 PC 1.1
    NODATAADDR,
    // [Settings] PES2010 PC demo
    NODATAADDR,
    // [Settings] PES2010 PC 
    NODATAADDR,
    // PES2010 PC 1.2
    NODATAADDR,
    // PES2010 PC 1.3
    NODATAADDR,
};

DWORD code[CODELEN];
DWORD data[DATALEN];
