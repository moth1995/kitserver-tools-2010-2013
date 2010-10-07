// lodmixer.cpp
#define UNICODE
#define THISMOD &k_lodmixer

#include <windows.h>
#include <stdio.h>
#include "kload_exp.h"
#include "lodmixer.h"
#include "lodmixer_addr.h"
#include "dllinit.h"

#define FLOAT_ZERO 0.0001f
#define SET(p,v) {if (v>FLOAT_ZERO) {*(p) = v;}}

KMOD k_lodmixer={MODID,NAMELONG,NAMESHORT,DEFAULT_DEBUG};

HINSTANCE hInst;
LMCONFIG _lmconfig = {
    {DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_ASPECT_RATIO}, 
    {
        DEFAULT_LOD_1,
        DEFAULT_LOD_2,
        DEFAULT_LOD_3,
        DEFAULT_LOD_4,
        DEFAULT_LOD_5,
        DEFAULT_LOD_6,
        DEFAULT_LOD_7,
        DEFAULT_LOD_8,
        DEFAULT_LOD_9,
        DEFAULT_LOD_10,
        DEFAULT_LOD_11,
        DEFAULT_LOD_12,
        DEFAULT_LOD_13,
        DEFAULT_LOD_14,
        DEFAULT_LOD_15,
        DEFAULT_LOD_16,
        DEFAULT_LOD_17,
        DEFAULT_LOD_18,
        DEFAULT_LOD_19,
        DEFAULT_LOD_20,
    },
    DEFAULT_ASPECT_RATIO_CORRECTION_ENABLED,
    DEFAULT_CONTROLLER_CHECK_ENABLED,
    DEFAULT_LODCHECK1,
    DEFAULT_VIDEO_CHECK_ENABLED,
};

EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);

void initLodMixer();
void lodmixerConfig(char* pName, const void* pValue, DWORD a);
void modifySettings();
void getResolution(DWORD& width, DWORD& height);
void setResolution(DWORD width, DWORD height);
void setAspectRatio(float aspectRatio, bool manual);
void lodAtModeCheckCallPoint();
void lodAtModeCheckCallPoint2010();
void lodAtSettingsReadPoint();
void lodAtSettingsResetPoint();
KEXPORT DWORD lodAtModeCheck(DWORD mode);

EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		hInst=hInstance;
		RegisterKModule(&k_lodmixer);

		if (!checkGameVersion()) {
			LOG(L"Sorry, your game version isn't supported!");
			return false;
		}

		copyAdresses();
		hookFunction(hk_D3D_Create, initLodMixer);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		LOG(L"Detaching dll...");
	}
	return true;
}

void modifySettings()
{
    setResolution(_lmconfig.screen.width, _lmconfig.screen.height);
    if (_lmconfig.aspectRatioCorrectionEnabled)
    {
        DWORD width = 0, height = 0;
        getResolution(width,height);
        float ar = (_lmconfig.screen.aspectRatio > FLOAT_ZERO) ?
            _lmconfig.screen.aspectRatio :  // manual
            float(width) / float(height);   // automatic

        setAspectRatio(ar, _lmconfig.screen.aspectRatio > FLOAT_ZERO);
    }
}

