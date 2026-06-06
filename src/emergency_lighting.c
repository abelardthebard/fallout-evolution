#include "global.h"
#include "emergency_lighting.h"
#include "event_data.h"
#include "field_weather.h"
#include "palette.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "constants/flags.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// ============================================================================
// POWER OUTAGE / EMERGENCY LIGHTING
// ----------------------------------------------------------------------------
// The flicker is a one-shot palette blink to black (modelled on the Berry Crush
// impact flash, src/berry_crush.c, and the weather lightning state machine,
// src/field_weather_effect.c). It runs while the player is locked (lockall
// freezes object events), so BlendPalettes holds cleanly with no streaming.
//
// On completion it toggles FLAG_EMERGENCY_LIGHTING and refreshes the overworld
// palettes; the persistent red wash itself is applied in field_weather.c's
// palette-reload path (ApplyEmergencyTintToRange), so it survives walking and
// map changes. See RefreshEmergencyLighting().
// ============================================================================

// {blend coeff toward black (0-16), frames to hold}. Hand-tuned irregular
// "failing power" stutter -- tune freely.
static const u8 sOutageFlicker[][2] =
{
    {16, 2}, {0, 3}, {16, 2}, {0, 9}, {16, 3}, {0, 2},
    {16, 1}, {0, 6}, {16, 4}, {0, 2}, {16, 12},
};

// Restore: a couple of stutters as power surges back, then steady.
static const u8 sRestoreFlicker[][2] =
{
    {16, 2}, {0, 2}, {16, 1}, {0, 5}, {16, 2}, {0, 8},
};

// Leave the text/UI palette (slot 15) untouched while flickering.
#define FLICKER_MASK   (PALETTES_ALL & ~(1 << 15))

#define tStep     data[0]
#define tTimer    data[1]
#define tRestore  data[2]

static void Task_PowerFlicker(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    const u8 (*pattern)[2] = tRestore ? sRestoreFlicker : sOutageFlicker;
    u8 len = tRestore ? ARRAY_COUNT(sRestoreFlicker) : ARRAY_COUNT(sOutageFlicker);

    if (tTimer != 0)
    {
        tTimer--;
        return;
    }

    if (tStep >= len)
    {
        // Flicker finished -- settle the persistent state and hand control back.
        if (tRestore)
            FlagClear(FLAG_EMERGENCY_LIGHTING);
        else
            FlagSet(FLAG_EMERGENCY_LIGHTING);
        RefreshEmergencyLighting();
        DestroyTask(taskId);
        ScriptContext_Enable();
        return;
    }

    BlendPalettes(FLICKER_MASK, pattern[tStep][0], RGB_BLACK);
    tTimer = pattern[tStep][1];
    tStep++;
}

static void StartPowerFlicker(u8 restore, u16 sound)
{
    u8 taskId = CreateTask(Task_PowerFlicker, 80);
    gTasks[taskId].data[0] = 0;   // tStep
    gTasks[taskId].data[1] = 0;   // tTimer
    gTasks[taskId].data[2] = restore;
    PlaySE(sound);
}

void StartPowerOutage(void)
{
    StartPowerFlicker(FALSE, SE_PC_OFF);
}

void StartPowerRestore(void)
{
    StartPowerFlicker(TRUE, SE_PC_ON);
}

#undef tStep
#undef tTimer
#undef tRestore
