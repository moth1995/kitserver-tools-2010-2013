// ADDRESSES for fserv.cpp
BYTE allowedGames[] = {
    gvPES2013demo1,
};

#define CODELEN 10
enum { 
    C_CHECK_FACE_AND_HAIR_ID, //C_CHECK_FACE_ID, C_CHECK_HAIR_ID,
    C_GET_FACE_BIN, C_GOT_FACE_BIN,
    C_GET_HAIR_BIN, C_GOT_HAIR_BIN,
    C_RESET_HAIR, C_SQUAD_LIST, C_SET_DEFAULT_PLAYER,
    C_FACEHAIR_READ, C_ONLINE_ENTER,
};

#define NOCODEADDR {0,0,0,0,0,0,0,0,0,0}
DWORD codeArray[][CODELEN] = { 
	// PES2013 demo 1
	{
		0x1145DD1, 
		0x11467a3, 
		0x1146807, 
		0x121B6C2,
		0x121B737,
		0x72E38F,  
		0,
		0x7B9B5F,  
		0x114578B,
		0xCB2F23,  
	},
};

#define DATALEN 2 
enum {
    EDIT_DATA_PTR, MENU_MODE_IDX,
    //PFIRST_FACE, PFIRST_HAIR,
};

#define NODATAADDR {0,0}
DWORD dataArray[][DATALEN] = {
	// PES2013 demo 1
	{
		0x19326A4,0, //
	}
};

DWORD code[CODELEN];
DWORD data[DATALEN];
