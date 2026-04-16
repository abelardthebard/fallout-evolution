// FE: "Please Stand By" splash screen shown before title screen
#include "global.h"
#include "bg.h"
#include "decompress.h"
#include "gpu_regs.h"
#include "main.h"
#include "palette.h"
#include "sprite.h"
#include "task.h"
#include "title_screen.h"
#include "intro.h"
#include "m4a.h"
#include "constants/rgb.h"
#include "constants/songs.h"

#define SPLASH_DURATION     300 // 5 seconds at 60fps
#define SPLASH_FADE_DELAY   4   // fade speed (4 = ~1 second fade in/out)
#define CIRCLE_FRAME_HOLD   3   // frames to hold each rotation frame
#define NUM_CIRCLE_FRAMES   10

#define TAG_SPLASH_CIRCLE   30000

// Graphics data
static const u32 sSplashBgGfx[] = INCBIN_U32("graphics/title_screen/PleaseStandBy.4bpp.smol");
static const u32 sSplashBgTilemap[] = INCBIN_U32("graphics/title_screen/PleaseStandBy.bin.smolTM");
static const u32 sSplashCircleGfx[] = INCBIN_U32("graphics/title_screen/splash_circle.4bpp.smol");
static const u16 sSplashBgPal[] = INCBIN_U16("graphics/title_screen/PleaseStandBy.gbapal");
static const u16 sSplashCirclePal[] = INCBIN_U16("graphics/title_screen/splash_circle.gbapal");

// Circle sprite animation — 10 frames, each 32x32 = 16 tiles per frame in 4bpp
static const union AnimCmd sSplashCircleAnim[] =
{
    ANIMCMD_FRAME(0,   CIRCLE_FRAME_HOLD),
    ANIMCMD_FRAME(16,  CIRCLE_FRAME_HOLD),
    ANIMCMD_FRAME(32,  CIRCLE_FRAME_HOLD),
    ANIMCMD_FRAME(48,  CIRCLE_FRAME_HOLD),
    ANIMCMD_FRAME(64,  CIRCLE_FRAME_HOLD),
    ANIMCMD_FRAME(80,  CIRCLE_FRAME_HOLD),
    ANIMCMD_FRAME(96,  CIRCLE_FRAME_HOLD),
    ANIMCMD_FRAME(112, CIRCLE_FRAME_HOLD),
    ANIMCMD_FRAME(128, CIRCLE_FRAME_HOLD),
    ANIMCMD_FRAME(144, CIRCLE_FRAME_HOLD),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const sSplashCircleAnimTable[] =
{
    sSplashCircleAnim,
};

static const struct OamData sSplashCircleOam =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x32),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
};

static const struct CompressedSpriteSheet sSplashCircleSpriteSheet =
{
    .data = sSplashCircleGfx,
    .size = 32 * 32 / 2 * NUM_CIRCLE_FRAMES, // 10 frames, 32x32, 4bpp
    .tag = TAG_SPLASH_CIRCLE,
};

static const struct SpritePalette sSplashCircleSpritePal =
{
    .data = sSplashCirclePal,
    .tag = TAG_SPLASH_CIRCLE,
};

