#ifndef GUARD_DATA_TERMINAL_HACK_H
#define GUARD_DATA_TERMINAL_HACK_H

#include "constants/terminal_hack.h"

struct TerminalDifficulty
{
    u8 wordLenMin;
    u8 wordLenMax;
    u8 wordCount;
    u8 lockoutSteps;
    u8 bracketPairsMin;
    u8 bracketPairsMax;
};

static const struct TerminalDifficulty sTerminalDifficulty[NUM_TERMINAL_TIERS] =
{
    [TERMINAL_TIER_1] = { .wordLenMin =  4, .wordLenMax =  5, .wordCount = 10, .lockoutSteps =  50, .bracketPairsMin = 3, .bracketPairsMax = 4 },
    [TERMINAL_TIER_2] = { .wordLenMin =  6, .wordLenMax =  8, .wordCount = 10, .lockoutSteps =  75, .bracketPairsMin = 5, .bracketPairsMax = 6 },
    [TERMINAL_TIER_3] = { .wordLenMin =  9, .wordLenMax = 10, .wordCount = 10, .lockoutSteps = 100, .bracketPairsMin = 7, .bracketPairsMax = 8 },
};

// Only chars available in pokeemerald's charmap. `[` `]` `{` `}` `@` `#` `^`
// `*` `"` `|` `\` would require charmap + font edits to add.
static const u8 sTerminalGarbageChars[]    = _("!%&-+,.;:'?/·×");
static const u8 sTerminalBracketOpeners[]  = _("(<");
static const u8 sTerminalBracketClosers[]  = _(")>");

#define TERMINAL_NUM_GARBAGE_CHARS  (sizeof(sTerminalGarbageChars)   - 1)
#define TERMINAL_NUM_BRACKET_TYPES  (sizeof(sTerminalBracketOpeners) - 1)

// APPEND-ONLY: inserting or reordering shifts every subsequent word's global
// id, which silently corrupts burn-flag state in existing saves. New words
// go at the end of their bucket; new buckets go after all existing ones.
static const u8 sTerminalWordsLen4[][5] = {
    _("ROCK"), _("BARD"), _("FIRE"), _("HACK"),
    _("DARK"), _("NUKE"), _("BALL"), _("TECH"),
    _("GAME"), _("BUGS"), _("ICED"), _("OPEN"),
    _("CATS"), _("FORT"),
    _("CAPS"), _("CRAM"), _("COLA"), _("GLOW"),
    _("ATOM"), _("BOMB"), _("CORE"), _("RAID"),
    _("DRUM"), _("DUST"), _("RUST"), _("WIRE"),
    _("GEAR"), _("GATE"), _("DOOR"), _("DUCK"),
    _("COVE"), _("FUEL"), _("LEAD"), _("SAFE"),
    _("TAPE"), _("CODE"), _("IRON"), _("MELT"),
    _("WARD"), _("FOOD"), _("BUNK"), _("SILO"),
    _("LOCK"), _("KEYS"), _("WIFE"), _("DRAB"),
    _("STAR"), _("RANK"), _("MEDS"), _("PIPS"),
    _("CHEM"), _("JUNK"), _("WAVE"), _("TOWN"),
    _("HOLE"), _("FOIL"), _("FLAG"), _("KILN"),
};

static const u8 sTerminalWordsLen5[][6] = {
    _("GHOST"), _("STEEL"), _("WATER"), _("GRASS"),
    _("FAIRY"), _("PRICE"), _("VAULT"), _("FREAK"),
    _("BOMBS"), _("HUMAN"), _("GHOUL"), _("RADIO"),
    _("POWER"),
    _("ATOMS"), _("ROBCO"), _("DIODE"), _("WIRES"),
    _("RIFLE"), _("RUSTY"), _("METRO"), _("SCRAP"),
    _("CANDY"), _("SUGAR"), _("FANCY"), _("DANDY"),
    _("APPLE"), _("STIMS"), _("TRUCE"), _("AUDIO"),
    _("FLAGS"), _("CRATE"), _("RATIO"), _("DRILL"),
    _("SIREN"), _("ALARM"), _("TOWER"), _("SKULL"),
    _("GAMMA"), _("CABLE"), _("CRANK"), _("SCOUT"),
    _("WAGON"), _("CREED"), _("CROPS"), _("PIPES"),
    _("METAL"), _("ARROW"), _("GAUGE"), _("RELAY"),
    _("SERUM"), _("STORM"), _("PROBE"), _("VALOR"),
    _("MEDAL"), _("CADET"), _("CHURN"), _("CIVIC"),
};

