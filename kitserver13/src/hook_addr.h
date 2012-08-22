// ADDRESSES for hook.cpp
#define CODELEN 19
enum {
	C_D3DCREATE_CS,
	C_LOADTEXTUREFORPLAYER_CS, C_LOADTEXTUREFORPLAYER,
	TBL_BEGINRENDER1,	TBL_BEGINRENDER2,
	C_EDITCOPYPLAYERNAME_CS, C_COPYSTRING_CS,
    C_SUB_MENUMODE, C_ADD_MENUMODE,
    C_READ_FILE, C_WRITE_FILE,
    C_COPY_DATA, C_COPY_DATA2,
    C_ENTER_UNIFORM_SELECT, C_EXIT_UNIFORM_SELECT,
    C_WRITE_DATA,
    C_READ_NAMES, C_READ_NAMES2,
    C_READ_DATA,
};

#define NOCODEADDR {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
DWORD codeArray[][CODELEN] = { 
   // PES2013 demo 1
  {
		0x10D2A45, //CALL JMP d3d9 direct3Dcreate9
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0, 0,
        0, 0,
        0, 0, 
        0,//0x12cb548, //0x12c8d58, 
        0,0,//0x11a8e2c, 0x12cc24c, //0x11a67fc, 0x12c9a5c,
        0,//0xd40e4b, //0xd3e3fb,
  },
};

#define DATALEN 13
enum {
	PLAYERDATA, ISREFEREEADDR,
	GENERALINFO, REPLAYGENERALINFO,
    MENU_MODE_IDX, MAIN_SCREEN_INDICATOR, INGAME_INDICATOR,
    NEXT_MATCH_DATA_PTR, CUP_MODE_PTR, EDIT_DATA_PTR,
    EDIT_DATA_SIZE, REPLAY_DATA_SIZE, BAL_DATA_SIZE,
};

#define NODATAADDR {0,0,0,0,0,0,0,0,0,0,0,0,0},
DWORD dataArray[][DATALEN] = {    
    //  PES2013 demo 1
    {
        0, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        123, 456, 789,
    },
};

#define LTFPLEN 15
#define NOLTFPADDR {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
BYTE ltfpPatchArray[][LTFPLEN] = {
	NOLTFPADDR
};

DWORD code[CODELEN];
DWORD data[DATALEN];
BYTE ltfpPatch[LTFPLEN];