void initLodMixer()
{
    // skip the settings-check call
    LOG(L"Initializing LOD mixer...");

    getConfig("lodmixer", "screen.width", DT_DWORD, 1, lodmixerConfig);
    getConfig("lodmixer", "screen.height", DT_DWORD, 2, lodmixerConfig);
    getConfig("lodmixer", "screen.aspect-ratio", DT_FLOAT, 3, lodmixerConfig);
    getConfig("lodmixer", "aspect-ratio.correction.enabled", DT_DWORD, 6, lodmixerConfig);
    getConfig("lodmixer", "controller.check.disabled", DT_DWORD, 7, lodmixerConfig);
    getConfig("lodmixer", "lod.check1", DT_DWORD, 8, lodmixerConfig);
    LOG2N(L"Screen resolution to force: %dx%d", 
            _lmconfig.screen.width, _lmconfig.screen.height);

    getConfig("lodmixer", "lod.players.entrance.s1", DT_FLOAT, 9,lodmixerConfig);
    getConfig("lodmixer", "lod.players.entrance.s2", DT_FLOAT,10,lodmixerConfig);
    getConfig("lodmixer", "lod.players.entrance.s3", DT_FLOAT,21,lodmixerConfig);
    getConfig("lodmixer", "lod.players.inplay.s1", DT_FLOAT,11,lodmixerConfig);
    getConfig("lodmixer", "lod.players.inplay.s2", DT_FLOAT,12,lodmixerConfig);
    getConfig("lodmixer", "lod.players.inplay.s3", DT_FLOAT,13,lodmixerConfig);
    getConfig("lodmixer", "lod.players.misc.s1", DT_FLOAT, 14,lodmixerConfig);
    getConfig("lodmixer", "lod.players.misc.s2", DT_FLOAT, 15,lodmixerConfig);
    getConfig("lodmixer", "lod.players.misc.s3", DT_FLOAT, 22,lodmixerConfig);
    getConfig("lodmixer", "lod.players.replay.s1", DT_FLOAT, 16,lodmixerConfig);
    getConfig("lodmixer", "lod.players.replay.s2", DT_FLOAT, 17,lodmixerConfig);
    getConfig("lodmixer", "lod.players.replay.s3", DT_FLOAT, 18,lodmixerConfig);

    getConfig("lodmixer", "lod.ref.inplay", DT_FLOAT, 19,lodmixerConfig);
    getConfig("lodmixer", "lod.ref.replay", DT_FLOAT, 20,lodmixerConfig);

    getConfig("lodmixer", "lod.active.player.ck.s1", DT_FLOAT, 24,
            lodmixerConfig);
    getConfig("lodmixer", "lod.active.player.ck.s2", DT_FLOAT, 25,
            lodmixerConfig);
    getConfig("lodmixer", "lod.active.player.ck.s3", DT_FLOAT, 26,
            lodmixerConfig);
    getConfig("lodmixer", "lod.active.player.fk.s1", DT_FLOAT, 27,
            lodmixerConfig);
    getConfig("lodmixer", "lod.active.player.fk.s2", DT_FLOAT, 28,
            lodmixerConfig);
    getConfig("lodmixer", "lod.active.player.fk.s3", DT_FLOAT, 29,
            lodmixerConfig);

    getConfig("lodmixer", "video.check.enabled", DT_DWORD, 23, lodmixerConfig);

    BYTE* bptr = (BYTE*)code[C_SETTINGS_CHECK];
    DWORD protection;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;
    if (bptr)
    {
        if (VirtualProtect(bptr, 6, newProtection, &protection)) {
            /* CALL */
            bptr[0] = 0xe8;
            DWORD* ptr = (DWORD*)(code[C_SETTINGS_CHECK] + 1);
            ptr[0] = (DWORD)modifySettings - (DWORD)(code[C_SETTINGS_CHECK] + 5);
            /* NOP */ 
            bptr[5] = 0x90;
            LOG(L"Settings check disabled. Settings overwrite enabled.");
        } 
    }

    HookCallPoint(code[C_SETTINGS_READ], lodAtSettingsReadPoint, 6, 0, true);
    HookCallPoint(code[C_SETTINGS_RESET], lodAtSettingsResetPoint, 6, 0, true);
    if (!_lmconfig.videoCheckEnabled)
    {
        if (code[C_VIDEO_CHECK1]!=0)
        {
            bptr = (BYTE*)code[C_VIDEO_CHECK1];
            if (VirtualProtect(bptr, 6, newProtection, &protection)) {
                if (getPesInfo()->gameVersion >= gvPES2010demo)
                {
                    /* jmp */  memcpy(bptr,"\xe9\xd1\x00\x00\x00",5);
                    /* nop */  bptr[5] = 0x90;
                }
                else
                {
                    /* jmp */  memcpy(bptr,"\xe9\x5d\x01\x00\x00",5);
                    /* nop */  bptr[5] = 0x90;
                }
                LOG(L"video check 1 bypassed");
            }
        }
        if (code[C_VIDEO_CHECK2]!=0)
        {
            bptr = (BYTE*)code[C_VIDEO_CHECK2];
            if (VirtualProtect(bptr, 4, newProtection, &protection)) {
                if (getPesInfo()->gameVersion >= gvPES2010demo)
                {
                    /* jmp */  memcpy(bptr,"\xe9\xb0\x00\x00\x00",5);
                    /* nop */  bptr[5] = 0x90;
                }
                else
                {
                    /* jmp */  memcpy(bptr,"\xe9\xaf\x00\x00\x00",5);
                    /* nop */  bptr[5] = 0x90;
                }
                LOG(L"video check 2 bypassed");
            }
        }
    }

    if (_lmconfig.controllerCheckEnabled)
    {
        bptr = (BYTE*)code[C_MODE_CHECK];
        if (bptr && getPesInfo()->gameVersion < gvPES2009demo)
        {
            // need to insert a bit of code, to handle the special
            // case of Exhibition Mode. 61 bytes before C_MODE_CHECK is a good place.
            BYTE* codeInsert = bptr - 61;
            if (VirtualProtect(bptr, 4, newProtection, &protection)) {
                BYTE patch[] = {0xeb,0xc1}; // jmp short "TO_TEST_ECX"
                memcpy(bptr, patch, sizeof(patch));
                if (VirtualProtect(codeInsert, 12, newProtection, &protection)) {
                    BYTE patch2[] = {
                        0x85,0xc9,  // test ecx,ecx
                        0x74,0x03,  // je short "TO_XOR_EBX"
                        0x33,0xc9,  // xor ecx,ecx
                        0x41,       // inc ecx
                        0x33,0xdb,  // xor ebx,ebx  - the command that was replaced with jmp
                        0xeb,0x34,  // jmp short "BACK_TO_AFTER_JMP"
                    };
                    memcpy(codeInsert, patch2, sizeof(patch2));
                    LOG(L"Mode check disabled for controller selection.");
                }
            } 
        }
        else if (bptr && getPesInfo()->gameVersion < gvPES2010)
        {
            // PES2009: use callpoint
            HookCallPoint(code[C_MODE_CHECK], lodAtModeCheckCallPoint, 6, 1);
            LOG(L"Mode check disabled for controller selection.");
        }
        else if (bptr)
        {
            // PES2010: use callpoint
            HookCallPoint(code[C_MODE_CHECK], 
                    lodAtModeCheckCallPoint2010, 6, 1);
            LOG(L"Mode check disabled for controller selection.");
        }
    }

    if (!_lmconfig.lodCheck1)
    {
        bptr = (BYTE*)code[C_LODCHECK_1];
        if (bptr && VirtualProtect(bptr, 4, newProtection, &protection)) {
            bptr[0] = 0x90;  // nop
            bptr[1] = 0x90;  // nop
            LOG(L"LOD check: lowest lod level disabled.");
        }
    }

    if (getPesInfo()->gameVersion >= gvPES2010demo)
    {
        // LOD modifications
        float* fptr =(float*)data[LOD_PLAYERS_TABLE1];
        if (fptr)
        {
            SET(fptr+0, _lmconfig.lod.lodPlayersMiscS1);
            SET(fptr+1, _lmconfig.lod.lodPlayersMiscS2);
            SET(fptr+2, _lmconfig.lod.lodPlayersMiscS3);
            SET(fptr+3, _lmconfig.lod.lodPlayersReplayS1);
            SET(fptr+4, _lmconfig.lod.lodPlayersReplayS2);
            SET(fptr+5, _lmconfig.lod.lodPlayersReplayS3);
            // active player: corner kicks
            SET(fptr+6, _lmconfig.lod.lodActivePlayerCKs1); 
            SET(fptr+7, _lmconfig.lod.lodActivePlayerCKs2); 
            SET(fptr+8, _lmconfig.lod.lodActivePlayerCKs3); 
            // active player: free kicks
            SET(fptr+9, _lmconfig.lod.lodActivePlayerFKs1); 
            SET(fptr+10, _lmconfig.lod.lodActivePlayerFKs2); 
            SET(fptr+11, _lmconfig.lod.lodActivePlayerFKs3); 
        }

        fptr = (float*)data[LOD_PLAYERS_TABLE2];
        if (fptr)
        {
            SET(fptr+0, _lmconfig.lod.lodPlayersEntranceS1);
            SET(fptr+1, _lmconfig.lod.lodPlayersEntranceS2);
            SET(fptr+2, _lmconfig.lod.lodPlayersEntranceS3);
            SET(fptr+3, _lmconfig.lod.lodPlayersInplayS1);
            SET(fptr+4, _lmconfig.lod.lodPlayersInplayS2);
            SET(fptr+5, _lmconfig.lod.lodPlayersInplayS3);
        }

        fptr = (float*)data[LOD_REF_TABLE1];
        if (fptr) SET(fptr, _lmconfig.lod.lodRefReplay);
        fptr = (float*)data[LOD_REF_TABLE2];
        if (fptr) SET(fptr, _lmconfig.lod.lodRefInplay);

        LOG(L"LOD levels set");
    }

    LOG(L"Initialization complete.");
    unhookFunction(hk_D3D_Create, initLodMixer);
}

