#define UNICODE

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <math.h>

extern const wchar_t* _getTransl(char* section, char* key);
#define lang(s) _getTransl("setup",s)

#define BUFLEN 4096

#include "lodcfgui.h"

HWND g_lodListControl[5];          // lod lists
HWND g_crowdCheckBox;             // crowd
HWND g_JapanCheckBox;             // Japan check
HWND g_arCheckBox;            // aspect ratio correction check
HWND g_controllerCheckBox;            // controller check
HWND g_lodCheckBox;            // lod check
HWND g_timeCheckBox;            // match time

HWND g_weatherListControl;         // weather (default,fine,rainy,random)
HWND g_timeListControl;            // time of the day (default,day,night)
HWND g_seasonListControl;          // season (default,summer,winter)
HWND g_stadiumEffectsListControl;  // stadium effects (default,0/1)
HWND g_numberOfSubs;               // number of substitutions allowed
HWND g_homeCrowdListControl;       // home crowd attendance (default,0-3)
HWND g_awayCrowdListControl;       // away crowd attendance (default,0-3)
HWND g_crowdStanceListControl;      // crowd stance (default,1-3)

HWND g_resCheckBox;
HWND g_resWidthControl;
HWND g_resHeightControl;
HWND g_arRadio1;
HWND g_arRadio2;
HWND g_arEditControl;
HWND g_angleControl;
HWND g_timeControl;
HWND g_defLodControl;
HWND g_lodCheckBox1;
HWND g_lodCheckBox2;
HWND g_lodCheckBox3;
HWND g_lodCheckBox4;
HWND g_lodCheckBox5;
HWND g_lodCheckBox6;
HWND g_lodCheckBox7;
HWND g_lodCheckBox8;
HWND g_lodLabel1;
HWND g_lodLabel2;
HWND g_lodLabel3;
HWND g_lodLabel4;
HWND g_lodLabel5;
HWND g_lodLabel6;
HWND g_lodLabel7;
HWND g_lodLabel8;
HWND g_lodLabel9;
HWND g_lodLabel10;
HWND g_lodLabel11;
HWND g_lodLabel12;
HWND g_lodLabel13;
HWND g_lodLabel14;
HWND g_lodLabel15;
HWND g_lodLabel16;
HWND g_lodLabel17;
HWND g_lodLabel18;
HWND g_lodLabel19;
HWND g_lodLabel20;
HWND g_lodTrackBarControl[20];
HWND g_lodEditControl[20];
HWND g_speedCheckBox;
HWND g_speedTrackBarControl;
HWND g_speedEditControl;

HWND g_statusTextControl;          // displays status messages
HWND g_saveButtonControl;        // save settings button
HWND g_defButtonControl;          // default settings button

// A - curve steepness: 50 is less steeper than 100, etc.
#define A 32  

float f(float x)
{
    // defined on the range: [0,100]
    return 100.0*(0.001*(x/100.0-1) + exp(A*(x/100.0-1)) + 0.0011);
}

float getSwitchValue(int tickValue)
{
    return f((float)(100.0-tickValue));
}

float getMinSwitchValue()
{
    return f(0.0);
}

float getMaxSwitchValue()
{
    return f(100.0);
}

// B - curve steepness: 50 is less steeper than 100, etc.
#define B 32  

float f2(float x)
{
    // defined on the range: [0,100]
    return 1.3*(0.35*(x/100.0-1) + exp(B*(x/100.0-1))) + 1.2;
}

float getSwitchValue2(int tickValue)
{
    return f2((float)(tickValue));
}

float getMinSwitchValue2()
{
    return f2(0.0);
}

float getMaxSwitchValue2()
{
    return f2(100.0);
}

float round(float val)
{
    float lo = floorf(val);
    float hi = ceilf(val);
    return (fabs(val-lo) < fabs(val-hi)) ? lo : hi;
}

float inv_f(float y)
{
    // inverse function (binary search basically)
    static float PRECISION = 0.0005;
    float x, z, a = 0.0, b = 100.0;
    while (true)
    {
        x = 0.5*(a+b);
        z = f(x);
        if (abs(y-z) < PRECISION)
            return round(x);
        if (z>y) b=x;
        else if (z<y) a=x;
    }
}

