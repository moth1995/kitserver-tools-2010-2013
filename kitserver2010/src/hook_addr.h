// ADDRESSES for hook.cpp
#define CODELEN 15
enum {
	C_D3DCREATE_CS,
	C_LOADTEXTUREFORPLAYER_CS, C_LOADTEXTUREFORPLAYER,
	TBL_BEGINRENDER1,	TBL_BEGINRENDER2,
	C_EDITCOPYPLAYERNAME_CS, C_COPYSTRING_CS,
    C_SUB_MENUMODE, C_ADD_MENUMODE,
    C_READ_FILE, C_WRITE_FILE,
    C_COPY_DATA, C_COPY_DATA2,
    C_ENTER_UNIFORM_SELECT, C_EXIT_UNIFORM_SELECT,
};

#define NOCODEADDR {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
DWORD codeArray[][CODELEN] = { 
  // PES2008 DEMO
  NOCODEADDR
  // [Settings] PES2008 PC DEMO
  NOCODEADDR
  // PES2008
  NOCODEADDR
  // [Settings] PES2008 PC
  NOCODEADDR
  NOCODEADDR
  // PES2008 1.10
  NOCODEADDR
  NOCODEADDR
  // PES2008 1.20
  NOCODEADDR
  // PES2009 DEMO
  NOCODEADDR
  // PES2009
  NOCODEADDR
  // PES2009 1.10
  NOCODEADDR
  // PES2009 1.20
  NOCODEADDR
  // [Settings] PES2009 PC demo
  NOCODEADDR
  // [Settings] PES2009 PC 
  NOCODEADDR
  // PES2009 PC 1.30
  NOCODEADDR
  // PES2009 PC 1.40
  NOCODEADDR
  // PES2010 DEMO
  {
		0xb91895,
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0, 0,
        0, 0,
        0, 0,
  },
  // PES2010
  {
		0xf1c665,
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0xf4fc33, 0xf4fafa,
        0, 0,
        0xf7ac60, 0x40ffbf,
  },
  // PES2010 1.1
  {
		0xf7c315,
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0xfaf433, 0xfaf2fa,
        0, 0,
        0xfdea20, 0x40fe7f, 
  },
  // [Settings] PES2009 PC demo
  NOCODEADDR
  // [Settings] PES2009 PC 
  NOCODEADDR
  // PES2010 1.2
  {
		0xf7dd65,
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0xfb0e83, 0xfb0d4a,
        0, 0,
        0xfd9c50, 0x41041f, 
  },
  // PES2010 1.3
  {
		0xf855d5,
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0xfb8623, 0xfb84ea,
        0, 0,
        0xfe02f0, 0x410fef, 
  },
};

#define DATALEN 12
enum {
	PLAYERDATA, ISREFEREEADDR,
	GENERALINFO, REPLAYGENERALINFO,
    MENU_MODE_IDX, MAIN_SCREEN_INDICATOR, INGAME_INDICATOR,
    NEXT_MATCH_DATA_PTR, CUP_MODE_PTR, EDIT_DATA_PTR,
    EDIT_DATA_SIZE, REPLAY_DATA_SIZE,
};

#define NODATAADDR {0,0,0,0,0,0,0,0,0,0,0,0},
DWORD dataArray[][DATALEN] = {
    // PES2008 DEMO
    NODATAADDR
	// [Settings] PES2008 PC DEMO
	NODATAADDR
	// PES2008 PC
    NODATAADDR
    // [Settings] PES2008 PC
	NODATAADDR
	NODATAADDR
	// PES2008 PC 1.10
    NODATAADDR
	NODATAADDR
	// PES2008 PC 1.20
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
    // PES2009 PC 1.30
	NODATAADDR
    // PES2009 PC 1.40
	NODATAADDR
    // PES2010 DEMO
    NODATAADDR
    // PES2010
    {
        0, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        0x504388, 0x3fa020,
    },
    // PES2010 1.1
    {
        0, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        0x504388, 0x3fa020,
    },
    // [Settings] PES2010 PC demo
	NODATAADDR
    // [Settings] PES2010 PC
	NODATAADDR
    // PES2010 1.2
    {
        0, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        0x504388, 0x3fa020,
    },
    // PES2010 1.3
    {
        0, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        0x504388, 0x3fa020,
    },
};

#define LTFPLEN 15
#define NOLTFPADDR {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
BYTE ltfpPatchArray[][LTFPLEN] = {
	// PES2008 DEMO
	{
		0x8b, 0xf8,														// MOV EDI, EAX
		0x8b, 0x44, 0x24, 0x38, 							// MOV EAX,DWORD PTR SS:[ESP+38]
		0x83, 0xf8, 0x04,											// CMP EAX,4
		0x0f, 0x87, 0x1f, 0x06, 0x00, 0x00,		// JA 0043655B
	},
	// [Settings] PES2008 PC DEMO
	NOLTFPADDR
	// PES2008 PC
	{
		0x8b, 0xf8,														// MOV EDI, EAX
		0x8b, 0x44, 0x24, 0x38, 							// MOV EAX,DWORD PTR SS:[ESP+38]
		0x83, 0xf8, 0x04,											// CMP EAX,4
		0x0f, 0x87, 0x1f, 0x06, 0x00, 0x00,		// JA 0043655B
	},
    // [Settings] PES2008 PC
	NOLTFPADDR
	NOLTFPADDR
	// PES2008 PC 1.10
	{
		0x8b, 0xf8,														// MOV EDI, EAX
		0x8b, 0x44, 0x24, 0x38, 							// MOV EAX,DWORD PTR SS:[ESP+38]
		0x83, 0xf8, 0x04,											// CMP EAX,4
		0x0f, 0x87, 0x1f, 0x06, 0x00, 0x00,		// JA 0043E88B
	},
	NOLTFPADDR
    // PES2008 PC 1.20
	NOLTFPADDR //TODO
    // PES2009 DEMO
	NOLTFPADDR //TODO
    // PES2009 
	NOLTFPADDR //TODO
    // PES2009 1.10
	NOLTFPADDR //TODO
    // PES2009 1.20
	NOLTFPADDR //TODO
    // [Settings] PES2009 DEMO
	NOLTFPADDR //TODO
    // [Settings] PES2009
	NOLTFPADDR //TODO
    // PES2009 1.30
	NOLTFPADDR //TODO
    // PES2009 1.40
	NOLTFPADDR //TODO
    // PES2010 DEMO
	NOLTFPADDR //TODO
    // PES2010
	NOLTFPADDR //TODO
    // PES2010 1.1
	NOLTFPADDR //TODO
    // [Settings] PES2009 DEMO
	NOLTFPADDR
    // [Settings] PES2009
	NOLTFPADDR
    // PES2010 1.2
	NOLTFPADDR
    // PES2010 1.3
	NOLTFPADDR
};


DWORD code[CODELEN];
DWORD data[DATALEN];
BYTE ltfpPatch[LTFPLEN];
