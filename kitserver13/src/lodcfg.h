#ifndef _LODCFG_H_
#define _LODCFG_H_
#define MYDLL_RELEASE_BUILD 1
#ifdef MYDLL_RELEASE_BUILD
#define LODCFG_WINDOW_TITLE "Kitserver 13 Configuration"
#else
#define LODCFG_WINDOW_TITLE "Kitserver 13 Configuration (debug build)"
#endif
#define CREDITS "Program info: v13.0.0 (Aug 2012) by Juce and Jenkey1002."

typedef struct _LCM {
    WORD homeTeamId;
    WORD awayTeamId;
    BYTE stadium;
    BYTE unknown1;
    BYTE timeOfDay;
    BYTE weather;
    BYTE season;
    BYTE effects;
    BYTE unknown2[6];
    BYTE unknown3[2];
    BYTE crowdStance;
    BYTE numSubs;
    BYTE homeCrowd;
    BYTE awayCrowd;
    BYTE unknown4[10];
} LCM;

#endif