void lodmixerConfig(char* pName, const void* pValue, DWORD a)
{
	switch (a) {
		case 1:	// width
			_lmconfig.screen.width = *(DWORD*)pValue;
			break;
		case 2: // height
			_lmconfig.screen.height = *(DWORD*)pValue;
			break;
		case 3: // aspect ratio
			_lmconfig.screen.aspectRatio = *(float*)pValue;
			break;
		case 6: // LOD
			_lmconfig.aspectRatioCorrectionEnabled = *(DWORD*)pValue != 0;
			break;
		case 7: // Controller check
			_lmconfig.controllerCheckEnabled = *(DWORD*)pValue != 0;
			break;
        case 8: // lod-check
            _lmconfig.lodCheck1 = *(DWORD*)pValue != 0;
            break;

        case 9: // lod.players.entrance.s1
            _lmconfig.lod.lodPlayersEntranceS1 = *(float*)pValue;
            LOG1F(L"lod.players.entrance.s1 = %03f", 
                    _lmconfig.lod.lodPlayersEntranceS1);
            break;
        case 10: // lod.players.entrance.s2
            _lmconfig.lod.lodPlayersEntranceS2 = *(float*)pValue;
            LOG1F(L"lod.players.entrance.s2 = %03f", 
                    _lmconfig.lod.lodPlayersEntranceS2);
            break;
        case 11: // lod.players.inplay.s1
            _lmconfig.lod.lodPlayersInplayS1 = *(float*)pValue;
            LOG1F(L"lod.players.inplay.s1 = %03f", 
                    _lmconfig.lod.lodPlayersInplayS1);
            break;
        case 12: // lod.players.inplay.s2
            _lmconfig.lod.lodPlayersInplayS2 = *(float*)pValue;
            LOG1F(L"lod.players.inplay.s2 = %03f", 
                    _lmconfig.lod.lodPlayersInplayS2);
            break;
        case 13: // lod.players.inplay.s3
            _lmconfig.lod.lodPlayersInplayS3 = *(float*)pValue;
            LOG1F(L"lod.players.inplay.s3 = %03f", 
                    _lmconfig.lod.lodPlayersInplayS3);
            break;

        case 14: // lod.players.misc.s1
            _lmconfig.lod.lodPlayersMiscS1 = *(float*)pValue;
            LOG1F(L"lod.players.misc.s1 = %03f", 
                    _lmconfig.lod.lodPlayersMiscS1);
            break;
        case 15: // lod.players.misc.s2
            _lmconfig.lod.lodPlayersMiscS2 = *(float*)pValue;
            LOG1F(L"lod.players.misc.s2 = %03f", 
                    _lmconfig.lod.lodPlayersMiscS2);
            break;
        case 16: // lod.players.replay.s1
            _lmconfig.lod.lodPlayersReplayS1 = *(float*)pValue;
            LOG1F(L"lod.players.replay.s1 = %03f", 
                    _lmconfig.lod.lodPlayersReplayS1);
            break;
        case 17: // lod.players.other.s2
            _lmconfig.lod.lodPlayersReplayS2 = *(float*)pValue;
            LOG1F(L"lod.players.replay.s2 = %03f", 
                    _lmconfig.lod.lodPlayersReplayS2);
            break;
        case 18: // lod.players.replay.s3
            _lmconfig.lod.lodPlayersReplayS3 = *(float*)pValue;
            LOG1F(L"lod.players.replay.s3 = %03f", 
                    _lmconfig.lod.lodPlayersReplayS3);
            break;

        case 19: // lod.ref.inplay
            _lmconfig.lod.lodRefInplay = *(float*)pValue;
            LOG1F(L"lod.ref.inplay = %03f", 
                    _lmconfig.lod.lodRefInplay);
            break;
        case 20: // lod.ref.other
            _lmconfig.lod.lodRefReplay = *(float*)pValue;
            LOG1F(L"lod.ref.replay = %03f", 
                    _lmconfig.lod.lodRefReplay);
            break;
        case 21: // lod.players.entrance.s3
            _lmconfig.lod.lodPlayersEntranceS3 = *(float*)pValue;
            LOG1F(L"lod.players.entrance.s3 = %03f", 
                    _lmconfig.lod.lodPlayersEntranceS3);
            break;
        case 22: // lod.players.misc.s3
            _lmconfig.lod.lodPlayersMiscS3 = *(float*)pValue;
            LOG1F(L"lod.players.misc.s3 = %03f", 
                    _lmconfig.lod.lodPlayersMiscS3);
            break;
        case 23: // video.check.skipped
            _lmconfig.videoCheckEnabled = *(DWORD*)pValue != 0;
            LOG1N(L"video.check.enabled = %d", 
                    _lmconfig.videoCheckEnabled);
            break;
        case 24: // lod.active.player.ck.s1
            _lmconfig.lod.lodActivePlayerCKs1 = *(float*)pValue;
            LOG1F(L"lod.active.player.ck.s1 = %03f", 
                    _lmconfig.lod.lodActivePlayerCKs1);
            break;
        case 25: // lod.active.player.ck.s2
            _lmconfig.lod.lodActivePlayerCKs2 = *(float*)pValue;
            LOG1F(L"lod.active.player.ck.s2 = %03f", 
                    _lmconfig.lod.lodActivePlayerCKs2);
            break;
        case 26: // lod.active.player.ck.s3
            _lmconfig.lod.lodActivePlayerCKs3 = *(float*)pValue;
            LOG1F(L"lod.active.player.ck.s3 = %03f", 
                    _lmconfig.lod.lodActivePlayerCKs3);
            break;
        case 27: // lod.active.player.fk.s1
            _lmconfig.lod.lodActivePlayerFKs1 = *(float*)pValue;
            LOG1F(L"lod.active.player.fk.s1 = %03f", 
                    _lmconfig.lod.lodActivePlayerFKs1);
            break;
        case 28: // lod.active.player.fk.s2
            _lmconfig.lod.lodActivePlayerFKs2 = *(float*)pValue;
            LOG1F(L"lod.active.player.fk.s2 = %03f", 
                    _lmconfig.lod.lodActivePlayerFKs2);
            break;
        case 29: // lod.active.player.fk.s3
            _lmconfig.lod.lodActivePlayerFKs3 = *(float*)pValue;
            LOG1F(L"lod.active.player.fk.s3 = %03f", 
                    _lmconfig.lod.lodActivePlayerFKs3);
            break;
	}
}

