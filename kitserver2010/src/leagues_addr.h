// ADDRESSES for camera module
BYTE allowedGames[] = {
    gvPES2010demo,
    gvPES2010,
    gvPES2010v11,
    gvPES2010v12,
	gvPES2010v13
};

#define CODELEN 5
enum { 
    C_READ_NATIONALS, C_READ_CLUBS, C_MAKE_LEAGUE, C_READ_RELINK, C_READ_NAMES
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
    // [Settings] PES2009 PC demo
    NOCODEADDR,
    // [Settings] PES2009 PC
    NOCODEADDR,
    // PES2009 PC 1.30
    NOCODEADDR,
    // PES2009 PC 1.40
    NOCODEADDR,
    // PES2010 Demo
    NOCODEADDR,
    // PES2010 
    {
        0xf6e1f3, 0xf6e134, 0xf6d910, 0xf6d92b, 0xf6d948
    },
    // PES2010 1.1
    {
        0xfd3003, 0xfd2f44, 0xfd2720, 0xfd273b, 0xfd2758
    },
    // [Settings] PES2010 PC demo
    NOCODEADDR,
    // [Settings] PES2010 PC
    NOCODEADDR,
    // PES2010 1.2
    {
        0xfd3003, 0xfd2f44, 0xfd2720, 0xfd273b, 0xfd2758
    },
	// PES2010 1.3
	{
		0xfda173, 0xfda0b4, 0xfd9890, 0xfd98ab, 0xfd98c8
	}
};

#define DATALEN 3 
enum {
    D_RELINKS_OFFSET, D_NAMES_OFFSET, D_LOGOS_OFFSET
};

#define NODATAADDR {0,0,0}
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
	{
		0x1204674, 0x12046a0, 0x1607a40
	},
    // PES2010 PC 1.1
    {
		0x12704b4, 0x12704e0, 0x16794c0
	},
    // [Settings] PES2010 PC demo
    NODATAADDR,
    // [Settings] PES2010 PC 
    NODATAADDR,
    // PES2010 PC 1.2
	{
		0x12704b4, 0x12704e0, 0x16794c0
	},
	// PES2010 PC 1.3
	{
		0x12744bc, 0x12744e8, 0x167d4e4
	}
};

DWORD code[CODELEN];
DWORD data[DATALEN];
