#include "global.h"
#include "terminal.h"
#include "terminal_internal.h"

const struct TerminalPage *const gTerminalContents[NUM_TERMINAL_CONTENTS] =
{
    [TERMINAL_CONTENT_MEDICAL_RECORDS] = &sTerminalMedicalRecords_Page,
    [TERMINAL_CONTENT_TEST_SCORES]     = &sTerminalTestScores_Page,
};
