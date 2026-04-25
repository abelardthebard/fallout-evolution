#ifndef GUARD_DATA_TERMINAL_HACK_H
#define GUARD_DATA_TERMINAL_HACK_H

#include "constants/terminal_hack.h"

struct TerminalDifficulty
{
    u8 wordLenMin;
    u8 wordLenMax;
    u8 wordCount;
    u8 lockoutSteps;
    u8 bracketPairs;
};

// #defines (not just initializers) so terminal_hack.c can use them in
// STATIC_ASSERTs. Higher tiers = shorter words, more of them: weaker
// likeness signal, less dud-removal leverage.
#define TIER_1_WORD_LEN_MIN     7
#define TIER_1_WORD_LEN_MAX     8
#define TIER_1_WORD_COUNT      10
#define TIER_1_LOCKOUT_STEPS   50
#define TIER_1_BRACKET_PAIRS    4

#define TIER_2_WORD_LEN_MIN     5
#define TIER_2_WORD_LEN_MAX     7
#define TIER_2_WORD_COUNT      12
#define TIER_2_LOCKOUT_STEPS  125
#define TIER_2_BRACKET_PAIRS    4

#define TIER_3_WORD_LEN_MIN     4
#define TIER_3_WORD_LEN_MAX     5
#define TIER_3_WORD_COUNT      14
#define TIER_3_LOCKOUT_STEPS  200
#define TIER_3_BRACKET_PAIRS    4

