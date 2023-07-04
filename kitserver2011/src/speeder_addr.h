// ADDRESSES for speeder module
BYTE allowedGames[] = {
    gvPES2011demo,
    gvPES2011,
    gvPES2011v101,
    gvPES2011v102,
    gvPES2011v103,
};

#define CODELEN 1
enum { 
    DUMMY1,
};

#define NOCODEADDR {0}
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
    NOCODEADDR,
};

#define DATALEN 1 
enum {
    DUMMY
};

#define NODATAADDR {0}
DWORD dtaArray[][DATALEN] = {
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
DWORD dta[DATALEN];
