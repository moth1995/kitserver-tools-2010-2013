#ifndef _LODCFG_H_
#define _LODCFG_H_

#ifdef MYDLL_RELEASE_BUILD
#define LODCFG_WINDOW_TITLE "Kitserver 2011 Configuration"
#else
#define LODCFG_WINDOW_TITLE "Kitserver 2011 Configuration (debug build)"
#endif
#define CREDITS "Program info: v10.1.0 (12/2010) by Juce."

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