static const struct TerminalDifficulty sTerminalDifficulty[NUM_TERMINAL_TIERS] =
{
    [TERMINAL_TIER_1] = {
        .wordLenMin   = TIER_1_WORD_LEN_MIN,
        .wordLenMax   = TIER_1_WORD_LEN_MAX,
        .wordCount    = TIER_1_WORD_COUNT,
        .lockoutSteps = TIER_1_LOCKOUT_STEPS,
        .bracketPairs = TIER_1_BRACKET_PAIRS,
    },
    [TERMINAL_TIER_2] = {
        .wordLenMin   = TIER_2_WORD_LEN_MIN,
        .wordLenMax   = TIER_2_WORD_LEN_MAX,
        .wordCount    = TIER_2_WORD_COUNT,
        .lockoutSteps = TIER_2_LOCKOUT_STEPS,
        .bracketPairs = TIER_2_BRACKET_PAIRS,
    },
    [TERMINAL_TIER_3] = {
        .wordLenMin   = TIER_3_WORD_LEN_MIN,
        .wordLenMax   = TIER_3_WORD_LEN_MAX,
        .wordCount    = TIER_3_WORD_COUNT,
        .lockoutSteps = TIER_3_LOCKOUT_STEPS,
        .bracketPairs = TIER_3_BRACKET_PAIRS,
    },
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
    _("THAT"), _("WITH"), _("THIS"), _("HAVE"),
    _("FROM"), _("YOUR"), _("THEY"), _("WILL"),
    _("JUST"), _("LIKE"), _("WHAT"), _("WHEN"),
    _("MORE"), _("WERE"), _("TIME"), _("BEEN"),
    _("SOME"), _("ALSO"), _("THEM"), _("THAN"),
    _("GOOD"), _("ONLY"), _("INTO"), _("KNOW"),
    _("MAKE"), _("OVER"), _("THEN"), _("BACK"),
    _("WANT"), _("WELL"), _("SAID"), _("MOST"),
    _("MUCH"), _("VERY"), _("EVEN"), _("HERE"),
    _("NEED"), _("WORK"), _("YEAR"), _("MADE"),
    _("TAKE"), _("MANY"), _("LIFE"), _("DOWN"),
    _("LAST"), _("BEST"), _("SUCH"), _("LOVE"),
    _("HOME"), _("LONG"), _("LOOK"), _("SAME"),
    _("USED"), _("BOTH"), _("COME"), _("PART"),
    _("FIND"), _("HELP"), _("HIGH"), _("DOES"),
    _("GIVE"), _("NEXT"), _("EACH"), _("MUST"),
    _("SHOW"), _("FEEL"), _("SURE"), _("TEAM"),
    _("EVER"), _("KEEP"), _("FREE"), _("AWAY"),
    _("LEFT"), _("CITY"), _("JUNE"), _("JULY"),
    _("DAYS"), _("NAME"), _("PLAY"), _("REAL"),
    _("DONE"), _("CARE"), _("WEEK"), _("CASE"),
    _("FULL"), _("LIVE"), _("READ"), _("TOLD"),
    _("FOUR"), _("HARD"), _("MEAN"), _("ONCE"),
    _("TELL"), _("SEEN"), _("STOP"), _("CALL"),
    _("HEAD"), _("TOOK"), _("CAME"), _("SIDE"),
    _("WENT"), _("LESS"), _("LINE"), _("SAYS"),
    _("AREA"), _("FACE"), _("FIVE"), _("KIND"),
    _("HOPE"), _("ABLE"), _("BOOK"), _("POST"),
    _("TALK"), _("FACT"), _("GUYS"), _("HALF"),
    _("HAND"), _("MIND"), _("BODY"), _("TRUE"),
    _("LOST"), _("ROOM"), _("ELSE"), _("GIRL"),
    _("NICE"), _("YEAH"), _("IDEA"), _("PAST"),
    _("MOVE"), _("WAIT"), _("DATA"), _("LATE"),
    _("STAY"), _("DEAL"), _("SOON"), _("TURN"),
    _("FORM"), _("EASY"), _("NEAR"), _("PLAN"),
    _("WEST"), _("KIDS"),
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
    _("ABOUT"), _("THEIR"), _("THERE"), _("WHICH"),
    _("WOULD"), _("OTHER"), _("AFTER"), _("FIRST"),
    _("THINK"), _("COULD"), _("THESE"), _("WHERE"),
    _("RIGHT"), _("YEARS"), _("BEING"), _("GOING"),
    _("STILL"), _("NEVER"), _("THOSE"), _("WORLD"),
    _("GREAT"), _("WHILE"), _("EVERY"), _("STATE"),
    _("THREE"), _("SINCE"), _("UNDER"), _("THING"),
    _("HOUSE"), _("PLACE"), _("AGAIN"), _("FOUND"),
    _("MIGHT"), _("MONEY"), _("NIGHT"), _("UNTIL"),
    _("DOING"), _("GROUP"), _("WOMEN"), _("START"),
    _("TODAY"), _("POINT"), _("MUSIC"), _("BASED"),
    _("SMALL"), _("WHITE"), _("LATER"), _("ORDER"),
    _("PARTY"), _("THANK"), _("USING"), _("BLACK"),
    _("WHOLE"), _("MAYBE"), _("STORY"), _("GAMES"),
    _("LEAST"), _("MEANS"), _("EARLY"), _("LOCAL"),
    _("VIDEO"), _("YOUNG"), _("COURT"), _("GIVEN"),
    _("LEVEL"), _("OFTEN"), _("DEATH"), _("HOURS"),
    _("SOUTH"), _("KNOWN"), _("LARGE"), _("WRONG"),
    _("ALONG"), _("MARCH"), _("APRIL"), _("WITCH"),
    _("NEEDS"), _("CLASS"), _("CLOSE"), _("COMES"),
    _("LOOKS"), _("CAUSE"), _("HAPPY"), _("WOMAN"),
    _("LEAVE"), _("NORTH"), _("WATCH"), _("LIGHT"),
    _("SHORT"), _("TAKEN"), _("THIRD"), _("AMONG"),
    _("CHECK"), _("HEART"), _("ASKED"), _("CHILD"),
    _("MAJOR"), _("QUITE"), _("FINAL"), _("FRONT"),
    _("READY"), _("BRING"), _("HEARD"), _("STUDY"),
    _("CLEAR"), _("MONTH"), _("WORDS"), _("BOARD"),
    _("FIELD"), _("SEEMS"), _("FIGHT"), _("FORCE"),
    _("ISSUE"), _("SHOWS"), _("SPACE"), _("TOTAL"),
    _("ABOVE"), _("SHARE"), _("SENSE"), _("WEEKS"),
    _("BREAK"), _("EVENT"), _("SORRY"), _("GIRLS"),
    _("GUESS"), _("LEARN"), _("ADDED"), _("ALONE"),
    _("HANDS"), _("MOVIE"), _("PRESS"), _("TRIED"),
    _("WORTH"), _("AREAS"), _("BOOKS"), _("SOUND"),
    _("VALUE"), _("LIVES"), _("ROUND"), _("STAND"),
    _("STUFF"), _("DRIVE"), _("PHONE"),
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
    _("PEOPLE"), _("SHOULD"), _("REALLY"), _("BEFORE"),
    _("AROUND"), _("ALWAYS"), _("BETTER"), _("LITTLE"),
    _("THINGS"), _("DURING"), _("SCHOOL"), _("FAMILY"),
    _("PLEASE"), _("SECOND"), _("NUMBER"), _("CALLED"),
    _("HAVING"), _("PUBLIC"), _("SYSTEM"), _("PERSON"),
    _("CHANGE"), _("ENOUGH"), _("MAKING"), _("STATES"),
    _("THOUGH"), _("SEASON"), _("TRYING"), _("UNITED"),
    _("COURSE"), _("HEALTH"), _("WITHIN"), _("THANKS"),
    _("SOCIAL"), _("SINGLE"), _("BECOME"), _("COMING"),
    _("OFFICE"), _("ALMOST"), _("TAKING"), _("ANYONE"),
    _("MATTER"), _("PRETTY"), _("FRIEND"), _("SAYING"),
    _("MONTHS"), _("SERIES"), _("EITHER"), _("POLICE"),
    _("RATHER"), _("REASON"), _("REPORT"), _("MYSELF"),
    _("LIVING"), _("BEHIND"), _("MARKET"), _("FORMER"),
    _("STREET"), _("CHANCE"), _("FATHER"), _("ACROSS"),
    _("ACTION"), _("MOMENT"), _("MOTHER"), _("PLAYED"),
    _("POINTS"), _("SUMMER"), _("KILLED"), _("STRONG"),
    _("PERIOD"), _("RECORD"), _("COMMON"), _("LIKELY"),
    _("AUGUST"), _("MAYDAY"), _("MARVEL"), _("BATTLE"),
    _("WINTER"), _("AUTUMN"), _("SPRING"),
    _("CENTER"), _("COUNTY"), _("COUPLE"), _("HAPPEN"),
    _("INSIDE"), _("ISSUES"), _("PLAYER"), _("RETURN"),
    _("HIGHER"), _("MEMBER"), _("MIDDLE"), _("RESULT"),
    _("ANSWER"), _("DESIGN"), _("POLICY"), _("CHURCH"),
    _("LONGER"), _("BECAME"), _("GIVING"), _("SOURCE"),
    _("FOLLOW"), _("AMOUNT"), _("LEAGUE"), _("GROUPS"),
    _("REVIEW"), _("CANNOT"), _("ITSELF"), _("ATTACK"),
    _("ENTIRE"), _("TURNED"), _("CHOICE"), _("EVENTS"),
    _("SIMPLE"), _("SIMPLY"), _("CAREER"), _("FIGURE"),
    _("MODERN"), _("FORGET"), _("LISTEN"), _("ACCESS"),
    _("RECENT"), _("SEEING"), _("GROWTH"), _("CHARGE"),
    _("CREATE"), _("EFFECT"), _("EXCEPT"), _("MOVING"),
    _("WEIGHT"), _("DOUBLE"), _("EXPECT"), _("ISLAND"),
    _("CREDIT"), _("FEMALE"), _("NEARLY"), _("REGION"),
    _("TRAVEL"), _("BEYOND"), _("FORCES"), _("MINUTE"),
    _("NATURE"), _("UNLESS"), _("INCOME"), _("LEVELS"),
    _("POSTED"), _("SAFETY"), _("SOUNDS"), _("ASKING"),
    _("SEARCH"), _("AUTHOR"), _("GLOBAL"), _("SILENT"),
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
    _("BECAUSE"), _("THROUGH"), _("BETWEEN"), _("ANOTHER"),
    _("WITHOUT"), _("AGAINST"), _("SOMEONE"), _("THOUGHT"),
    _("HOWEVER"), _("GETTING"), _("LOOKING"), _("ALREADY"),
    _("NOTHING"), _("SUPPORT"), _("BELIEVE"), _("SERVICE"),
    _("COUNTRY"), _("GENERAL"), _("WORKING"), _("FRIENDS"),
    _("CONTROL"), _("PROBLEM"), _("HISTORY"), _("SEVERAL"),
    _("STARTED"), _("PLAYING"), _("MEMBERS"), _("SPECIAL"),
    _("MILLION"), _("MORNING"), _("WHETHER"), _("FURTHER"),
    _("MINUTES"), _("PLAYERS"), _("TALKING"), _("COLLEGE"),
    _("CURRENT"), _("EXAMPLE"), _("PROCESS"), _("HIMSELF"),
    _("OUTSIDE"), _("INSTEAD"), _("RESULTS"), _("RUNNING"),
    _("ACCOUNT"), _("INCLUDE"), _("PARENTS"), _("SIMILAR"),
    _("PERFECT"), _("ENGLISH"), _("PRIVATE"), _("PRESENT"),
    _("FINALLY"), _("SOCIETY"), _("AVERAGE"), _("BROUGHT"),
    _("CERTAIN"), _("MEDICAL"), _("EXACTLY"), _("MEETING"),
    _("PROVIDE"), _("USUALLY"), _("READING"), _("FEDERAL"),
    _("FEELING"), _("PICTURE"), _("CENTRAL"), _("CHANGES"),
    _("FORWARD"), _("JANUARY"), _("OCTOBER"), _("TRAINER"),
    _("EDUCATE"),
    _("SCIENCE"), _("VARIOUS"), _("BROTHER"), _("NATURAL"),
    _("QUALITY"), _("AMAZING"), _("RELATED"), _("SERIOUS"),
    _("ARTICLE"), _("DECIDED"), _("PERHAPS"), _("RELEASE"),
    _("WRITTEN"), _("COUNCIL"), _("FOREIGN"), _("CHANGED"),
    _("POPULAR"), _("SYSTEMS"), _("VERSION"), _("WRITING"),
    _("SUCCESS"), _("TOWARDS"), _("WAITING"), _("CREATED"),
    _("MISSING"), _("PERCENT"), _("ALLOWED"), _("CULTURE"),
    _("MARRIED"), _("OFFICER"), _("RESPECT"), _("TONIGHT"),
    _("CENTURY"), _("LIMITED"), _("NETWORK"), _("STUDENT"),
    _("CAPITAL"), _("STATION"), _("WESTERN"), _("CONTENT"),
    _("DESPITE"), _("HUSBAND"), _("LEADING"), _("MESSAGE"),
    _("QUICKLY"), _("SECTION"), _("CONTACT"), _("WELCOME"),
    _("EARLIER"), _("LEAVING"), _("STUDIES"), _("WINNING"),
    _("EPISODE"), _("JUSTICE"), _("MANAGER"), _("ABILITY"),
    _("CALLING"), _("HAPPENS"), _("SUBJECT"), _("PRODUCT"),
    _("REGULAR"), _("STORIES"), _("WORKERS"), _("ANYMORE"),
    _("GROWING"), _("OPENING"), _("OPINION"), _("BIGGEST"),
    _("DEFENSE"), _("REASONS"), _("WEEKEND"), _("AWESOME"),
    _("CLEARLY"), _("IMAGINE"), _("PROTECT"), _("TELLING"),
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
    _("BUSINESS"), _("ANYTHING"), _("ACTUALLY"), _("AMERICAN"),
    _("CHILDREN"), _("EVERYONE"), _("TOGETHER"), _("RESEARCH"),
    _("REMEMBER"), _("PROBABLY"), _("POSSIBLE"), _("QUESTION"),
    _("SERVICES"), _("YOURSELF"), _("ALTHOUGH"), _("BUILDING"),
    _("STUDENTS"), _("THINKING"), _("POSITION"), _("HAPPENED"),
    _("PERSONAL"), _("PROBLEMS"), _("TRAINING"), _("INTEREST"),
    _("ORIGINAL"), _("RECEIVED"), _("DIRECTOR"), _("EVIDENCE"),
    _("OFFICIAL"), _("WHATEVER"), _("PROPERTY"), _("COMPLETE"),
    _("ECONOMIC"), _("INVOLVED"), _("LANGUAGE"), _("NOVEMBER"),
    _("DECISION"), _("CONTINUE"), _("ELECTION"), _("INCREASE"),
    _("DAUGHTER"), _("DECEMBER"), _("HOSPITAL"), _("STARTING"),
    _("PRACTICE"), _("FOLLOWED"), _("RELEASED"), _("DISTRICT"),
    _("MINISTER"), _("PRODUCTS"), _("STRAIGHT"), _("FEBRUARY"),
    _("INCLUDED"), _("RESPONSE"), _("SPECIFIC"), _("STANDARD"),
    _("PROVIDED"), _("RECENTLY"), _("REQUIRED"), _("TOMORROW"),
    _("WATCHING"), _("ADDITION"), _("PRESSURE"), _("CAMPAIGN"),
    _("PREVIOUS"), _("REPORTED"), _("CONSIDER"), _("POSITIVE"),
    _("FAVORITE"), _("MOVEMENT"), _("DESIGNED"), _("EXPECTED"),
    _("INCLUDES"), _("MATERIAL"), _("CONTRACT"), _("FAMILIES"),
    _("FEATURES"), _("FINISHED"), _("MAJORITY"), _("PHYSICAL"),
    _("APPROACH"), _("COMPARED"), _("LEARNING"), _("MULTIPLE"),
    _("PICTURES"), _("ANALYSIS"), _("MARRIAGE"), _("PATIENTS"),
    _("SPEAKING"), _("SUPPOSED"), _("CONGRESS"), _("DIRECTLY"),
    _("PLANNING"), _("PROGRAMS"), _("COMMENTS"), _("OFFICERS"),
    _("PRODUCED"), _("ACTIVITY"), _("DIVISION"), _("LOCATION"),
    _("STANDING"), _("DISTANCE"), _("EXCHANGE"), _("NORTHERN"),
    _("POWERFUL"), _("BENEFITS"), _("SOLUTION"), _("SOUTHERN"),
    _("APPEARED"), _("CRITICAL"), _("SEPARATE"), _("RETURNED"),
    _("BECOMING"), _("PROJECTS"), _("CHAIRMAN"), _("PROVIDES"),
    _("SOMEBODY"), _("STRENGTH"), _("GREATEST"), _("NEGATIVE"),
    _("FUNCTION"), _("PROGRESS"), _("SHOOTING"), _("POSSIBLY"),
    _("BIRTHDAY"), _("CHANGING"), _("BELIEVED"), _("CRIMINAL"),
    _("CULTURAL"), _("EXISTING"), _("SLIGHTLY"), _("SURPRISE"),
    _("VIOLENCE"), _("BREAKING"), _("MAGAZINE"), _("PREPARED"),
    _("RELIGION"), _("STRATEGY"), _("TEACHERS"), _("ACCOUNTS"),
    _("AUDIENCE"), _("MEDICINE"), _("PRESENCE"), _("REACTION"),
    _("CITIZENS"), _("ENTIRELY"), _("FESTIVAL"), _("REGIONAL"),
    _("TEACHING"), _("TRANSFER"), _("ACCIDENT"), _("ADVANCED"),
    _("ANYWHERE"), _("ARTICLES"), _("BRINGING"), _("CAPACITY"),
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

// Bumping a bucket's count shifts every later bucket's globalIdBase,
// which invalidates burn-flags in existing saves. Plan a save migration
// if word counts change post-ship.
static const struct TerminalWordPool sTerminalWordPools[TERMINAL_NUM_LENGTH_BUCKETS] =
{
    [0] = TERMINAL_WORD_BUCKET(sTerminalWordsLen4,   4,   0),
    [1] = TERMINAL_WORD_BUCKET(sTerminalWordsLen5,   5, 200),
    [2] = TERMINAL_WORD_BUCKET(sTerminalWordsLen6,   6, 400),
    [3] = TERMINAL_WORD_BUCKET(sTerminalWordsLen7,   7, 600),
    [4] = TERMINAL_WORD_BUCKET(sTerminalWordsLen8,   8, 800),
};

#endif // GUARD_DATA_TERMINAL_HACK_H
