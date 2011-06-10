// ADDRESSES for sides module
BYTE allowedGames[] = {
    gvPES2011,
    gvPES2011v101,
    gvPES2011v102,
    gvPES2011v103,
};

#define CODELEN 4
enum { 
    CS1, CS2, CS3, CS4,
};

#define NOCODEADDR {0,0,0,0}
DWORD codeArray[][CODELEN] = { 
    // PES2011 demo
    NOCODEADDR,
    // PES2011 
    {
        0x4fcbce, 0x4fb79b, 0x4fc41a, 0x4fc38e,
    },
    // PES2011 v1.01
    {
        0x4fcbae, 0x4fb77b, 0x4fc3fa, 0x4fc36e,
    },
    // PES2011 v1.02
    {
        0x4fcc5e, 0x4fb82b, 0x4fc4aa, 0x4fc41e,
    },
    // PES2011 v1.03
    {
        0x4fcc5e, 0x4fb82b, 0x4fc4aa, 0x4fc41e,
    },
};

#define DATALEN 1 
enum {
    DUMMY
};

#define NODATAADDR {0}
DWORD dataArray[][DATALEN] = {
    // PES2011 demo
    NODATAADDR,
    // PES2011 
    NODATAADDR,
    // PES2011 v1.01
    NODATAADDR,
    // PES2011 v1.02
    NODATAADDR,
    // PES2011 v1.03
    NODATAADDR,
};

DWORD code[CODELEN];
DWORD data[DATALEN];