void getResolution(DWORD& width, DWORD& height)
{
	DWORD protection;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;

    // get resolution
    if (VirtualProtect((BYTE*)data[SCREEN_WIDTH], 4, newProtection, &protection))
        width = *(DWORD*)data[SCREEN_WIDTH]; 
    if (VirtualProtect((BYTE*)data[SCREEN_HEIGHT], 4, newProtection, &protection))
        height = *(DWORD*)data[SCREEN_HEIGHT];
}

void setResolution(DWORD width, DWORD height)
{
	DWORD protection;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;

    // set resolution
    if (width>0 && height>0)
    {
        if (VirtualProtect((BYTE*)data[SCREEN_WIDTH], 4, newProtection, &protection))
            *(DWORD*)data[SCREEN_WIDTH] = width;
        if (VirtualProtect((BYTE*)data[SCREEN_HEIGHT], 4, newProtection, &protection))
            *(DWORD*)data[SCREEN_HEIGHT] = height;

        LOG2N(L"Resolution set: %dx%d", width, height);
    }
}

void setAspectRatio(float aspectRatio, bool manual)
{
	DWORD protection;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;

    if (aspectRatio <= FLOAT_ZERO) // safety-check
        return;

    if (fabs(aspectRatio - 1.33333) < fabs(aspectRatio - 1.77777)) {
        // closer to 4:3
        *(DWORD*)data[WIDESCREEN_FLAG] = 0;
        if (VirtualProtect((BYTE*)data[RATIO_4on3], 4, newProtection, &protection)) {
            *(float*)data[RATIO_4on3] = aspectRatio;
        }
        LOG(L"Widescreen mode: no");
    } else {
        // closer to 16:9
        *(DWORD*)data[WIDESCREEN_FLAG] = 1;
        if (VirtualProtect((BYTE*)data[RATIO_16on9], 4, newProtection, &protection)) {
            *(float*)data[RATIO_16on9] = aspectRatio;
        }
        LOG(L"Widescreen mode: yes");
    }
    LOG1F(L"Aspect ratio: %0.5f", aspectRatio);
    LOG1S(L"Aspect ratio type: %s", (manual)?L"manual":L"automatic");
}

void lodAtModeCheckCallPoint()
{
    __asm {
        pushfd 
        push ebp
        push eax
        push ebx
        push edx
        push esi
        push edi
        mov [edi+0x13c], eax  // execute replaced code
        push ecx
        call lodAtModeCheck
        mov ecx,eax
        add esp,4     // pop parameters
        pop edi
        pop esi
        pop edx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

void lodAtModeCheckCallPoint2010()
{
    __asm {
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        call lodAtModeCheck
        mov edi,eax
        add esp,4     // pop parameters
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        mov dword ptr ds:[esi+0x13c],ecx // execute replaced code
        retn
    }
}

KEXPORT DWORD lodAtModeCheck(DWORD mode)
{
    if (mode != 0)
        return 1;
    return 0;
}

void lodAtSettingsReadPoint()
{
    __asm {
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        call modifySettings
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

void lodAtSettingsResetPoint()
{
    __asm {
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        call modifySettings
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