int getTickValue(float switchValue)
{
    return 100.01-(int)inv_f(switchValue);
}

float inv_f2(float y)
{
    // inverse function (binary search basically)
    static float PRECISION = 0.0005;
    float x, z, a = 0.0, b = 100.0;
    while (true)
    {
        x = 0.5*(a+b);
        z = f2(x);
        if (abs(y-z) < PRECISION)
            return round(x);
        if (z>y) b=x;
        else if (z<y) a=x;
    }
}

int getTickValue2(float switchValue)
{
    return (int)inv_f2(switchValue);
}

// CreateTrackbar - creates and initializes a trackbar. 
HWND WINAPI CreateTrackbar( 
    HWND hwndDlg,  // handle of dialog box (parent window) 
    UINT x, UINT y,
    UINT w, UINT h,
    UINT iMin,     // minimum value in trackbar range 
    UINT iMax,     // maximum value in trackbar range 
    UINT iSelMin,  // minimum value in trackbar selection 
    UINT iSelMax)  // maximum value in trackbar selection 
{ 

    InitCommonControls(); // loads common control's DLL 

    HWND hwndTrack = CreateWindowEx( 
        0,                             // no extended styles 
        TRACKBAR_CLASS,                // class name 
        L"Trackbar Control",            // title (caption) 
        WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | /*TBS_ENABLESELRANGE |*/ TBS_HORZ | TBS_TOP | TBS_FIXEDLENGTH, 
        //TBS_AUTOTICKS | TBS_ENABLESELRANGE,  // style 
        x, y,                        // position 
        w, h,                       // size 
        hwndDlg,                       // parent window 
        NULL,//ID_TRACKBAR,             // control identifier 
        NULL,                       // instance 
        NULL                           // no WM_CREATE parameter 
        ); 

    SendMessage(hwndTrack, TBM_SETRANGE, 
        (WPARAM) TRUE,                   // redraw flag 
        (LPARAM) MAKELONG(iMin, iMax));  // min. & max. positions 
    SendMessage(hwndTrack, TBM_SETPAGESIZE, 
        0, (LPARAM) 4);                  // new page size 

    SendMessage(hwndTrack, TBM_SETSEL, 
        (WPARAM) FALSE,                  // redraw flag 
        (LPARAM) MAKELONG(iSelMin, iSelMax)); 
    SendMessage(hwndTrack, TBM_SETPOS, 
        (WPARAM) TRUE,                   // redraw flag 
        (LPARAM) iSelMin); 

    SendMessage(hwndTrack, TBM_SETTHUMBLENGTH, (WPARAM)25, (LPARAM)0);
    //SetFocus(hwndTrack); 

    return hwndTrack; 
} 


/**
 * build all controls
 */
