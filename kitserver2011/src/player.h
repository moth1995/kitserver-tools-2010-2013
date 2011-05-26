// player.h

#ifndef _PLAYER_H
#define _PLAYER_H

#define MAX_PLAYERS 0x27b0

#define SPECIAL_FACE 0x20
#define SPECIAL_HAIR 0x40

typedef struct _PLAYER_INFO
{
    DWORD id;
    BYTE unknown1[0x2e];
    char name[0x2e];
    char name2[0x10];
    DWORD id_again;
    BYTE unknown2[0x30];
    BYTE unknown3[3];
    BYTE specialHair;
    DWORD faceHairBits;
    BYTE unknown4[0x2c];
    WORD nationality;
    BYTE specialFace;
    BYTE unknown5;
    DWORD unknown6;
    DWORD unknown7;

} PLAYER_INFO;

typedef struct _PLAYER_DETAILS
{
    BYTE unknown3[3];
    BYTE specialHair;
    DWORD faceHairBits;
    BYTE unknown4[0x2c];
    WORD nationality;
    BYTE specialFace;
    BYTE unknown5;
    DWORD unknown6;
    DWORD unknown7;

} PLAYER_DETAILS; 

/*****************************************
 * Face/Hair bits:
 *
 * 00-10 : (11) hair id (bin-number - 3705 + 1)
 * 11-14 :  (4) hair flags
 * 15-25 : (11) face id (bin-number - 131 + 1)
 * 26-29 :  (4) face flags
 * 30-31 :  (2) unknown
 *
 */

int GetHairId(PLAYER_INFO* p);
int GetFaceId(PLAYER_INFO* p);

bool IsSpecialHair(PLAYER_INFO* p);
bool IsSpecialFace(PLAYER_INFO* p);
void SetSpecialHair(PLAYER_INFO* p, int hairId);
void SetSpecialFace(PLAYER_INFO* p, int faceId);

const DWORD FIRST_HAIR_BIN = 3705;
const DWORD FIRST_FACE_BIN = 131;

//#define CLEAR_HAIR_MASK  0xfffff800
//#define CLEAR_HAIR_FLAGS 0xffff87ff
//#define CLEAR_FACE_MASK  0xffc07fff
//#define CLEAR_FACE_FLAGS 0xc3ffffff
#define CLEAR_HAIR_MASK  0xffff8000
#define CLEAR_FACE_MASK  0xc0007fff
#define CLEAR_EXTENDED_BITS 0xc3ff87ff
#define CLEAR_EXTENDED_FACE_BITS 0xc3ffffff
#define CLEAR_EXTENDED_HAIR_BITS 0xffff87ff

#endif