static const struct SpriteTemplate sSplashCircleSpriteTemplate =
{
    .tileTag = TAG_SPLASH_CIRCLE,
    .paletteTag = TAG_SPLASH_CIRCLE,
    .oam = &sSplashCircleOam,
    .anims = sSplashCircleAnimTable,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

// Corner positions for the 4 circles (centered on the corner decorations)
#define CIRCLE_TL_X  36
#define CIRCLE_TL_Y  28
#define CIRCLE_TR_X  204
#define CIRCLE_TR_Y  28
#define CIRCLE_BL_X  36
#define CIRCLE_BL_Y  132
#define CIRCLE_BR_X  204
#define CIRCLE_BR_Y  132

#define tCounter data[0]
#define tState   data[1]

enum {
    SPLASH_STATE_FADE_IN,
    SPLASH_STATE_HOLD,
    SPLASH_STATE_FADE_OUT,
    SPLASH_STATE_SILENCE,
    SPLASH_STATE_DONE,
};

static void Task_SplashScreen(u8 taskId);
static void VBlankCB_Splash(void);
void CB2_SplashScreenMain(void);
void CB2_InitLogoAnim(void);

static void VBlankCB_Splash(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void Task_SplashScreen(u8 taskId)
{
    switch (gTasks[taskId].tState)
    {
    case SPLASH_STATE_FADE_IN:
        if (!gPaletteFade.active)
        {
            gTasks[taskId].tState = SPLASH_STATE_HOLD;
            gTasks[taskId].tCounter = SPLASH_DURATION;
        }
        break;
    case SPLASH_STATE_HOLD:
        if (--gTasks[taskId].tCounter == 0 || JOY_NEW(A_BUTTON | B_BUTTON | START_BUTTON))
        {
            BeginNormalPaletteFade(PALETTES_ALL, SPLASH_FADE_DELAY, 0, 16, RGB_BLACK);
            gTasks[taskId].tState = SPLASH_STATE_FADE_OUT;
        }
        break;
    case SPLASH_STATE_FADE_OUT:
        if (!gPaletteFade.active)
        {
            // Stop static immediately at fade-out end; hold 1s black+silence before disclaimer
            m4aSongNumStop(MUS_STATIC_LOOP);
            gTasks[taskId].tCounter = 60;
            gTasks[taskId].tState = SPLASH_STATE_SILENCE;
        }
        break;
    case SPLASH_STATE_SILENCE:
        if (--gTasks[taskId].tCounter == 0)
            gTasks[taskId].tState = SPLASH_STATE_DONE;
        break;
    case SPLASH_STATE_DONE:
        ResetSpriteData();
        FreeAllSpritePalettes();
        DestroyTask(taskId);
        SetMainCallback2(CB2_InitCopyrightScreenAfterBootup);
        break;
    }
}

void CB2_InitSplashScreen(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        CpuFill32(0, (void *)VRAM, VRAM_SIZE);
        CpuFill32(0, (void *)OAM, OAM_SIZE);
        CpuFill16(0, (void *)(PLTT + 2), PLTT_SIZE - 2);
        ResetPaletteFade();
        ResetTasks();
        ResetSpriteData();
        FreeAllSpritePalettes();
        gMain.state++;
        break;
    case 1:
        // Load BG
        DecompressDataWithHeaderVram(sSplashBgGfx, (void *)BG_CHAR_ADDR(0));
        DecompressDataWithHeaderVram(sSplashBgTilemap, (void *)BG_SCREEN_ADDR(7));
        LoadPalette(sSplashBgPal, BG_PLTT_ID(0), PLTT_SIZE_4BPP);

        // Load circle sprites
        LoadCompressedSpriteSheet(&sSplashCircleSpriteSheet);
        LoadSpritePalette(&sSplashCircleSpritePal);

        // Create 4 corner circles
        CreateSprite(&sSplashCircleSpriteTemplate, CIRCLE_TL_X, CIRCLE_TL_Y, 0);
        CreateSprite(&sSplashCircleSpriteTemplate, CIRCLE_TR_X, CIRCLE_TR_Y, 0);
        CreateSprite(&sSplashCircleSpriteTemplate, CIRCLE_BL_X, CIRCLE_BL_Y, 0);
        CreateSprite(&sSplashCircleSpriteTemplate, CIRCLE_BR_X, CIRCLE_BR_Y, 0);

        gMain.state++;
        break;
    case 2:
        SetGpuReg(REG_OFFSET_BG0CNT, BGCNT_PRIORITY(0)
                                    | BGCNT_CHARBASE(0)
                                    | BGCNT_SCREENBASE(7)
                                    | BGCNT_16COLOR
                                    | BGCNT_TXT256x256);
        SetGpuReg(REG_OFFSET_BG0HOFS, 0);
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0
                                    | DISPCNT_OBJ_1D_MAP
                                    | DISPCNT_BG0_ON
                                    | DISPCNT_OBJ_ON);
        SetVBlankCallback(VBlankCB_Splash);
        BeginNormalPaletteFade(PALETTES_ALL, SPLASH_FADE_DELAY, 16, 0, RGB_BLACK);

        // Start looping TV static under the "Please Stand By" scene
        m4aSongNumStart(MUS_STATIC_LOOP);

        CreateTask(Task_SplashScreen, 0);
        SetMainCallback2(CB2_SplashScreenMain);
        break;
    }
}

