// manage.h
#ifndef _MANAGE_
#define _MANAGE_

typedef struct _KMOD {
        DWORD id;
        wchar_t nameLong[BUFLEN];
        wchar_t nameShort[BUFLEN];
        DWORD debug;
} KMOD;

typedef struct _PESINFO {
        wchar_t myDir[BUFLEN];
        wchar_t processFile[BUFLEN];
        wchar_t shortProcessFile[BUFLEN];
        wchar_t shortProcessFileNoExt[BUFLEN];
        wchar_t pesDir[BUFLEN];
        wchar_t gdbDir[BUFLEN];
        wchar_t logName[BUFLEN];
        int gameVersion;
        int realGameVersion;
        wchar_t lang[32];
        HANDLE hProc;
        UINT bbWidth;
        UINT bbHeight;
        double stretchX;
        double stretchY;
} PESINFO;

enum {
    gvPES2008demo,                // PES2008 PC DEMO
    gvPES2008demoSet,        // PES2008 PC DEMO (Settings)
    gvPES2008,                    // PES2008 PC
    gvPES2008Set,            // PES2008 PC (Settings)
    gvPES2008fltNODVD,  // PES2008 PC FLT-NODVD
    gvPES2008v110,      // PES2008 PC 1.10
    gvPES2008v110NODVD, // PES2008 PC 1.10 NODVD
    gvPES2008v120,      // PES2008 PC 1.20
    gvPES2009demo,      // PES2009 PC DEMO
    gvPES2009,          // PES2009 PC
    gvPES2009v110,      // PES2009 PC 1.10
    gvPES2009v120,      // PES2009 PC 1.20
    gvPES2009demoSet,   // PES2009 PC DEMO (Settings)
    gvPES2009Set,       // PES2009 PC (Settings)
    gvPES2009v130,      // PES2009 PC 1.30
    gvPES2009v140,      // PES2009 PC 1.40
    gvPES2010demo,      // PES2010 PC DEMO
    gvPES2010,          // PES2010 PC
    gvPES2010v11,       // PES2010 PC 1.1
    gvPES2010demoSet,   // PES2010 PC DEMO (Settings)
    gvPES2010Set,       // PES2010 PC (Settings)
    gvPES2010v12,       // PES2010 PC 1.2
    gvPES2010v13,       // PES2010 PC 1.3
};

#endif
