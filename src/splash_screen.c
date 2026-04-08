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
#include "constants/rgb.h"

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
    SPLASH_STATE_DONE,
};

static void Task_SplashScreen(u8 taskId);
static void VBlankCB_Splash(void);
void CB2_SplashScreenMain(void);

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
            gTasks[taskId].tState = SPLASH_STATE_DONE;
        }
        break;
    case SPLASH_STATE_DONE:
        ResetSpriteData();
        FreeAllSpritePalettes();
        DestroyTask(taskId);
        SetMainCallback2(CB2_InitTitleScreen);
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
