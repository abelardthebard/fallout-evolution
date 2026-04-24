# Palette slot contract

**Status:** draft 2 — covers the audited screens (main menu, Birch intro,
terminals, dialogue frames, options menu). Expect revisions as overworld,
battle, and other screens are audited.

**Purpose:** establish a single source of truth for how BG and OBJ palette
slots are allocated across Fallout Evolution, so features can share screen
space without accidental color corruption and the Pip-Boy theme can be applied
consistently everywhere — including live-switching from the options menu at
runtime.

---

## GBA palette budget

The GBA has two palette regions:

| Region | Slots | Colors per slot | Total colors |
| --- | --- | --- | --- |
| BG | 16 | 16 | 256 |
| OBJ | 16 | 16 | 256 |

A palette slot is selected by the 4-bit palette field in a tilemap entry (BG)
or an OAM entry (OBJ). Color index 0 of every slot is always transparent.

Features get a slot by calling
`LoadPalette(src, BG_PLTT_ID(n), size)` or `LoadPalette(src, OBJ_PLTT_ID(n), size)`.
When two features pick the same slot on the same screen, whichever loads
second wins — the first feature's colors are silently erased.

---

## How theming works today

Two mechanisms already exist in the codebase, and the contract assumes both.

### Mechanism 1: `GetActiveTheme()`