static const u8 sTerminalWordsLen6[][7] = {
    _("NORMAL"), _("FLYING"), _("POISON"), _("GROUND"),
    _("DRAGON"), _("FUTURE"), _("RUGRAT"), _("MUTANT"),
    _("ATOMIC"), _("ENERGY"), _("FUSION"), _("VAULTS"),
    _("ROBOTS"), _("RAIDER"), _("SETTLE"), _("DRAFTS"),
    _("WAGONS"), _("SODIUM"), _("COPPER"), _("NICKEL"),
    _("SILVER"), _("GADGET"), _("POCKET"), _("ORANGE"),
    _("STATIC"), _("ANTHEM"), _("EAGLES"), _("BANNER"),
    _("PLEDGE"), _("HONEST"), _("BEACON"), _("RATION"),
    _("REFUGE"), _("RESCUE"), _("SHIELD"), _("TURRET"),
    _("ARMORY"), _("RIFLES"), _("PISTOL"), _("SCRAPS"),
    _("CAVERN"), _("ALCOVE"), _("COMBAT"), _("SNACKS"),
    _("FREEZE"), _("TOASTY"), _("DOCTOR"), _("PARLOR"),
    _("SALOON"),
};

static const u8 sTerminalWordsLen7[][8] = {
    _("PSYCHIC"), _("ABELARD"), _("FALLOUT"), _("NUCLEAR"),
    _("ADVANCE"), _("AMERICA"), _("URANIUM"), _("MONSTER"),
    _("CHICAGO"), _("ROMHACK"), _("LIBERTY"), _("ECONOMY"),
    _("RAIDERS"), _("CARAVAN"), _("SETTLER"), _("REACTOR"),
    _("CAPSULE"), _("COMPASS"), _("CIRCUIT"), _("RUSTING"),
    _("ORDERLY"), _("BUNKERS"), _("STORAGE"), _("CANTEEN"),
    _("COMPUTE"), _("GARBAGE"), _("CHAMBER"), _("FACTORY"),
    _("GRENADE"), _("ISOTOPE"), _("COMPANY"), _("FACTION"),
    _("FREEDOM"), _("PRIMARY"), _("PROPHET"), _("PROTEGE"),
    _("PROJECT"), _("PROGRAM"), _("PATTERN"), _("MISSILE"),
    _("NEUTRON"), _("NEWSMAN"), _("OUTPOST"), _("PACKAGE"),
    _("PATRIOT"), _("PENCILS"), _("PHARAOH"), _("PHOENIX"),
    _("PHYSICS"), _("PLASTIC"), _("PRAYERS"),
};

static const u8 sTerminalWordsLen8[][9] = {
    _("FIGHTING"), _("ELECTRIC"), _("PASSWORD"), _("BETHESDA"),
    _("NINTENDO"), _("ILLINOIS"), _("SECURITY"), _("REDACTED"),
    _("MUTATION"), _("TERMINAL"), _("CHEMICAL"), _("GARRISON"),
    _("SETTLERS"), _("OVERSEER"), _("CARAVANS"), _("VETERANS"),
    _("MILITARY"), _("DETONATE"), _("FIREWALL"), _("FOOTBALL"),
    _("FREEDOMS"), _("GASOLINE"), _("GENERATE"), _("HARDWARE"),
    _("HIGHWAYS"), _("INDUSTRY"), _("KITCHENS"), _("MARSHALS"),
    _("MEMORIES"), _("MISSIONS"), _("MOUNTAIN"), _("MUTATING"),
    _("NATIONAL"), _("NEIGHBOR"), _("NEWSREEL"), _("OPERATOR"),
    _("PAGEANTS"), _("PAMPHLET"), _("PATRIOTS"), _("PILGRIMS"),
    _("POLITICS"), _("POSTCARD"), _("PRESERVE"), _("PROTOCOL"),
};

