// replay.h

#ifndef _REPLAY_H
#define _REPLAY_H

typedef struct _REPLAY_PLAYER_INFO
{
    WORD unknown1;
    BYTE unknown2;
    BYTE specialFace;
    DWORD faceHairBits;
    BYTE unknown3[0x3b];
    char name[0x2e];
    char nameOnShirt[0x10];
    BYTE unknown4[3];
} REPLAY_PLAYER_INFO;

typedef struct _REPLAY_DATA_PAYLOAD
{
    BYTE miscInfo[0xec];
    REPLAY_PLAYER_INFO players[22];
} REPLAY_DATA_PAYLOAD;

typedef struct _REPLAY_DATA
{
    BYTE header[0x160];
    BYTE ksSignature[4];
    BYTE padding[0x1c];
    DWORD fileType;
    DWORD size;
    DWORD checksum;
    DWORD unknown;
    DWORD unknown1[4];
    REPLAY_DATA_PAYLOAD payload;
} REPLAY_DATA;

#endif