Returns the player's current theme (`THEME_GREEN / BLUE / RED / YELLOW`) from
`gSaveBlock2Ptr->optionsWindowFrameType`. The value is **already mutable** —
changing it in memory changes what the function returns immediately. That
means live theme switching is an API concern (every load needs to call it at
load time), not a data-model concern.
Defined: [src/text_window.c:303](../src/text_window.c#L303).

### Mechanism 2: `PipBoy_ApplyThemeToPalettes(bgStart, bgCount, objStart, objCount)`

Scans a range of the palette buffer and **remaps** any color matching an entry
in `gPipBoyGreenRamp` to the corresponding entry in `gPipBoyThemeRamps[theme]`.
Colors not in the green ramp are left alone. Works on _palette buffer indices_
(0..255), not on slot numbers.
Defined: [src/text_window.c:99-132](../src/text_window.c#L99).

The key insight: you don't always need a theme-specific palette file. If your
feature's palette contains the standard Pip-Boy green ramp, calling
`PipBoy_ApplyThemeToPalettes` after the load maps it to the active theme —
that's how the terminal chrome works today. Features still can load per-theme
palette files when they need more control (e.g. the Birch backgrounds use
distinct authored palettes per theme). Both approaches are valid.

---

## Current state — audited screens

### Current BG slots

| Slot | Owner / use | Theme-aware? | Loaded at |
| --- | --- | --- | --- |
| 0 | Per-screen backdrop (Birch bg0, terminal chrome, option menu bg) | varies | per-screen init |
| 0 (+1..8) | Birch spotlight gradient (shares slot 0 with Birch bg0) | YES — `sBirchGradientPals[theme]` | [main_menu.c:1445](../src/main_menu.c#L1445), :2203, :2419, :2635, :2669 |
| 1 | Per-screen secondary / accent | varies | per-screen init |
| 2 | Window frame tiles (Birch, option menu) | YES — `GetWindowFrameTilesPal(theme)` | [main_menu.c:2797](../src/main_menu.c#L2797), [option_menu.c:227](../src/option_menu.c#L227) |
| 3 | **Free** — candidate for shared theme-gradient slot | — | — |
| 4 | **Free** | — | — |
| 5 | **Free** | — | — |
| 6 | **Free** | — | — |
| 7 | Option menu frame (live-reload demo) | YES — `GetWindowFrameTilesPal(sel)` | [option_menu.c:227](../src/option_menu.c#L227), :548, :559 |
| 8-13 | **Free** | — | — |
| 14 | `STD_WINDOW_PALETTE_NUM` — standard dialogue text | YES — `GetActiveThemeTextPal()` | [menu.c:356](../src/menu.c#L356) |
| 15 | `DLG_WINDOW_PALETTE_NUM` — message box text/frame | YES — `GetActiveThemeTextPal()` | [main_menu.c:737](../src/main_menu.c#L737), [terminal_ui.c:99](../src/terminal_ui.c#L99), [menu.c:207](../src/menu.c#L207) |

### Current OBJ slots

| Slot | Owner / use | Notes |
| --- | --- | --- |
| auto | Trainer / player-character sprites | `CreateTrainerSprite()` allocates an OBJ slot from the pool; caller reads `gSprites[spriteId].oam.paletteNum`. |
| _slot_.1..3 | Skin ramp (within whatever slot the trainer sprite got) | `OBJ_PLTT_ID(slot) + 1`, 3 colors from `gSkinPalettes` |
| _slot_.4..6 | Hair ramp (within whatever slot the trainer sprite got) | `OBJ_PLTT_ID(slot) + 4`, 3 colors from `gHairPalettes` |

OBJ slot auto-allocation means we can't reserve a fixed OBJ slot without
reworking how trainer sprites claim palettes. For now the contract treats OBJ
slots as first-come-first-served per screen.

---

## Target state — the contract

### Reserved BG slots

```text
  0 ── per-screen backdrop                    (varies; may be themed via color-map)
  1 ── per-screen secondary                   (varies)
  2 ── window-frame tiles                     (themed — GetWindowFrameTilesPal)
  3 ── PIP-BOY THEME GRADIENT (reserved)      (themed — the 8-color ramp)
  4 ── free
  5 ── free
  6 ── free
  7 ── option-menu live-preview frame         (themed)
  8..13  free
 14 ── standard dialogue text                 (STD_WINDOW_PALETTE_NUM, themed)
 15 ── message-box frame/text                 (DLG_WINDOW_PALETTE_NUM, themed)
```

### Slot 3: the theme backdrop palette

**Reserved for the active screen's 16-color theme backdrop palette.** Every
screen that renders themed backdrop artwork (Birch intro's spotlight +
bg0 region, terminal's content-page chrome, future themed splash screens,
etc.) loads its own authored per-theme 16-color palette into slot 3 and
references `palette = 3` in the tilemap entries that render themed content.

### Sharing the spotlight tilemap across screens -- BG 1 + WIN0

The spotlight tile art + tilemap (`graphics/birch_speech/shadow.4bpp`,
`graphics/birch_speech/map.bin`) is shared between the Birch intro and the
terminal content viewer via `PipBoy_LoadSpotlight(charBase, mapBase)`. Both
screens render it on BG 1.

**Birch** owns its whole screen, so it can show the 32x20 fullscreen
tilemap without clipping.

**Terminal content pages** have their own chrome (BG 2) and text (BG 0);
the 32x20 fullscreen spotlight would opaque-cover the chrome if rendered
as-is. Every zero tiles in Birch's tilemap are non-transparent (verified
by pixel peek), so we can't rely on tile-0 transparency to reveal chrome
around the disc.

The contract for a terminal page that wants the spotlight:

1. Author a `SpotlightLayout` that specifies the WIN0 clip rect (screen
   pixels) and BG 1 scroll offsets (so the Birch disc's native coords
   translate into the clip rect).
2. Call `TerminalUI_ShowSpotlight(&layout)` from the page's
   `createSprites` hook. The helper:
   - Sets WIN0 to the rect.
   - Configures `WININ` so BG 0/1/2/OBJ render inside WIN0.
   - Configures `WINOUT` so BG 0/2/OBJ (no BG 1) render outside WIN0 --
     chrome shows, spotlight is suppressed.
   - Enables `DISPCNT_WIN0_ON`.
   - Sets BG 1 scroll.
3. Call `TerminalUI_HideSpotlight()` on teardown (already wired into
   `TerminalContent_Teardown`, so pages get cleanup for free).

Pages never poke `REG_OFFSET_WIN0H` / `WININ` / `WINOUT` / `DISPCNT`
directly -- that's what `TerminalUI_ShowSpotlight` encapsulates.

**Shared invariant across every feature:** positions 1–8 hold the 8-shade
Pip-Boy gradient (`gPipBoyGradients[theme]`). This is the spine — any tile
whose pixels use color indices 1–8 will render with themed gradient shades
regardless of which screen it appears on. `PipBoy_LoadThemeGradient(BG_PLTT_ID(3))`
loads this 8-color block by itself for features that don't need the full
16-color slot.

**Feature-specific positions:** 0 (usually transparent) and 9–15 are the
screen's own accent colors — an outline shade, a highlight, a local chrome
color. Each screen's `bg0_*.gbapal`-equivalent file authors these alongside
the gradient. The two are loaded in one call: `LoadPalette(screenThemePal[theme], BG_PLTT_ID(3), PLTT_SIZE_4BPP)`.

Only one screen is on at a time, so features don't conflict even though
they "share" slot 3. On screen entry, each feature repopulates slot 3 with
its own palette; on exit, the next screen's setup re-populates it.

### Boot-time application

```c
  LoadPalette(sBgImagePal, BG_PLTT_ID(0), PLTT_SIZE_4BPP);      // per-screen
  PipBoy_LoadThemeGradient(BG_PLTT_ID(3));                      // slot 3 ramp
  LoadPalette(GetActiveThemeTextPal(), BG_PLTT_ID(15), ...);    // text
  PipBoy_ApplyThemeToPalettes(...);                             // color-map
```

`PipBoy_LoadThemeGradient(destPalBase)` doesn't exist yet — it's the minimum
new API needed for this contract. Implementation is a thin wrapper around
`LoadPalette(sBirchGradientPals[GetActiveTheme()][palIdx], destPalBase + 1, PLTT_SIZEOF(8))`.
Since it calls `GetActiveTheme()` internally, every invocation picks up the
current theme — the same function works for both boot-time and live-switch
paths.

### Target OBJ slots

No changes. Trainer sprites continue to claim dynamically. Hair/skin
conventions (indices 1–3 / 4–6 within the assigned slot) stay as they are.

---

## Live theme switching — model

**Pokeemerald-idiomatic CB-driven re-setup. No listener registry.**

Pokeemerald's architecture already gives us live theme switching almost for
free:

- `GetActiveTheme()` reads from `gSaveBlock2Ptr->optionsWindowFrameType` on
  every call, not from a boot-time cache.
- Every CB2 setup reloads its palettes when it runs — if theme-aware loads
  call `GetActiveTheme()` at load time, they pick up whatever the current
  theme is without any additional signalling.
- Returning from one CB to another (options menu → field, field → options
  menu) re-enters the destination CB's setup state machine, which re-runs
  those palette loads.

The live-switch story becomes:

1. Player opens the options menu. Its setup loads its own themed palettes
   from `GetActiveTheme()`.
2. Player changes the theme selection. `gSaveBlock2Ptr->optionsWindowFrameType`
   updates; the options menu reloads its own palette locally to preview the
   change (exactly what [`option_menu.c:548`](../src/option_menu.c#L548) and
   `option_menu.c:559` already do for frame selection).
3. Player exits. Destination CB re-initializes and reads the new theme from
   the save block.

### Feature participation rules

1. **Read theme at CB setup time**, not from boot-time-cached state. Any
   `LoadPalette` call that picks a theme-specific source should call
   `GetActiveTheme()` inline, or call a helper like `PipBoy_LoadThemeGradient`
   that does so internally.
2. **Live preview is a feature's own business.** If your screen needs to show
   a theme change immediately (like the options menu's frame preview), handle
   it locally — call your own reload code from wherever the selection changes.
   This is not a project-wide event.

### Explicitly out of scope

- A global listener registry. Features do not subscribe to a "theme changed"
  event, and no code walks a list of features to update them. This would be a
  non-idiomatic layer on top of pokeemerald's CB-driven flow, and it's
  unnecessary — the CB flow already delivers the same guarantees by
  re-initializing features on screen re-entry.
- Mid-CB theme updates for features that aren't the active screen. By
  definition, only one screen's CB is active at a time; nothing else needs to
  update in response to a theme change.

---

## Adding a new themed feature — checklist

1. Decide which of your palettes should be themed:
   - **Gradient ramp?** Load into slot 3 via `PipBoy_LoadThemeGradient`.
   - **Text / frame?** Use slot 14 or 15 and `GetActiveThemeTextPal()`.
   - **Window-frame tiles?** Use slot 2 and `GetWindowFrameTilesPal(GetActiveTheme())->pal`.
   - **Color-mapped chrome** (your palette contains the green ramp)? After
     `LoadPalette`, call `PipBoy_ApplyThemeToPalettes` on the affected index
     range.
2. Pick remaining slots from the free pool (3 is reserved, 4–6 and 8–13 are
   open). Document the choice in a comment next to the `LoadPalette` call.
3. Ensure every theme-aware load runs inside your CB2 setup state machine
   (not at file scope, not cached in a boot-time global). This is what makes
   the feature pick up theme changes on re-entry.
4. If your screen has a local live-preview concern (user-facing control that
   needs to show the theme change without leaving the screen), call your own
   reload code from the control handler.

---

## Migration plan

**Phase 0 — contract doc (this file).** In review.

**Phase 1 — stand up `src/pipboy_theme.c` and `include/pipboy_theme.h`.**
Introduce `PipBoy_LoadThemeGradient(destPalBase)`. Migrate the existing theme
functions (`GetActiveTheme`, `GetActiveThemeTextPal`, `GetActiveThemeFramePal`,
`PipBoy_ApplyThemeToPalettes`) and the `gPipBoy*Ramp` / `sBirchGradientPals`
tables out of `text_window.c` and `main_menu.c` into the new module.
`THEME_GREEN`/`BLUE`/`RED`/`YELLOW` enum moves to
`include/constants/pipboy_theme.h`. All current callers update their
includes. No visual change.

**Phase 2 — migrate the Birch spotlight off slot 0 onto slot 3.** Two touch
points: re-author the spotlight source tilemap (`map.bin` that compiles to
`map.bin.smolTM`) so entries reference palette slot 3, and move the
`LoadPalette` call to consume `PipBoy_LoadThemeGradient(BG_PLTT_ID(3))`.
Birch intro stays visually identical; internally it consumes the new helper.

**Phase 3 — terminal content page renders the spotlight.** BG 2 carries the
spotlight tilemap; slot 3 carries the gradient via the shared helper. Player
sprite OBJ draws on top. This is the visible milestone.

**Phase 4 — extend to other UI surfaces.** Dialogue frames already use
`GetActiveThemeTextPal`; they're compliant as long as their slot assignments
(14 / 15) match the contract. Any custom dialogue UI that steps outside the
contract needs porting.

**Phase 5 — wire the options menu's theme selector.** Options menu already
previews frame changes live. Extend the same local-reload pattern to the
gradient / text palettes so switching theme while in the options menu updates
the whole sample screen. No registry needed — the menu owns the reload path.

---

## Known gaps

- Overworld, battle, and per-map palette usage not audited. Adding these
  could discover additional theme hooks or collisions in the free-slot range.
- OBJ slot 0 vs. auto-allocation — if we ever want a themed OBJ gradient (a
  themed UI-sprite glow, for instance), we'll need a reserved OBJ slot. Not
  required yet.
- `sBirchBg0Pals` / `sBirchBg1Pals` are Birch-specific theme artwork, not
  part of the gradient contract. They stay with the Birch intro as-is.

---

## Decisions locked in

These were the four open questions in draft 1; resolved by project standards
and pokeemerald-expansion idioms rather than personal preference.

### 1. Project-specific design docs location

**`design/` at repo root.** Pokeemerald and expansion keep their upstream
mdBook at `docs/`; project-specific architecture docs belong in their own
top-level folder so they survive upstream merges and don't pollute the
inherited book. `design/` parallels `src/`, `include/`, `data/`, `tools/` as
top-level categorical directories. The README should link to it.

### 2. Module home for theme functions

**New `src/pipboy_theme.c` + `include/pipboy_theme.h`, plus
`include/constants/pipboy_theme.h` for the enum.** Pokeemerald's
one-system-per-file convention is explicit (see `palette.c`,
`scanline_effect.c`, `text_window.c` each owning a single concern). The
Pip-Boy theme system currently living partly in `text_window.c` and partly in
`main_menu.c` is a historical wart; the refactor consolidates it. Migration
is part of Phase 1.

### 3. Listener registry

**None.** Pokeemerald is CB-driven, not event-driven. Screens re-query theme
at CB setup; local live-preview is a feature-local concern. A registry would
be a non-idiomatic layer on top of the existing flow — see the
"Live theme switching — model" section above.

### 4. Birch spotlight tilemap migration

**Re-author the source tilemap.** The source `map.bin` (which compiles to
`map.bin.smolTM`) gets its palette bits updated to reference slot 3. No
runtime patching — runtime bit-twiddling on compiled asset data is a lazy
bandaid that hides the real change from git diffs and asset tooling.
Phase 2 handles this.