static const u8 sTerminalWordsLen9[][10] = {
    _("WASTELAND"), _("DEMOCRACY"), _("RADIATION"), _("BOTTLECAP"),
    _("PLUTONIUM"), _("MILKSHAKE"), _("BREAKFAST"), _("BLUEPRINT"),
    _("MAINFRAME"), _("SANCTUARY"), _("PATRIOTIC"), _("CHEMISTRY"),
    _("OVERSEERS"), _("HEIRLOOMS"), _("OPERATORS"), _("CONTAINER"),
    _("CONTRACTS"), _("DEMOCRATS"), _("DIPLOMACY"), _("DISTILLED"),
    _("DRUMSTICK"), _("ELECTIONS"), _("ELEVATORS"), _("EMERGENCE"),
    _("EMPLOYEES"), _("ENDURANCE"), _("EVERGREEN"), _("EXECUTIVE"),
    _("EXPLOSION"), _("FESTIVITY"), _("FIREFIGHT"), _("FLAGPOLES"),
    _("FLASHCARD"), _("FORTITUDE"), _("FRONTIERS"), _("GALVANIZE"),
    _("GENERATOR"), _("GEOGRAPHY"), _("GOVERNORS"), _("GRADUATES"),
    _("GUARDIANS"), _("GUNPOWDER"), _("HEADLINES"), _("HEIRESSES"),
    _("HISTORIES"), _("HOMEFRONT"), _("HOMESTEAD"), _("SYNTHESIS"),
};

static const u8 sTerminalWordsLen10[][11] = {
    _("CAPITALISM"), _("FEDERATION"), _("EXPERIENCE"), _("APOCALYPSE"),
    _("DETONATION"), _("SUBVERSIVE"), _("TELEVISION"), _("TECHNOLOGY"),
    _("GENERATION"), _("ENCRYPTION"), _("BOBBLEHEAD"),
    _("GOVERNMENT"), _("PROPAGANDA"), _("LABORATORY"), _("MILITARIST"),
    _("NEGOTIATOR"), _("NEIGHBORLY"), _("NEWSCASTER"), _("OBLIGATION"),
    _("OPERATIONS"), _("ORCHESTRAL"), _("PARAMEDICS"), _("PERMISSION"),
    _("PHARMACIST"), _("PHONOGRAPH"), _("PILGRIMAGE"), _("POPULATION"),
    _("PRESIDENTS"), _("PRESERVING"), _("PROFESSORS"), _("PROHIBITED"),
    _("PROJECTILE"), _("PROSPERING"), _("PROTECTION"), _("PROVIDENCE"),
    _("RADIOLOGIC"), _("RAILROADED"), _("REGIMENTAL"), _("REPRESSION"),
    _("RESERVISTS"), _("RESPIRATOR"), _("SCIENTISTS"), _("SEISMOLOGY"),
};

struct TerminalWordPool
{
    const u8 *words;
    u16 count;
    u16 globalIdBase;
    u8 wordLen;
    u8 stride;
};

#define TERMINAL_WORD_BUCKET(arr, len, base) \
    { .words = (const u8 *)(arr), .count = ARRAY_COUNT(arr), .globalIdBase = (base), .wordLen = (len), .stride = (len) + 1 }

static const struct TerminalWordPool sTerminalWordPools[TERMINAL_NUM_LENGTH_BUCKETS] =
{
    [0] = TERMINAL_WORD_BUCKET(sTerminalWordsLen4,   4,   0),
    [1] = TERMINAL_WORD_BUCKET(sTerminalWordsLen5,   5,  58),
    [2] = TERMINAL_WORD_BUCKET(sTerminalWordsLen6,   6, 115),
    [3] = TERMINAL_WORD_BUCKET(sTerminalWordsLen7,   7, 164),
    [4] = TERMINAL_WORD_BUCKET(sTerminalWordsLen8,   8, 215),
    [5] = TERMINAL_WORD_BUCKET(sTerminalWordsLen9,   9, 259),
    [6] = TERMINAL_WORD_BUCKET(sTerminalWordsLen10, 10, 307),
};

#endif // GUARD_DATA_TERMINAL_HACK_H
