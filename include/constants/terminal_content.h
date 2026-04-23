#ifndef GUARD_CONSTANTS_TERMINAL_CONTENT_H
#define GUARD_CONSTANTS_TERMINAL_CONTENT_H

// Content IDs. Usable by both .inc scripts (via the `terminal` macro) and
// the C lookup table (gTerminalContents) in src/terminal_content_pages.c.
// Append new entries at the end; the table is indexed by these IDs, so
// reordering would mis-map existing scripts.
#define TERMINAL_CONTENT_MEDICAL_RECORDS    0

#define NUM_TERMINAL_CONTENTS               1

#endif // GUARD_CONSTANTS_TERMINAL_CONTENT_H
