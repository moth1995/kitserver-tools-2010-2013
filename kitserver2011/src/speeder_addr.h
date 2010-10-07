// ADDRESSES for speeder module
BYTE allowedGames[] = {
    gvPES2011demo,
    gvPES2011,
    gvPES2011v101,
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
};

DWORD code[CODELEN];
DWORD data[DATALEN];
