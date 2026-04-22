#ifndef GUARD_CONSTANTS_TERMINAL_HACK_H
#define GUARD_CONSTANTS_TERMINAL_HACK_H

// Defined as #defines (not an enum) so they're visible to GAS in .inc macros.
// TIER_0 skips the minigame (no-password terminal); TIER_1..3 run the hack.
#define TERMINAL_TIER_0         0
#define TERMINAL_TIER_1         1
#define TERMINAL_TIER_2         2
#define TERMINAL_TIER_3         3
#define NUM_TERMINAL_TIERS      4

#define TERMINAL_MAX_ATTEMPTS      4
#define TERMINAL_WORD_COUNT        10

#define TERMINAL_MIN_WORD_LENGTH   4
#define TERMINAL_MAX_WORD_LENGTH   10
#define TERMINAL_NUM_LENGTH_BUCKETS (TERMINAL_MAX_WORD_LENGTH - TERMINAL_MIN_WORD_LENGTH + 1)

// Must match SaveBlock1.terminalWordBurned bitfield capacity in bits.
#define TERMINAL_MAX_WORD_IDS      512

// Upper bound on any single bucket's count. Bump if a bucket approaches this.
#define TERMINAL_MAX_POOL_SAMPLE   128

#endif // GUARD_CONSTANTS_TERMINAL_HACK_H
