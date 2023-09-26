// ADDRESSES for speeder module
BYTE allowedGames[] = {
    gvPES2013demo1,
    gvPES2013v104,
};

#define CODELEN 1
enum { 
    CS_QUERY_PERFORMANCE_FREQUENCY,
};

#define NOCODEADDR {0}
DWORD codeArray[][CODELEN] = { 
	// PES2013 demo 1
    {
        0x10A3099,
    },
    // PES2013 v1.04
    {
        0x10A3099,
    }
};

#define DATALEN 1 
enum {
    DUMMY
};

#define NODATAADDR {0}
DWORD dtaArray[][DATALEN] = {
    // PES2013 demo 1
    NODATAADDR,
    // PES2013 v1.04
    NODATAADDR
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
