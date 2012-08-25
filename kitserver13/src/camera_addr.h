
#define Cam_length 0x4C
#define ESPILON 0.00001
#define Cam_ID_max 19
#define Cam_Set_max 7

//
DWORD menu_transparent[] = {0x14978E8}; //f255

int cam_setting_ID[Cam_ID_max] = {18,0,1,2,3,4,5,6,7,8,9,0xA,0xB,0xC,0xD,0xE,0xF,0x10,0x11};

DWORD cam_switch_base[] = {0xE0D458};

DWORD cam_angle_base[] =  {0xE0D3D0};

DWORD cam_setting_base[] = {0x1442090};

DWORD realtime_cam_base[] = {0xDA3F23};

DWORD start_match_cam[] = {0x144E9F0}; //float 4

DWORD radar_base[] = {0xC168B0};

//17 : vertical
//15 : vertical right

DWORD game_mode_base[] = {0x1932208}; 

DWORD powerbar_size_base[] = {0xC0F877}; 
//00 01 02 03 03 03 04 04 02 07 07 07 07 05 06 CC

DWORD cam_order[Cam_Set_max] = {6,0,2,1,4,7,8};

DWORD cam_setting_order[9] = {8,17975334,17975410,17975486,17975562,17975638,17975866,17976246,17976322};

float cam_default[Cam_ID_max][Cam_Set_max] = {{18,6,0,0,0,0},
{0,600,0.36,19.05,6675,12416,1},
{0,700,0.45,19.05,8495,12004,1.2},
{0,1000,0.52,19.05,11183,11262,1.3},
{0,1100,0.46,16.05,18016,11456,1.4},
{0,1100,0.3,14.21,19000,11456,1},
{0,1000,0.49,14.9,29000,8000,1},
{0,550,0.26,14.21,20000,11868,0.16},
{0,256,0.29,13.96,15640,11100,0.18},
{0,258,0.23,14.41,15640,12728,0.41},
{0,416,0.32,19.05,6675,11776,1.13},
{0,0,-1894,19.05,-7320,10831,1},
{0,0,-2642,19.05,-10017,9954,1},
{0,0,-4069,19.05,-13485,9040,1},
{0,800,0.3,40.05,4100,7300,1},
{0,0,0.38,43.6,2384,13465,1},
{0,0,0.38,66.9,43,13465,1},
{0,0,-0.45,45.7,-875,13465,1},
{0,800,0.3,40.05,4100,7300,1},
};

float cam_setting[Cam_ID_max][Cam_Set_max];

WCHAR*  cam_detail_setting[]  = {
	L"",
	L"Range",
	L"Heigh",
	L"Zoom.near",
	L"Zoom.far",  //distance
	L"Camera.angle",
	L"Angle.modulator"};


WCHAR* cam_type[] = {
	L"", //df
	L"Close", //0
	L"Mid Range", //1
	L"Long", //2
	L"Wide", //3
	L"Pitch Side",//4
	L"Blimp",//5
	L"Broadcasting wide",//6
	L"Broadcasting long",//7
	L"Live Broadcasting", //8
	L"Broadcasting close",//9
	L"Hidden camera 4",//A
	L"Hidden camera 5",//B
	L"Hidden camera 6",//C
	L"Vertical Wide",//D
	L"Player",//E
	L"Hidden camera 7",//F
	L"Front",//10
	L"Vertical Far"//0x11
	};//E