void CB2_SplashScreenMain(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

// =============================================
// FE: Abelard Logo Animation (26 frames)
// Plays between splash screen and title screen
// =============================================

#define LOGO_ANIM_NUM_FRAMES   26
#define LOGO_ANIM_FRAME_HOLD   4   // hardware frames per animation frame (~15fps)
#define LOGO_ANIM_FADE_DELAY   4
#define LOGO_ANIM_LOGO_HOLD    180 // hold frame 20 for 3 seconds
#define LOGO_ANIM_BLACK_HOLD   180 // hold frame 0 (black) for 3 seconds
#define LOGO_ANIM_LOGO_FRAME   20  // frame to hold on (full logo)

// Per-frame tilesets (compressed)
static const u32 sLogoAnimGfx_00[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame00.4bpp.smol");
static const u32 sLogoAnimGfx_01[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame01.4bpp.smol");
static const u32 sLogoAnimGfx_02[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame02.4bpp.smol");
static const u32 sLogoAnimGfx_03[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame03.4bpp.smol");
static const u32 sLogoAnimGfx_04[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame04.4bpp.smol");
static const u32 sLogoAnimGfx_05[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame05.4bpp.smol");
static const u32 sLogoAnimGfx_06[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame06.4bpp.smol");
static const u32 sLogoAnimGfx_07[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame07.4bpp.smol");
static const u32 sLogoAnimGfx_08[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame08.4bpp.smol");
static const u32 sLogoAnimGfx_09[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame09.4bpp.smol");
static const u32 sLogoAnimGfx_10[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame10.4bpp.smol");
static const u32 sLogoAnimGfx_11[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame11.4bpp.smol");
static const u32 sLogoAnimGfx_12[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame12.4bpp.smol");
static const u32 sLogoAnimGfx_13[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame13.4bpp.smol");
static const u32 sLogoAnimGfx_14[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame14.4bpp.smol");
static const u32 sLogoAnimGfx_15[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame15.4bpp.smol");
static const u32 sLogoAnimGfx_16[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame16.4bpp.smol");
static const u32 sLogoAnimGfx_17[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame17.4bpp.smol");
static const u32 sLogoAnimGfx_18[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame18.4bpp.smol");
static const u32 sLogoAnimGfx_19[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame19.4bpp.smol");
static const u32 sLogoAnimGfx_20[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame20.4bpp.smol");
static const u32 sLogoAnimGfx_21[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame21.4bpp.smol");
static const u32 sLogoAnimGfx_22[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame22.4bpp.smol");
static const u32 sLogoAnimGfx_23[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame23.4bpp.smol");
static const u32 sLogoAnimGfx_24[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame24.4bpp.smol");
static const u32 sLogoAnimGfx_25[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame25.4bpp.smol");

static const u32 *const sLogoAnimGfxTable[LOGO_ANIM_NUM_FRAMES] = {
    sLogoAnimGfx_00, sLogoAnimGfx_01, sLogoAnimGfx_02, sLogoAnimGfx_03,
    sLogoAnimGfx_04, sLogoAnimGfx_05, sLogoAnimGfx_06, sLogoAnimGfx_07,
    sLogoAnimGfx_08, sLogoAnimGfx_09, sLogoAnimGfx_10, sLogoAnimGfx_11,
    sLogoAnimGfx_12, sLogoAnimGfx_13, sLogoAnimGfx_14, sLogoAnimGfx_15,
    sLogoAnimGfx_16, sLogoAnimGfx_17, sLogoAnimGfx_18, sLogoAnimGfx_19,
    sLogoAnimGfx_20, sLogoAnimGfx_21, sLogoAnimGfx_22, sLogoAnimGfx_23,
    sLogoAnimGfx_24, sLogoAnimGfx_25,
};

// Per-frame tilemaps (compressed)
static const u32 sLogoAnimMap_00[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame00.bin.smolTM");
static const u32 sLogoAnimMap_01[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame01.bin.smolTM");
static const u32 sLogoAnimMap_02[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame02.bin.smolTM");
static const u32 sLogoAnimMap_03[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame03.bin.smolTM");
static const u32 sLogoAnimMap_04[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame04.bin.smolTM");
static const u32 sLogoAnimMap_05[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame05.bin.smolTM");
static const u32 sLogoAnimMap_06[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame06.bin.smolTM");
static const u32 sLogoAnimMap_07[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame07.bin.smolTM");
static const u32 sLogoAnimMap_08[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame08.bin.smolTM");
static const u32 sLogoAnimMap_09[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame09.bin.smolTM");
static const u32 sLogoAnimMap_10[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame10.bin.smolTM");
static const u32 sLogoAnimMap_11[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame11.bin.smolTM");
static const u32 sLogoAnimMap_12[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame12.bin.smolTM");
static const u32 sLogoAnimMap_13[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame13.bin.smolTM");
static const u32 sLogoAnimMap_14[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame14.bin.smolTM");
static const u32 sLogoAnimMap_15[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame15.bin.smolTM");
static const u32 sLogoAnimMap_16[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame16.bin.smolTM");
static const u32 sLogoAnimMap_17[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame17.bin.smolTM");
static const u32 sLogoAnimMap_18[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame18.bin.smolTM");
static const u32 sLogoAnimMap_19[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame19.bin.smolTM");
static const u32 sLogoAnimMap_20[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame20.bin.smolTM");
static const u32 sLogoAnimMap_21[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame21.bin.smolTM");
static const u32 sLogoAnimMap_22[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame22.bin.smolTM");
static const u32 sLogoAnimMap_23[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame23.bin.smolTM");
static const u32 sLogoAnimMap_24[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame24.bin.smolTM");
static const u32 sLogoAnimMap_25[] = INCBIN_U32("graphics/title_screen/abelard_anim/frame25.bin.smolTM");

static const u32 *const sLogoAnimMapTable[LOGO_ANIM_NUM_FRAMES] = {
    sLogoAnimMap_00, sLogoAnimMap_01, sLogoAnimMap_02, sLogoAnimMap_03,
    sLogoAnimMap_04, sLogoAnimMap_05, sLogoAnimMap_06, sLogoAnimMap_07,
    sLogoAnimMap_08, sLogoAnimMap_09, sLogoAnimMap_10, sLogoAnimMap_11,
    sLogoAnimMap_12, sLogoAnimMap_13, sLogoAnimMap_14, sLogoAnimMap_15,
    sLogoAnimMap_16, sLogoAnimMap_17, sLogoAnimMap_18, sLogoAnimMap_19,
    sLogoAnimMap_20, sLogoAnimMap_21, sLogoAnimMap_22, sLogoAnimMap_23,
    sLogoAnimMap_24, sLogoAnimMap_25,
};

// Palette from frame12 (most complex frame, has all colors)
static const u16 sLogoAnimPal[] = INCBIN_U16("graphics/title_screen/abelard_anim/frame12.gbapal");

#define tAnimFrame  data[0]
#define tHoldTimer  data[1]
#define tState2     data[2]

enum {
    LOGO_ANIM_FADE_IN,
    LOGO_ANIM_PLAYING,
    LOGO_ANIM_HOLD_LOGO,
    LOGO_ANIM_PLAYING_END,
    LOGO_ANIM_HOLD_BLACK,
    LOGO_ANIM_DONE,
};

static void Task_LogoAnim(u8 taskId);
static void LogoAnim_LoadFrame(u8 frameNum);
static void LogoAnim_LoadFrameToBank(u8 frameNum, u8 bank);
static void LogoAnim_SwapBank(void);
static void VBlankCB_LogoAnim(void);
void CB2_LogoAnimMain(void);

// Double-buffer: two VRAM banks for flicker-free frame swaps
// Bank 0: CHARBASE(0) + SCREENBASE(31)  — tiles at 0x06000000, map at 0x0600F800
// Bank 1: CHARBASE(2) + SCREENBASE(30)  — tiles at 0x06008000, map at 0x0600F000
#define LOGO_BANK_CHARBASE_0   0
#define LOGO_BANK_SCREENBASE_0 31
#define LOGO_BANK_CHARBASE_1   2
#define LOGO_BANK_SCREENBASE_1 30

static u8 sLogoAnimActiveBank;   // which bank is currently displayed (0 or 1)
static bool8 sLogoAnimSwapReady; // next frame is ready in the off-screen bank

static void VBlankCB_LogoAnim(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();

    // Swap BG0 to the new bank during VBlank (instant, no flicker)
    if (sLogoAnimSwapReady)
    {
        u8 charbase = (sLogoAnimActiveBank == 0) ? LOGO_BANK_CHARBASE_0 : LOGO_BANK_CHARBASE_1;
        u8 screenbase = (sLogoAnimActiveBank == 0) ? LOGO_BANK_SCREENBASE_0 : LOGO_BANK_SCREENBASE_1;
        SetGpuReg(REG_OFFSET_BG0CNT, BGCNT_PRIORITY(0)
                                    | BGCNT_CHARBASE(charbase)
                                    | BGCNT_SCREENBASE(screenbase)
                                    | BGCNT_16COLOR
                                    | BGCNT_TXT256x256);
        sLogoAnimSwapReady = FALSE;
    }
}

// Load a frame into the INACTIVE bank (safe — PPU isn't reading it)
static void LogoAnim_LoadFrameToBank(u8 frameNum, u8 bank)
{
    u8 charbase = (bank == 0) ? LOGO_BANK_CHARBASE_0 : LOGO_BANK_CHARBASE_1;
    u8 screenbase = (bank == 0) ? LOGO_BANK_SCREENBASE_0 : LOGO_BANK_SCREENBASE_1;
    DecompressDataWithHeaderVram(sLogoAnimGfxTable[frameNum], (void *)BG_CHAR_ADDR(charbase));
    DecompressDataWithHeaderVram(sLogoAnimMapTable[frameNum], (void *)BG_SCREEN_ADDR(screenbase));
}

// Queue a bank swap for the next VBlank
static void LogoAnim_SwapBank(void)
{
    sLogoAnimActiveBank ^= 1; // toggle 0↔1
    sLogoAnimSwapReady = TRUE;
}

// Legacy wrapper for first frame load (display is off, direct write is safe)
static void LogoAnim_LoadFrame(u8 frameNum)
{
    LogoAnim_LoadFrameToBank(frameNum, 0);
    sLogoAnimActiveBank = 0;
    sLogoAnimSwapReady = FALSE;
}

static void Task_LogoAnim(u8 taskId)
{
    switch (gTasks[taskId].tState2)
    {
    case LOGO_ANIM_FADE_IN:
        // No fade — frame 0 is already black. Go straight to playing.
        gTasks[taskId].tState2 = LOGO_ANIM_PLAYING;
        gTasks[taskId].tAnimFrame = 0;
        gTasks[taskId].tHoldTimer = 0;
        break;
    case LOGO_ANIM_PLAYING:
        if (JOY_NEW(A_BUTTON | B_BUTTON | START_BUTTON))
        {
            gTasks[taskId].tState2 = LOGO_ANIM_DONE;
            break;
        }

        gTasks[taskId].tHoldTimer++;

        // Preload next frame into inactive bank halfway through the hold
        if (gTasks[taskId].tHoldTimer == LOGO_ANIM_FRAME_HOLD / 2)
        {
            u8 nextFrame = gTasks[taskId].tAnimFrame + 1;
            if (nextFrame < LOGO_ANIM_NUM_FRAMES)
            {
                u8 inactiveBank = sLogoAnimActiveBank ^ 1;
                LogoAnim_LoadFrameToBank(nextFrame, inactiveBank);
            }
        }

        // Swap to the preloaded bank when hold is done
        if (gTasks[taskId].tHoldTimer >= LOGO_ANIM_FRAME_HOLD)
        {
            gTasks[taskId].tHoldTimer = 0;
            gTasks[taskId].tAnimFrame++;

            if (gTasks[taskId].tAnimFrame == LOGO_ANIM_LOGO_FRAME)
            {
                // Hit the logo frame — hold for 3 seconds
                LogoAnim_SwapBank();
                gTasks[taskId].tHoldTimer = 0;
                gTasks[taskId].tState2 = LOGO_ANIM_HOLD_LOGO;
            }
            else if (gTasks[taskId].tAnimFrame >= LOGO_ANIM_NUM_FRAMES)
            {
                // All frames played — preload frame 0 (black) and swap to it
                {
                    u8 inactiveBank = sLogoAnimActiveBank ^ 1;
                    LogoAnim_LoadFrameToBank(0, inactiveBank);
                    LogoAnim_SwapBank();
                }
                gTasks[taskId].tHoldTimer = 0;
                gTasks[taskId].tState2 = LOGO_ANIM_HOLD_BLACK;
            }
            else
            {
                LogoAnim_SwapBank();
            }
        }
        break;

    case LOGO_ANIM_HOLD_LOGO:
        // Hold on the full logo (frame 20) for 3 seconds
        if (JOY_NEW(A_BUTTON | B_BUTTON | START_BUTTON))
        {
            gTasks[taskId].tState2 = LOGO_ANIM_DONE;
            break;
        }
        if (++gTasks[taskId].tHoldTimer >= LOGO_ANIM_LOGO_HOLD)
        {
            // Resume playing from frame 21
            gTasks[taskId].tHoldTimer = 0;
            gTasks[taskId].tState2 = LOGO_ANIM_PLAYING;
        }
        break;

    case LOGO_ANIM_HOLD_BLACK:
        // Hold black (frame 0) for 3 seconds then transition
        if (JOY_NEW(A_BUTTON | B_BUTTON | START_BUTTON))
        {
            gTasks[taskId].tState2 = LOGO_ANIM_DONE;
            break;
        }
        if (++gTasks[taskId].tHoldTimer >= LOGO_ANIM_BLACK_HOLD)
        {
            gTasks[taskId].tState2 = LOGO_ANIM_DONE;
        }
        break;
    case LOGO_ANIM_DONE:
        DestroyTask(taskId);
        SetMainCallback2(CB2_InitTitleScreen);
        break;
    }
}

void CB2_InitLogoAnim(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        CpuFill32(0, (void *)VRAM, VRAM_SIZE);
        CpuFill32(0, (void *)OAM, OAM_SIZE);
        CpuFill16(0, (void *)(PLTT + 2), PLTT_SIZE - 2);
        ResetPaletteFade();
        ResetTasks();
        ResetSpriteData();
        FreeAllSpritePalettes();
        gMain.state++;
        break;
    case 1:
        // Load first frame into bank 0 (display is off, safe to write VRAM directly)
        LogoAnim_LoadFrame(0);
        LoadPalette(sLogoAnimPal, BG_PLTT_ID(0), PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    case 2:
        SetGpuReg(REG_OFFSET_BG0CNT, BGCNT_PRIORITY(0)
                                    | BGCNT_CHARBASE(LOGO_BANK_CHARBASE_0)
                                    | BGCNT_SCREENBASE(LOGO_BANK_SCREENBASE_0)
                                    | BGCNT_16COLOR
                                    | BGCNT_TXT256x256);
        SetGpuReg(REG_OFFSET_BG0HOFS, 0);
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0
                                    | DISPCNT_OBJ_1D_MAP
                                    | DISPCNT_BG0_ON);
        SetVBlankCallback(VBlankCB_LogoAnim);
        CreateTask(Task_LogoAnim, 0);
        SetMainCallback2(CB2_LogoAnimMain);
        break;
    }
}

void CB2_LogoAnimMain(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

#undef tAnimFrame
#undef tHoldTimer
#undef tState2
