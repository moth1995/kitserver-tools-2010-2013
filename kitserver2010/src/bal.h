// BAL (Be-A-Legend)
// save file structure and utils


typedef struct _BAL {
    union {
        struct {
            BYTE unknown1[0x1b86d0];
            DWORD unknown2[3];
            PLAYER_INFO player;
        } bal1;
        struct {
            BYTE unknown1[0x1b8754];
            PLAYER_DETAILS playerDetails;
        } bal2;
        struct {
            BYTE unknown1[0x1c2f04];
            PLAYER_INFO player;
        } bal3;
        struct {
            BYTE unknown1[0x800190];
            BYTE unknown2[0x0e];
            BYTE faceBit;
            BYTE hairBit;
        } bal4;
    };

} BAL;

