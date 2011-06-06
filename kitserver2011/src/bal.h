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
    };

} BAL;