bool BuildControls(HWND parent)
{
	HGDIOBJ hObj;
	DWORD style, xstyle;
	int x, y, spacer;
	int boxW, boxH, statW, statH, borW, borH, butW, butH, editW, editH;
    int topY;

	// Get a handle to the stock font object
	hObj = GetStockObject(DEFAULT_GUI_FONT);

	spacer = 6; 
	x = spacer, y = spacer;
	butH = 22;
	butW = 60;

	// use default extended style
	xstyle = WS_EX_LEFT;
	style = WS_CHILD | WS_VISIBLE;

	// TOP SECTION: Aspect ratio

	borW = WIN_WIDTH - spacer*3;
	statW = 40;
	boxW = borW - statW - spacer*3; boxH = 22; statH = 16;
	borH = spacer*3 + boxH*2;

	statW = borW - spacer*4;

    int i;
	style = WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX;

    x = spacer;
	HWND staticCrowdBorderTopControl = CreateWindowEx(
			xstyle, L"Static", L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDFRAME,
			x, y, borW, butH*2+spacer*3,
			parent, NULL, NULL, NULL);

    y += spacer;
    x = spacer*2;
	g_arCheckBox = CreateWindowEx(
			xstyle, L"button", L"Enable aspect ratio correction", style,
			x, y, 250, butH,
			parent, NULL, NULL, NULL);

    editW = 66;
    editH = statH + 6;
    x += spacer*2 + 250;

	style = WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON;
    butW = 90;
	g_arRadio1 = CreateWindowEx(
			xstyle, L"button", L"Automatic", style,
			x, y, butW, butH,
			parent, NULL, NULL, NULL);
    SendMessage(g_arRadio1, WM_SETFONT, (WPARAM)hObj, true);
    //EnableWindow(g_arRadio1, FALSE);

    y += spacer + butH;

    butW = 78;
	g_arRadio2 = CreateWindowEx(
			xstyle, L"button", L"Manual", style,
			x, y, butW, butH,
			parent, NULL, NULL, NULL);
    SendMessage(g_arRadio2, WM_SETFONT, (WPARAM)hObj, true);
    SendMessage(g_arRadio2, BM_SETCHECK, BST_CHECKED, 0);
    //EnableWindow(g_arRadio2, FALSE);
   
    x += spacer*2 + butW;
	style = WS_CHILD | WS_VISIBLE | ES_LEFT;
	g_arEditControl = CreateWindowEx(
			WS_EX_CLIENTEDGE, L"edit", L"", style,
			x, y-2, editW, editH,
			parent, NULL, NULL, NULL);
    SendMessage(g_arEditControl, WM_SETFONT, (WPARAM)hObj, true);
    EnableWindow(g_arEditControl, FALSE);

    ////////////// Custom resolution ////////////////////////////////////

    style = WS_CHILD | WS_VISIBLE;
    y += spacer*3 + butH;

    x = spacer;
	HWND staticWeatherBorderTopControl = CreateWindowEx(
			xstyle, L"Static", L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDFRAME,
			x, y, borW, boxH+spacer*2,
			parent, NULL, NULL, NULL);

    x = spacer*2;
    y += spacer;
	g_resCheckBox = CreateWindowEx(
			xstyle, L"button", L"Enforce custom resolution", style | BS_AUTOCHECKBOX,
			x, y, 250, butH,
			parent, NULL, NULL, NULL);
    SendMessage(g_resCheckBox, WM_SETFONT, (WPARAM)hObj, true);

    editW = 66;
    editH = statH + 6;

    x += spacer*2 + 250;

	style = WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER;
	g_resWidthControl = CreateWindowEx(
			WS_EX_CLIENTEDGE, L"edit", L"", style | WS_TABSTOP,
			x, y, editW, editH,
			parent, NULL, NULL, NULL);
    SendMessage(g_resWidthControl, WM_SETFONT, (WPARAM)hObj, true);
    EnableWindow(g_resWidthControl, FALSE);

    statW = 12;
    x += editW + spacer;
    HWND heightLabel = CreateWindowEx(
            xstyle, L"Static", L" x ", style,
            x, y+2, statW, statH, 
            parent, NULL, NULL, NULL);
    SendMessage(heightLabel, WM_SETFONT, (WPARAM)hObj, true);

    x += statW + spacer;
	style = WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER;
	g_resHeightControl = CreateWindowEx(
			WS_EX_CLIENTEDGE, L"edit", L"", style | WS_TABSTOP,
			x, y, editW, editH,
			parent, NULL, NULL, NULL);
    SendMessage(g_resHeightControl, WM_SETFONT, (WPARAM)hObj, true);
    EnableWindow(g_resHeightControl, FALSE);

	x = spacer*2;
	y += boxH + spacer*3;

    /*
    // CAMERA ANGLE
    x = spacer;
	HWND staticCameraBorderTopControl = CreateWindowEx(
			xstyle, L"Static", L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDFRAME,
			x, y, borW, boxH+spacer*2,
			parent, NULL, NULL, NULL);

    y += spacer;
    x = spacer*2;
    statW = 250;
	HWND angleLabel = CreateWindowEx(
			xstyle, L"Static", L"Camera angle (values > 9 are OK):", style,
			x, y+4, statW, statH, 
			parent, NULL, NULL, NULL);

	x += statW + spacer*2;
    boxW = 70;
	//butW = borW - statW - spacer*5 - boxW;

    g_angleControl = CreateWindowEx(
            xstyle, L"ComboBox", L"", 
			CBS_DROPDOWN | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP,
            x, y, boxW, boxH * 10,
            parent, NULL, NULL, NULL);

    x += boxW + spacer*2;
	HWND cameraText = CreateWindowEx(
			xstyle, L"Static", L"(default is 0)", style,
			x, y+4, statW, statH, 
			parent, NULL, NULL, NULL);

    SendMessage(g_angleControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)"0");
    SendMessage(g_angleControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)"1");
    SendMessage(g_angleControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)"2");
    SendMessage(g_angleControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)"3");
    SendMessage(g_angleControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)"4");
    SendMessage(g_angleControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)"5");
    SendMessage(g_angleControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)"6");
    SendMessage(g_angleControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)"7");
    SendMessage(g_angleControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)"8");
    SendMessage(g_angleControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)"9");

    x = spacer * 2;
    y += butH + spacer*3;
    */

    // LOD
    /////////////////////////////////////////////////////////////
    
	style = WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX;

    statW = 15;
    editW = 40;
    editH = statH;
    UINT twa = borW/2 - spacer*2;
    UINT tw = borW/2 - spacer*6 - statW - editW;
    UINT th = 28;
    borH = th*11 + spacer*15 + butH + statH*3 + spacer*3;

    x = spacer;
	HWND staticBorderTopControl3 = CreateWindowEx(
			xstyle, L"Static", L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDFRAME,
			x, y, borW, borH,
			parent, NULL, NULL, NULL);


    y += spacer;
    x = spacer*2;
	HWND lodTitle = CreateWindowEx(
			xstyle, L"Static", L"Level-Of-Detail (LOD) Configuration", WS_CHILD | WS_VISIBLE,
			x, y, 260, butH,
			parent, NULL, NULL, NULL);

	y += boxH + spacer;
    x = spacer*2;

    topY = y;

    // Players LOD: inplay
    
	g_lodCheckBox1 = CreateWindowEx(
			xstyle, L"button", L"PLAYERS (Gameplay)", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			x, y+4, twa, statH,
			parent, NULL, NULL, NULL);
    y += statH + spacer;

	g_lodLabel1 = CreateWindowEx(
			xstyle, L"Static", L"S1", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[0] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(100.0),getTickValue(100.0));
    x += tw + spacer;
	g_lodEditControl[0] = CreateWindowEx(
			xstyle, L"static", L"100.0", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    x = spacer*2;
	g_lodLabel2 = CreateWindowEx(
			xstyle, L"Static", L"S2", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[1] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(100.0),getTickValue(100.0));
    x += tw + spacer;
	g_lodEditControl[1] = CreateWindowEx(
			xstyle, L"static", L"100.0", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    x = spacer*2;
	g_lodLabel3 = CreateWindowEx(
			xstyle, L"Static", L"S3", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[2] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(0.100),getTickValue(0.100));
    x += tw + spacer;
	g_lodEditControl[2] = CreateWindowEx(
			xstyle, L"static", L"0.100", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    // Players LOD: replay
    y = topY;
    x = spacer*4 + twa;

	g_lodCheckBox2 = CreateWindowEx(
			xstyle, L"button", L"PLAYERS (Replay)", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			x, y+4, twa, statH,
			parent, NULL, NULL, NULL);
    y += statH + spacer;

	g_lodLabel4 = CreateWindowEx(
			xstyle, L"Static", L"S1", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[3] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(0.090),getTickValue(0.090));
    x += tw + spacer;
	g_lodEditControl[3] = CreateWindowEx(
			xstyle, L"static", L"0.090", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    x = spacer*4 + twa;
	g_lodLabel5 = CreateWindowEx(
			xstyle, L"Static", L"S2", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[4] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(0.070),getTickValue(0.070));
    x += tw + spacer;
	g_lodEditControl[4] = CreateWindowEx(
			xstyle, L"static", L"0.070", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    x = spacer*4 + twa;
	g_lodLabel6 = CreateWindowEx(
			xstyle, L"Static", L"S3", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[5] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(0.055),getTickValue(0.055));
    x += tw + spacer;
	g_lodEditControl[5] = CreateWindowEx(
			xstyle, L"static", L"0.055", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    y += spacer;
    topY = y;

    // Players LOD: entrance
    x = spacer*2;
    
	g_lodCheckBox3 = CreateWindowEx(
			xstyle, L"button", L"PLAYERS (Entrance Scene)", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			x, y+4, twa, statH,
			parent, NULL, NULL, NULL);
    y += statH + spacer;

	g_lodLabel7 = CreateWindowEx(
			xstyle, L"Static", L"S1", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[6] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(100.0),getTickValue(100.0));
    x += tw + spacer;
	g_lodEditControl[6] = CreateWindowEx(
			xstyle, L"static", L"100.0", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    x = spacer*2;
	g_lodLabel8 = CreateWindowEx(
			xstyle, L"Static", L"S2", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[7] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(0.150),getTickValue(0.150));
    x += tw + spacer;
	g_lodEditControl[7] = CreateWindowEx(
			xstyle, L"static", L"0.150", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    x = spacer*2;
	g_lodLabel9 = CreateWindowEx(
			xstyle, L"Static", L"S3", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[8] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(0.100),getTickValue(0.100));
    x += tw + spacer;
	g_lodEditControl[8] = CreateWindowEx(
			xstyle, L"static", L"0.100", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    // Players LOD: misc
    y = topY;
    x = spacer*4 + twa;
    
	g_lodCheckBox4 = CreateWindowEx(
			xstyle, L"button", L"PLAYERS (Cut-scenes, Highlights, etc.)", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			x, y+4, twa, statH,
			parent, NULL, NULL, NULL);
    y += statH + spacer;

	g_lodLabel10 = CreateWindowEx(
			xstyle, L"Static", L"S1", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[9] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(0.430),getTickValue(0.430));
    x += tw + spacer;
	g_lodEditControl[9] = CreateWindowEx(
			xstyle, L"static", L"0.430", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    x = spacer*4 + twa;
	g_lodLabel11 = CreateWindowEx(
			xstyle, L"Static", L"S2", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[10] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(0.150),getTickValue(0.150));
    x += tw + spacer;
	g_lodEditControl[10] = CreateWindowEx(
			xstyle, L"static", L"0.150", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    x = spacer*4 + twa;
	g_lodLabel12 = CreateWindowEx(
			xstyle, L"Static", L"S3", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[11] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(0.100),getTickValue(0.100));
    x += tw + spacer;
	g_lodEditControl[11] = CreateWindowEx(
			xstyle, L"static", L"0.100", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;
    y += spacer;

    // Players LOD: active.player.ck
    topY = y;
    x = spacer*2;
    
	g_lodCheckBox7 = CreateWindowEx(
			xstyle, L"button", L"PLAYERS (Active Player, CK, Edit Mode)", 
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			x, y+4, twa, statH,
			parent, NULL, NULL, NULL);
    y += statH + spacer;

	g_lodLabel15 = CreateWindowEx(
			xstyle, L"Static", L"S1", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[14] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(0.010),getTickValue(0.010));
    x += tw + spacer;
	g_lodEditControl[14] = CreateWindowEx(
			xstyle, L"static", L"0.010", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    x = spacer*2;
	g_lodLabel16 = CreateWindowEx(
			xstyle, L"Static", L"S2", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[15] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(0.010),getTickValue(0.010));
    x += tw + spacer;
	g_lodEditControl[15] = CreateWindowEx(
			xstyle, L"static", L"0.010", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    x = spacer*2;
	g_lodLabel17 = CreateWindowEx(
			xstyle, L"Static", L"S3", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[16] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(0.010),getTickValue(0.010));
    x += tw + spacer;
	g_lodEditControl[16] = CreateWindowEx(
			xstyle, L"static", L"0.010", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    // Players LOD: active.player.fk
    y = topY;
    x = spacer*4 + twa;
    
	g_lodCheckBox8 = CreateWindowEx(
			xstyle, L"button", L"PLAYERS (Active Player, FK)", 
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			x, y+4, twa, statH,
			parent, NULL, NULL, NULL);
    y += statH + spacer;

	g_lodLabel18 = CreateWindowEx(
			xstyle, L"Static", L"S1", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[17] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(100.0),getTickValue(100.0));
    x += tw + spacer;
	g_lodEditControl[17] = CreateWindowEx(
			xstyle, L"static", L"100.0", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    x = spacer*4 + twa;
	g_lodLabel19 = CreateWindowEx(
			xstyle, L"Static", L"S2", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[18] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(0.010),getTickValue(0.010));
    x += tw + spacer;
	g_lodEditControl[18] = CreateWindowEx(
			xstyle, L"static", L"0.010", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    x = spacer*4 + twa;
	g_lodLabel20 = CreateWindowEx(
			xstyle, L"Static", L"S3", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[19] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(0.010),getTickValue(0.010));
    x += tw + spacer;
	g_lodEditControl[19] = CreateWindowEx(
			xstyle, L"static", L"0.010", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;
    y += spacer;

    // Ref LOD: gameplay
    topY = y;
    x = spacer*2;
    
	g_lodCheckBox5 = CreateWindowEx(
			xstyle, L"button", L"REFEREES (Gameplay)", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			x, y+4, twa, statH,
			parent, NULL, NULL, NULL);
    y += statH + spacer;

	g_lodLabel13 = CreateWindowEx(
			xstyle, L"Static", L"S1", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[12] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(0.100),getTickValue(0.100));
    x += tw + spacer;
	g_lodEditControl[12] = CreateWindowEx(
			xstyle, L"static", L"0.100", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    // Players LOD: misc
    y = topY;
    x = spacer*4 + twa;
    
	g_lodCheckBox6 = CreateWindowEx(
			xstyle, L"button", L"REFEREES (Replay)", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			x, y+4, twa, statH,
			parent, NULL, NULL, NULL);
    y += statH + spacer;

	g_lodLabel14 = CreateWindowEx(
			xstyle, L"Static", L"S1", WS_CHILD | WS_VISIBLE,
			x, y+4, statW, statH,
			parent, NULL, NULL, NULL);

    x += statW + spacer;
    g_lodTrackBarControl[13] = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue(0.100),getTickValue(0.100));
    x += tw + spacer;
	g_lodEditControl[13] = CreateWindowEx(
			xstyle, L"static", L"0.100", WS_CHILD | WS_VISIBLE,
			x, y+4, editW, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    for (i=0; i<20; i++)
        SendMessage(g_lodTrackBarControl[i], 
                TBM_SETTICFREQ, (WPARAM)5, 0);


    // controller check
    y += spacer*3;
    topY = y;
    statW = 250;
    statW = statW/2 - spacer;
    borW = borW/2 - spacer;
    x = spacer;
	HWND staticBorderTopControl4 = CreateWindowEx(
			xstyle, L"Static", L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDFRAME,
			x, y, borW, boxH+spacer*2,
			parent, NULL, NULL, NULL);

    y += spacer;
    x = spacer*2;
	g_controllerCheckBox = CreateWindowEx(
			xstyle, L"button", L"Free sides selection", 
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			x, y, statW*2-spacer, butH,
			parent, NULL, NULL, NULL);

	y += boxH + spacer*2;
    x = spacer*2;

    // time control
    x = spacer;
	HWND staticTimeBorderTopControl = CreateWindowEx(
			xstyle, L"Static", L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDFRAME,
			x, y, borW, boxH+spacer*2,
			parent, NULL, NULL, NULL);

    y += spacer;
    x = spacer*2;
	g_timeCheckBox = CreateWindowEx(
			xstyle, L"button", L"Force match time:", 
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			x, y, statW, butH,
			parent, NULL, NULL, NULL);

	x += statW + spacer*2;
    boxW = 70;

    g_timeControl = CreateWindowEx(
            xstyle, L"ComboBox", L"", 
			CBS_DROPDOWN | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP,
            x, y, boxW, boxH * 9,
            parent, NULL, NULL, NULL);

    SendMessage(g_timeControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"1");
    SendMessage(g_timeControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"5");
    SendMessage(g_timeControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"10");
    SendMessage(g_timeControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"15");
    SendMessage(g_timeControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"20");
    SendMessage(g_timeControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"25");
    SendMessage(g_timeControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"30");
    SendMessage(g_timeControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"66");
    SendMessage(g_timeControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"90");

    EnableWindow(g_timeCheckBox, FALSE);
    EnableWindow(g_timeControl, FALSE);

    x = spacer * 2;
    y += butH + spacer*3;

    // Game speed
    x = spacer*3 + borW;
    y = topY;

   	HWND staticBorderTopControl5 = CreateWindowEx(
			xstyle, L"Static", L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDFRAME,
			x, y, borW, boxH*2+spacer*5,
			parent, NULL, NULL, NULL);

    y += spacer;
    x += spacer;
 
	g_speedCheckBox = CreateWindowEx(
			xstyle, L"button", L"Game Speed (SLOWER < 1.0 < FASTER)", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			x, y+4, statW*2-spacer, statH,
			parent, NULL, NULL, NULL);
    y += statH + spacer;

    y += spacer;
    tw = borW - editW - spacer*2;
    g_speedTrackBarControl = CreateTrackbar(parent,x,y,tw,th,0,100,
            getTickValue2(1.0),getTickValue2(1.0));
    x += tw + spacer;
	g_speedEditControl = CreateWindowEx(
			xstyle, L"static", L"1.00", WS_CHILD | WS_VISIBLE,
			x, y+4, editW-spacer, editH,
			parent, NULL, NULL, NULL);
    y += th + spacer;

    SendMessage(g_speedTrackBarControl, 
            TBM_SETTICFREQ, (WPARAM)2, 0);

	style = WS_CHILD | WS_VISIBLE;

    y += spacer*4;



	// BOTTOM sections: buttons
	
    borW = (borW + spacer)*2;
	x = borW - butW;
	g_saveButtonControl = CreateWindowEx(
			xstyle, L"Button", L"Save", style,
			x, y, butW + spacer, butH,
			parent, NULL, NULL, NULL);

    butW = 60;
	x -= butW + spacer;

	g_defButtonControl = CreateWindowEx(
			xstyle, L"Button", L"Help", style,
			x - spacer, y, butW + spacer, butH,
			parent, NULL, NULL, NULL);

	x = spacer;
	statW = WIN_WIDTH - spacer*4 - 160;

	g_statusTextControl = CreateWindowEx(
			xstyle, L"Static", L"", style,
			x, y+6, statW, statH,
			parent, NULL, NULL, NULL);

    for (i=0; i<5; i++) {
        SendMessage(g_lodListControl[i], WM_SETFONT, (WPARAM)hObj, true);
		EnableWindow(g_lodListControl[i], FALSE);
    }
    EnableWindow(g_speedEditControl, FALSE);
    EnableWindow(g_crowdCheckBox, FALSE);
    SendMessage(g_weatherListControl, WM_SETFONT, (WPARAM)hObj, true);
    EnableWindow(g_weatherListControl, FALSE);

	SendMessage(g_statusTextControl, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_saveButtonControl, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_defButtonControl, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_crowdCheckBox, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_arCheckBox, WM_SETFONT, (WPARAM)hObj, true);
	//SendMessage(angleLabel, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_angleControl, WM_SETFONT, (WPARAM)hObj, true);
	//SendMessage(cameraText, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_timeCheckBox, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_timeControl, WM_SETFONT, (WPARAM)hObj, true);

	SendMessage(lodTitle, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_controllerCheckBox, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_defLodControl, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodCheckBox1, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodCheckBox2, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodCheckBox3, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodCheckBox4, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodCheckBox5, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodCheckBox6, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodCheckBox7, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodCheckBox8, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel1, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel2, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel3, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel4, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel5, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel6, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel7, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel8, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel9, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel10, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel11, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel12, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel13, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel14, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel15, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel16, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel17, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel18, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel19, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_lodLabel20, WM_SETFONT, (WPARAM)hObj, true);
    for (int x=0; x<20; x++)
        SendMessage(g_lodEditControl[x], WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_speedCheckBox, WM_SETFONT, (WPARAM)hObj, true);
    SendMessage(g_speedEditControl, WM_SETFONT, (WPARAM)hObj, true);

    //EnableWindow(g_lodLabel1, FALSE);
    //EnableWindow(g_lodLabel2, FALSE);
    for (i=0; i<20; i++) {
        EnableWindow(g_lodTrackBarControl[i], FALSE);
        EnableWindow(g_lodEditControl[i], TRUE);
    }

	//EnableWindow(g_saveButtonControl, FALSE);
	//EnableWindow(g_defButtonControl, FALSE);

	return true;
}

