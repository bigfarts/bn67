# Disassembly notes

All addresses are GBA ROM addresses unless identified as file offsets. The
port was translated from the supplied English BN5 ROM rather than from the
unrelated dormant effects originally mistaken for SearchMan in BN6.

## DarkAura

BN3 Blue's DarkAura is chip `0x135`. Its 32-byte record stores code A, 55 MB,
Giga class, barrier kind `8`, and a `0x0BB8` (3,000-frame) lifespan. Barrier
kind `8` is BN3's 300-threshold dark aura; attacks below 300 are rejected and
an attack at or above 300 breaks it.

The BN6 replacement occupies Falzar's BugDthTh slot `0x136` and uses the
native barrier-chip attack at family/subfamily `0x15/0x04`, with its MB cost
set to 61. Barrier parameter
`0x0F` selects BN6's enemy-positioned null-aura visual by default. The visual
hooks at `0x080E1E52` Gregar / `0x080E0B16` Falzar redirect only the marked
DarkAura instance to BN3 Blue's original centered archive, animation, and dark
battle palette. The extractor maps BN6's steady/flash selectors `2/3` to BN3's
centered player animations `0/1`, then terminates every numeric OAM list in
animations `0` through `3` before the pieces that form BN3's embedded 300 label.
Ordinary users of parameter `0x0F` retain their native visual,
100-damage threshold, and indefinite duration.

The registry compiler allocates this hook's long-branch relay, along with every
other section-patch relay, inside `0x0802CCB4` through `0x0802CD5B`. Those bytes
are the original family-`0x15` dispatch table; the compiler relocates the whole
table and rewrites all of its references before reusing that retired table
storage. Feature code no longer supplies relay addresses. In particular, the
family-`0x15`/subfamily-`0x26` launchers at `0x080EAADC` Gregar /
`0x080E979C` Falzar remain native: HubBatch and several related records dispatch
through them without the section hook's saved stack word.

BN6 stores its active aura threshold in collision byte `+0x17`, so 300 cannot
be represented directly. After the barrier controller calls the native setter
at `0x0801A7CC`, the DarkAura-only hook at `0x080E4E50` Gregar /
`0x080E3B10` Falzar writes sentinel `0xFF` and duration `0x0BB8` to the user's
collision data. The shared comparison at `0x0801A948` expands only barrier
type `0x0F` with that sentinel to 300. Every other barrier and aura follows the
unchanged byte threshold and native duration path.

## VarSword ElementSonic

BN6's VarSword action begins at `0x080F096E` in Gregar and `0x080EF62E` in
Falzar. Its native initialization selects one of two five-pointer command
tables according to the user's facing direction, and the shared matcher at
`0x08012792` advances one byte of command progress for every candidate. The
patch repoints that table to two six-pointer copies, changes the per-side stride
and matcher span from 20 to 24 bytes, and puts BN3's
`B, B, Back, Down, Up` sequence first. Putting the new command first assigns it
action-work progress byte `+0x30`, which is already zeroed by the native
VarSword initializer.
For the opposite-facing player, only the horizontal Back input is mirrored.

Both ElementSonic and ordinary SonicBoom select BN6's hidden `SonicBom` action
record `0x173`; their command-progress bytes distinguish them without consuming
or modifying another hidden record. SonicBoom's init at `0x080F0CD0` Gregar /
`0x080EF990` Falzar retains a repeat count of one for the native command and
sets four only after all five ElementSonic inputs have completed. The native
21-frame arm/wave cycle decrements that count and restarts itself, preserving
BN6's animation, movement, and collision timing.

Immediately before each shell-`0x58` spawn, the remaining counts `4, 3, 2, 1`
are translated to BN6 attack parameters `1, 3, 4, 2`.
Shell `0x58` maps those values to its Fire, Elec, Wood, and Aqua
palettes and passes the same parameter through native hit processing, so each
wave receives the matching damage element and weakness interaction. Damage and
attack bonus remain the values cached from the activating VarSwrd chip; all
four waves therefore use its 160 base power plus the same activation bonus.
The first three created shells receive a private marker in unused object byte
`+0x0F`; shell `0x58` converts that marker to hit modifier `1` (stagger only)
instead of its native modifier `3` (stagger plus fixed flashing
invulnerability). The final Aqua wave is unmarked and retains modifier `3`, as
does every native shell-`0x58` caller.

## Native BN5 attack chain

The BN5 SearchMan time-freeze wrapper is at `0x080C1458`. It creates type-1
object `0x31`; the object's main routine begins at `0x080C11D8` and its init
begins at `0x080C11F8`. The actor loads sprite group/index `0x08/0x03`.

The actor creates two cooperating native objects:

| Role | BN5 class/id | Main | Init | Spawn helper |
| --- | --- | ---: | ---: | ---: |
| moving scope | type 4 / `0x43` | `0x080E5208` | `0x080E5228` | `0x080E547C` |
| shot hit | type 3 / `0x57` | `0x080D0D78` | `0x080D0D98` | `0x080D0E1E` |

The reticle uses group `0x10`, index `0x23` normally and index `0x22` for its
alternate form. It scans the opponent's block region, accepts A to lock, and
sets the actor's delete-command byte when B accompanies A or is pressed during
the short window after the lock.

The firing loop at `0x080C136C`-`0x080C13CC` initializes a count of five. Each
shot is spawned when its ten-frame timer reaches seven. The special flag is
set only when both the delete-command byte is nonzero and the remaining-shot
count is one. Therefore the **fifth spawned shot** is the only possible delete
shot. Normal shots use passive hit type `25`; the special shot uses type
`29`. In BN6's hit-type table, type `25` resolves to flags `0x8000008C`
or `0x4000008C` according to owner side, while type `29` resolves to
`0x8000009C` or `0x4000009C`. The additional `0x10` hit-property bit is
what carries the delete-shot behavior.

Separately, hit init maps hit type `25` to hit-effect visual `0` (normal
impact) and type `29` to visual `9` (the chip-delete ping/slash). Despite that
semantic name, hit effect `9` does not itself delete traps: it chooses the
matching contact animation, while hit type `29` supplies the extra
`0x10` delete-property bit. The hit object removes hit, spawns that
visual after contact or creates the miss visual, clears/frees its hit
data, and frees itself in that same update. The full BN6 hit-effect visual
table is documented in `src/README.md`.

BN6's shared player response at `0x0801A2CC` normally clears the active chip
ID and advances the loaded-chip cursor once. The patch redirects that routine
to `chip_delete_entire_hand_main`, which advances the cursor to the terminating
`0xFFFF` entry instead. Consequently, any landed hit with the `0x10` property
discards the complete remaining hand; non-player objects retain the native
active-chip and loaded-count cleanup.

## BN6 hooks and translation

The port installs all three cooperating object classes because BN6 does not
contain a usable SearchMan actor/reticle/hit set.

| Dispatch source | BN6 file offset | Patched target |
| --- | ---: | ---: |
| family `0x1B` table pointer | `0x017BBC` | relocated 256-entry attack table |
| class-1 table pointer | `0x003224` | relocated 256-entry object table |
| class-3 table pointer | `0x00322C` | relocated 256-entry object table |
| class-4 table pointer | `0x003230` | relocated 256-entry object table |

Every imported object keeps its native BN6 class and receives a real 8-bit ID
in that class's relocated table. IDs are assigned from dependency and
declaration order; package code passes `BN67_OBJ_ID(...)` to the native spawn
helper rather than storing a second discriminator in object memory. The native
prefixes, including released HeatMan `0x30` and LifeSync `0x5C`, remain intact.
FolderBack owns a fixed section patch at the shared dispatcher after it resolves
an entry. Its target substitutes the freeze wrapper only for class 1 and
otherwise invokes that resolved entry directly; it does not duplicate a class
table full of wrapper pointers.

Status Bug keeps BN6's native selector and synchronized RNG path at
`0x08013E7E`. Its normal- and high-severity green-invulnerability dispatch
entries at `0x08013EA4` and `0x08013EB4` instead point to the native flashing
handlers at `0x08013EC3` and `0x08013EF1`. Those handlers write the 300- or
600-frame duration to hit state `+0x24` and enable modifier 2, which alternates
the Navi between visible and invisible. The adjacent `0x08013EA8` and
`0x08013EB8` entries remain on their native handlers. This removes green
invulnerability without replacing executable selection logic during battle
startup.

Uninstall's `0x2000` damage flag becomes native inflicted-effect `0xF8`, whose
player response calls the shared removal routine at `0x080140EE`. SunMoon's
BlueMoon hit carries the same flag and reaches that routine as well. Its
successful tail call at `0x0801414A` is redirected through
the nearby veneer at `0x080141BC` and then `status_guard_uninstall_main`, which
clears battle Navi property `0x52` before continuing to the displaced native
refresh routine at `0x0801469C`. The hook is after the native Link Navi guard,
so it does not broaden Uninstall's target eligibility. BN6's status gate checks
active transformation `0x2C == 7` independently of property `0x52`; clearing
the property therefore strips the underlying StatGrd program without removing
Tomahawk Cross's innate status immunity. Cross Out exposes the cleared base
property.

FolderBack's Folder restore runs only on the chip owner's local peer. Its local
hand reset clears that owner's two selection-work counters and six loaded-chip
IDs. The rest of the `0x50`-byte selection-work block is Custom-screen state;
zeroing it makes later chips render blank. The native initializer at
`0x0800A954` clears both players' blocks; calling it from this local-only path
erases the opponent's retained chips on just one peer and desynchronizes the
battle when the opponent next uses one. FolderBack's persistent controller also
exists only on its owner's core, so clearing the Navi object's loaded-chip count
from that controller cannot update the remote core. The shared object dispatcher
therefore watches the remote hand cursor: when it advances past FolderBack, the
dispatcher exhausts the remaining loaded IDs and invalidates the remote Navi's
active chip and loaded count. Leaving the remote cursor populated lets its next
A press replay the final chip that had been queued before FolderBack.
The restore calls BN6's Folder setup at `0x0800A318`, which already shuffles the
rebuilt 30-chip queue. It does not call the shuffle routine a second time;
doing so changes both the restored draw order and the shared RNG stream twice.
The native setup retains the equipped Regular and Tag designations and pins
any still-available designated chips to their privileged positions.
BN6 also keeps the number of Standard, Mega, and Giga chips selected during the
battle at `0x020367E0`. `BattleCheckChip` at `0x0800AFBA` rejects a Mega or
Giga once its accumulated count exceeds the Navi's class limit; it substitutes
the dummy chip ID, which renders as a blank Custom slot. Native Folder setup
does not reset these totals. FolderBack therefore clears all three of the
owner's class totals after rebuilding the Folder. The Standard total is not an
overall class limit but is reset with the restored Folder for consistent usage
bookkeeping; the opponent's totals remain untouched on that peer.

The native final-turn predicate at `0x0800A97A` first requires link-battle
effect flag `0x08`, then tests the battle-state turn byte at `+0x07` against
`0x0F`. FolderBack uses that predicate to keep its rumble, shake, and flash on
turn 15 while suppressing its Folder restore, Custom Gauge writes, and Custom
transition so the native Damage Judge remains authoritative.

BN67 runtime code and imported assets are allocated from file offset `0x800000`
onward in an expanded 16 MiB image; exact addresses are selected by Armips.
The object state machine, timers, reticle movement/input, five-shot loop, and
hit-effect selection follow BN5 state for state. The intentional engine
adaptations are:

- BN5 hit result flags are at hit data `+0x68`; BN6 uses `+0x70`.
- Calls from the expanded ROM are outside Thumb `BL` range and go through a
  long-call macro. The macro restores `r4` before entering BN6 because object
  spawners consume `r4` as an implicit argument.
- The miss visual's type-4 spawner replaces `r5` with the spawned effect. The
  port preserves the original shot-object `r5` across that call so cleanup
  always frees the shot rather than the effect.
- After using `r2` as the normal/final-shot selector, the actor copies its
  translated one-hot Cursor value (`0x40`, originating from chip-table element
  `6`) into every hit object's `+0x0E` byte. The runtime hit test confirms
  that all five 20-damage base pulses still resolve with this value; writing
  numeric table element `6` here is the invalid form.
- Base, EX, and SP all select actor palette `0`, giving every summoned
  SearchMan the same in-battle colors while leaving menu-art palettes distinct.

The last point is what prevents a missed hit object from surviving time
freeze and damaging something later.

## Imported assets

The following exact BN5 ROM slices are extracted at build time:

| Asset | BN5 file offset | Length |
| --- | ---: | ---: |
| chip icon | `0x7493B8` | `0x80` |
| 56x48 library art | `0x728568` | `0x540` |
| base palette | `0x7343C8` | `0x20` |
| SP palette | `0x7343E8` | `0x20` |
| SearchMan actor archive | `0x254F64` | `0xABFC` |
| alternate reticle archive | `0x358410` | `0x5B8` |
| reticle archive | `0x3589C8` | `0x460` |

All imported assets and runtime code are placed with Armips `.autoregion` in
the expanded ROM rather than assigned fixed output offsets.

ChaosLrd also imports BN5 group `0x14` entry `0x0D`. It includes its header at
`0x389E68`; dropping that first word selects unrelated animation data and
causes the visible `MegaBstr` teardown. The complete archive is appended to
the relocated BN6 table.

The hit also loads packed value `0x00010401` and calls BN5's type-4 `0x0A`
palette-object wrapper at `0x080E1158`. Its variant-1 state is the four-frame
whole-screen whitening effect. BN6 retains the state machine, but schedules
its native `0x0A` slot only after this time-freeze sequence exits. The port
therefore runs that same palette write/restore state through its compiler-assigned
class-4 ID, which is updated during time freeze and
places the flash on the actual hit.
BN6 splits the rendered scene across palette slots `0x14` and `0x15`; writing
only one leaves either the field or the actors colored. The translated state
writes and restores both slots together, producing four full-screen white
frames and returning every palette before the teardown continues.

Each independently compiled registry preserves its target's native sprite-table
pointers and appends these SearchMan mappings:

- `SEARCHMAN_BATTLE_SPRITE_GROUP` / `SEARCHMAN_BATTLE_SPRITE` -> actor archive
- `SEARCHMAN_RETICLE_ALT_SPRITE_GROUP` / `SEARCHMAN_RETICLE_ALT_SPRITE` -> alternate reticle
- `SEARCHMAN_RETICLE_SPRITE_GROUP` / `SEARCHMAN_RETICLE_SPRITE` -> normal reticle

Both halves of each selector are compiler outputs, not duplicated numeric
constants. An
archive label such as `bug_charge_gospel_sprite` is only a ROM address; it cannot
identify a table slot until a generated pointer-table entry refers to it.

ChaosLrd deliberately appends the Bass archive twice. The main Bass actor and
Ball Bass animate simultaneously, and BN6 keys its sprite cache by the packed
group/index selector rather than the resolved archive pointer. Giving both
objects one selector makes their animation loads overwrite one cache identity
and produces corrupt composite graphics; distinct entry labels preserve the
two selectors while still sharing the underlying archive bytes.

Ball Bass has a second selector path that is easy to miss: BN6's type-1
allocator copies implicit argument `r4` into object word `+0x04`, whose low
bytes are the sprite group and index read by `ChaosBallInit`. The packed word
is therefore assembled from `CHAOSLORD_BALL_SPRITE_GROUP` and
`CHAOSLORD_BALL_SPRITE`.
Changing only the visible `r2` value leaves the old index in the object and
loads BN6's native white-dot placeholder instead of the Ball Bass composite.

The controller also preserves BN5's `0x8E` opening timer so Nebula Gray's
fade is not shortened. Bass's final animation `0x0F` has two five-frame
frames and marks its looping boundary with the animation-end flag. Phase 16
retires the controller at that boundary, while retaining BN5's `0x28` timer
as a fallback. This removes only the invisible post-animation wait needed to
fit BN6's shorter time-freeze callback budget.

Ball Bass does not use group `0x14`/index `0x14` for its impact. BN5 derives
the type-4 `0x24` pattern selector as attack-object variant `1 + 5`, yielding
native pattern `6`; that pattern requests native generic effect `0x24`, whose
descriptor is group `0x14`/index `0x00` with palette `1`. The port preserves
that calculation, so neither the generic-effect table nor the pattern table
needs relocation. Forcing index `0x14` instead selects the lightning archive
and produces the stray bolt when Bass disappears.

BN5 has base and SP library-art palettes, not an EX palette. The patch keeps
the base foreground and uses yellow BGR555 values `0x03FF`, `0x0299`, and
`0x0190` for EX's background; SP uses the native BN5 SP palette.

## Manifest-owned chip records

Package manifests declare chip records by ID under `[chips]`. The package
compiler maps semantic fields for codes, rarity, element, class, MB, behavior
route and object-spawn fields, library metadata, power, and artwork onto BN6's `0x2C`-
byte record. Unspecified fields retain their native edition value; this is
used for LaserMan's library metadata and for edition-native menu art. Nested
`gregar` and `falzar` tables override the common fields before assembly, so
the remaining version differences no longer require conditional record bytes
in package assembly. The alphabetical key at `+0x18` is deliberately absent
from the schema because the final sort pass regenerates it from chip names.

## Relocated text archives

Chip names and descriptions each use two archives. Archive 0 has IDs
`0x000`-`0x0FF`; archive 1 begins at `0x100`. NaviCust program names and
descriptions use one archive apiece, with the description archive stored as
LZ77-compressed text scripts. `build_text_archives.py` reads all six original
archives, applies readable charset-encoded replacements, rebuilds the NCP
script wrappers, and recompresses the NCP descriptions. Low-level text edits
select an explicit archive and local index. `config.gregar.toml` and
`config.falzar.toml` define each archive's format, extent, generated binary,
compression, and direct or compression-flagged pointer references. The
generated package assembly repoints those references.
Verification compares every untouched entry against the source ROM in both
versions.

## Jealousy port

BN5 Jealousy is chip ID `0xD4`. Its family-`0x15`/subfamily-`0x13` wrapper at
`0x080E4540` creates type-4 object `0x36`, whose controller begins at
`0x080E443C`. The controller is invisible; Jealousy has no dedicated battle
sprite archive to import.

The controller scans the opposing side's four unit pointers and preserves the
largest loaded-chip count. Every ten frames it scans all 18 blocks, passes the
opposing ownership flag in the block predicate's required-flags `r2` argument, creates an
80-damage hit object on each valid opposing block, and decrements that
count once. Its final 90-frame state refreshes the native chip-delete overlay
and runs the original link-battle cleanup calls before entering the generic
type-4 outro.

LifeSync is chip ID `0xBF` in BN6. Its released launcher stays in the copied
native prefix, while Jealousy's controller receives a new class-4 ID:

| Hook | BN6 file offset | Patched target |
| --- | ---: | ---: |
| family `0x15` table | compiler-relocated | `jealousy_attack_main` entry |
| class-4 table | compiler-relocated | `jealousy_controller_main` entry |

LifeSync's released object entry `0x5C` remains native and unmodified.

BN6 retains direct equivalents of all five generic type-4 lifecycle states and
of Jealousy's side comparison, chip-list lookup, block predicate, overlay,
gauge, and damage-object helpers. Gregar's damage-object wrapper is at
`0x080C6C16`; Falzar's is at `0x080C53A6`, so that one call is selected in the
version assembly rather than shared as a fixed address.

Jealousy's pulse loop remains the normal BN6 full-field damage-object path; it
does not inspect the opponent's hand or trap table. Every pulse resolves through
ordinary hit and damage prevention. As in BN5, the later chip-deletion
phase is separate from those hits and runs whenever the link battle is active,
even if AntiDamage or another trap intercepted the complete pulse train.

Jealousy's two time-freeze DMA records copy `0x100` bytes of BN5 overlay tiles
from file offset `0x6FAD2C` to `0x06017940` and its `0x20`-byte palette from
`0x6FAE2C` to BN6's relocated staging buffer at `0x030016F0`. The equivalent
native BN6 transfer tables use this address; BN5's `0x030036F0` overwrites
unrelated BN6 IWRAM. The menu icon (`0x748F38`, `0x80` bytes), library
art (`0x7250E8`, `0x540` bytes), and palette (`0x734188`, `0x20` bytes) are also
relocated with `.autoregion`.

## BugChain port

Blue Moon BugChain is chip ID `0xD3`. Its family-`0x0C`/subfamily-`0x1F`
wrapper at `0x080E6678` creates type-4 controller `0x3F`, whose main begins at
`0x080E65EC`. The chip-specific state waits 60 frames and calls the transfer
routine at `0x080E669A`; that routine duplicates active bug properties from
the user onto the opposing Navi without clearing the source. Blue Moon gates
the effect to link battles. It also creates type-4 object `0x40` on both Navis;
that object's main at `0x080E6724` loads group `0x0C`/index `0x32` and follows
its owner for 50 frames. At timer value 42, `0x080E67A4` plays SFX `0x15D`.

BN6 CopyDamage is chip ID `0xBE`. The port keeps that Standard-chip library
slot. The compiler assigns BugChain a new family-`0x15` subfamily and assigns
its controller and aura distinct IDs in the relocated class-4 table. The
common BN6 time-freeze lifecycle supplies the intro, freeze, outro, and cleanup
states. BN6 no longer keeps Blue Moon's
battle-kind enum in the same byte: its native link-only paths instead read the
battle configuration flags through `0x0802D246` and test bit `0x08`, which the
port uses for the equivalent gate.

BN6 expanded the battle-property block, so copying Blue Moon's seven raw byte
offsets would move unrelated state. The translated transfer instead covers
the nine byte fields and one halfword field cleared by BN6 BugFix at
`0x080E5D04`: `0x31`, `0x13`, `0x14`, `0x16`, `0x24`, `0x19`, `0x18`,
`0x1A`, `0x63`, and halfword `0x54`. A nonzero source value replaces the
target only when it is larger, preserving a stronger bug already present.
Block-bug severity `0x13` is transferred together with its action byte `0x12`;
otherwise CrackStep's action is left at zero and the affected panel is changed
to the empty-panel type. Buster-bug severity `0x14` likewise carries its level
byte `0x15`, matching the paired writes in BN6's NaviCust setup.

Blue Moon SFX `0x15D` has no exact BN6 match. Its one-track sequence, sample
header, and complete `0x4A1`-byte PCM body at
`0x081970A0` are imported at generated selector `BUGCHAIN_AURA_SONG`. Each aura
plays the cue at timer 42, matching `0x080E67A4`.

The exact Blue Moon menu assets are relocated from icon `0x74626C` (`0x80`
bytes), image `0x7315EC` (`0x540` bytes), and palette `0x73F1AC` (`0x20`
bytes). The aura archive is `0x380CA4`-`0x381C30` (`0xF8C` bytes). The unified
installer relocates the complete group-`0x08`, `0x0C`, `0x10`, and `0x14`
tables and appends every imported archive after each group's native entries.
The compiler assigns every imported archive a stable generated group/index
pair.
ChaosLrd, SearchMan, LaserMan, BugCharge, SignalRed, and
DeathPhoenix use later appended entries in the same final table images. The
original tables and every native pointer remain byte-for-byte untouched.

## BugCharge and SignalRed slots

BN6 BugFix remains native at chip ID `0x0B0`: neither its chip record nor its
family-`0x15`/subfamily-`0x1A` and type-4 `0x3B` dispatch entries are patched.
BugCharge instead occupies chip ID `0x131`, the Gregar-exclusive BugRSword
slot previously used by this patch's SignalRed port. It keeps that physical
library position and version flag, and uses the Giga class while importing BN5's B code,
77 MB cost, Null element, and 200 per-shot power.

SignalRed moves to Navi+20's Standard-chip ID `0x0C1`, retaining Navi+20's
library position. The compiler assigns BugCharge and SignalRed distinct
family-`0x15` subfamilies, so neither needs to inspect record byte `0x0A` or
forward through HubBatch's launcher. Their controllers and BugCharge's
charge-head visual likewise receive distinct class-4 IDs. Native attack and
object entries are copied unchanged into the relocated table prefixes.

The Custom Screen's selected-hand cleanup at `0x08029224` also recognizes the
old add-on chips by exact ID. Its branches at `0x08029238` and `0x0802923C`
would consume IDs `0x0C0` and `0x0C1` after an Attack or Navi chip even after
their records became Rook and SignalRed. Both branches are replaced with NOPs;
the adjacent native handling for the remaining add-on chips is preserved.

Exe6Runtime tracing of Colonel BugCharge identifies group `0x0C`/index `0x43` as
both the stationary and moving Gospel archive. The stationary object is 24
pixels forward of the user at Z `0x17`. BN5's object code gives this visual a
fixed 60-tick active state and a 30-tick teardown, so with a high bug count the
apparition can disappear before the controller finishes launching heads. The
port instead assigns `55 + 15 * shot_count` to its hold timer, matching the
controller's 40-frame charge, 15-frame shot cadence, and 30-frame recovery.
The moving object begins on the front block at Z `0x14`, advances 10 pixels per
frame, and changes its hit block at the midpoint between block centers.
The translated counter consumes BN5's nine property types plus BN6 field
`0x63`; after clearing it calls BugFix's native `0x0801E658` runtime-state reset
so an already-latched Custom bug is removed too. Every nonzero BN6 severity
counts as one active type rather than leaving values greater than one behind.

## SignalRed port

Blue Moon SignalRed is chip ID `0x131`. Its family-`0x15`/subfamily-`0x26`
wrapper at `0x080E8404` creates the short-lived type-4 controller whose effect
begins at `0x080E8448`. That controller calls `0x080DD772` to create the real
persistent traffic light, type 3 / ID `0x81`, controlled by `0x080DD544`.

The native object is placed one block in front of its owner, has 100 HP, and
loads sprite group/index `0x0C/0x33`. It spends 420 frames in animation 0
(red), with the opposing chip-enable flag cleared, followed by 50 frames in
animation 1 (green), with that flag restored. Blue Moon uses mask `0x08` for
owner 0 and `0x04` for owner 1 and plays sound `0x15C` when green begins. The
cycle repeats until the object is destroyed. The placement cue is Blue Moon
sound `0x0A0`. BN6 sound `0x180` reuses its note stream but not its sample,
priority, or timbre, so SignalRed imports the original Blue Moon song header,
voice, track, and PCM sample at generated selector `SIGNALRED_SPAWN_SONG`.
Blue Moon's green cue `0x15C` corresponds to BN6 sound `0x0D1` (the same
sequence with BN6's native volume balance), rather than the unrelated
same-numbered sounds.

The compiler installs the translated SignalRed entries through relocated tables:

| Hook | BN6 file offset | Patched target |
| --- | ---: | ---: |
| family `0x15` table | compiler-relocated | `signal_red_attack_main` entry |
| class-4 table | compiler-relocated | `signal_red_controller_main` entry |

The unified sprite installer appends `signal_red_battle_sprite` to group `0x10`
at derived index `0x61`; it does not modify the native entry formerly used by
the port.

SignalRed's persistent light receives its own compiler-assigned class-3 ID,
alongside the ChaosLrd, SearchMan, LaserMan, and BugCharge entries.

Blue Moon also registers the light in its per-owner deployable list through
`0x0800B230` and unregisters it through `0x0800B272`. Their structurally
identical BN6 counterparts are `0x0800F614` and `0x0800F656`. Retaining that
registration is what exposes the object to DustCross's B+Left suction path;
hit targetability alone is not sufficient. SignalRed registers in the
one-object deployable slot for its owner, while DustCross scans all eight
deployable pointers and marks a target with the suction bit belonging to the
DustCross user. SignalRed accepts only the bit for the side opposite its owner,
so a player cannot vacuum their own light.

BN6 hit setup keeps a matched obstacle mask pair on SignalRed, just as
Blue Moon does; clearing either complete word prevents incoming attacks from
resolving against the light. BN6 retains Blue Moon's owner-specific hit
pair at regions `19/20`: the active masks remain `0x02010000/0x01010000` and
`0x55800000/0xAA800000`, with only BN6's standard passive bit `0x80` added to
region 19. Using that pair prevents the chip owner's attacks from damaging the
light; the neutral `14/15` RockCube pair does not. SignalRed resubmits region
`19` every frame. Rush eligibility is decided earlier by `0x08010740`, which
tests bit `0x02` of chip-record byte `0x16`. SignalRed clears only that bit
(`0xC6` to `0xC4`) so its activation is not classified as Image Invis. The port
allocates the record at object initialization but defers
setup/presentation until time stop ends, so the persistent hurtbox begins in
normal battle time. Its three-frame placement startup and imported cue still
run during the activating chip's time freeze; only hit and the subsequent
red/green timer are deferred. Each active frame then clears prior hit presentation,
collects the five element-specific
hit damage slots, runs deployable lifetime maintenance, and applies the
total through BN6's saturating object-HP subtractor at `0x0800E2D8`. The broader
Navi damage state controller is not used because it requires a secondary status
object this passive obstacle does not own. Incoming damage sets the same
one-frame white sprite flash used by RockCube. The controller destroys the
light when its resulting HP reaches zero, spawning BN6's fire-explosion effect
kind `0x00` at a 16-pixel height and playing its native explosion SFX `0x070`.
Rook retains BN3's `0x0708`-frame lifespan and restores its visible object flag
before BN6's native helper applies each of the final 180 frames' blink states;
this prevents the first hidden blink frame from persisting until deletion.
Red/green state changes pause while time is stopped.
RockCube contributes the neutral support-object panel flag `0x00800000`
through collision type `14`. Rook and SignalRed contribute the owner-specific
support flags `0x01000000/0x02000000` through collision type `19`, allowing
their owner's attacks to pass through. WindRack and Tengu's shared traveling
wind check is extended from only `0x00800000` to the neutral bit plus the
opposing side's support-object bit, so Rook stops hostile wind without stopping
friendly attacks. Placement uses the
native panel-parameter check to require a solid panel and reject all three
support-object flags. An invalid panel uses the materialization-failure cue;
support-object occupancy uses the same damage explosion as RockCube contact.
Rook and SignalRed register only after that check passes, preventing a failed
placement from replacing an existing deployable-list entry.

DustCross's suction sweep at `0x080F10B4` walks all eight deployable-list
entries. For each eligible object, `0x0800F8B0` raises the common removal bit
`0x8000` and the DustCross owner's suction bit (`0x100000` or `0x200000`). On
either owner-specific bit, native deployables call `0x0800F90E` before cleanup;
that helper serializes the object's kind, animation, palette, flip, and position
into DustCross's stored-ammo path. SignalRed checks only the opponent's suction
bit before calling that helper and uses the otherwise-free four-bit
kind 15 and redirects DustCross's suction/firing sprite tables to a 16-entry
copy whose final entry is SignalRed's group `0x10`, index `0x61` archive. It
then restores the chip flag, cleans up hit state, unregisters, and frees
the field object. Hit event `0x40000` is a separate timed wind-removal
path handled by `0x0800F8CE`; its 20-frame visibility timer owns object byte
`+0x0B`, so SignalRed leaves that byte clear during normal operation.

The BN6 battle-flag helpers at `0x08001382` and `0x0800138E` retain Blue
Moon's set/clear contract; the battle structure field moved from `+0x64` to
`+0x5C`. SignalRed's gameplay routines live in `src/chips/signal_red.c`;
its assembly file contains only binary resources and the DustCross table
extension.

The exact Blue Moon asset slices are:

| Asset | Blue Moon file offset | Length |
| --- | ---: | ---: |
| chip icon | `0x746EEC` | `0x80` |
| 56x48 library art | `0x73A8EC` | `0x540` |
| menu palette | `0x73FAEC` | `0x20` |
| traffic-light battle archive | `0x381C30` | `0x694` |

Both versions use the imported battle archive and repoint Navi+20's three
menu-art fields to the imported SignalRed assets.

## DeathPhoenix port

BN5 DeathPhoenix's time-freeze wrapper creates type-1 object `0x28`, whose
controller at `0x080BFD98` builds three shuffled block rows and schedules
twelve strikes. Each strike creates type-4 object `0x89` through the wrapper at
`0x080EA71C`. Its main function is `0x080EA5E4`; the adjacent type-4 `0x8A`
main at `0x080EA740` is unrelated MegaBuster behavior and must not be ported.

Type-4 `0x89` creates direct-damage contacts at animation frames 0 and 16. It
also calls `0x080E8BF4` every four frames to create the visible type-4 `0x71`
flame actor. Type-4 `0x71` is implemented at `0x080E8ACC`, loads sprite group
`0x10`/index `0x49`, and supplies the moving purple fireball animation. That
archive occupies BN5 ROM `0x36F074`-`0x36F7BC`; group `0x10`/index `0x48` at
`0x36E908` is a different vertical-column effect.

The port translates type-4 `0x89` and `0x71` into distinct compiler-assigned
class-4 IDs. Both use the imported archive
through released BN6 sprite group `0x14`/index `0x21`. The damage contacts
remain separate native BN6 objects, matching BN5's split between hit and
visible flame actors. The main DeathPhoenix controller likewise receives its
own class-1 ID.

After the twelfth strike and the phoenix's disappear phase, the controller
checks BN6's saved-Navi record at `0x0203C960`. The record's backing pointer is
validated in addition to its ID because BN6 leaves an unused record zeroed,
whereas BN5 initializes the ID to `0xFF`. If a Navi was used previously, the
port invokes it through the saved-Navi dispatch table at `0x0802CD5C`, waits
for it to finish, and preserves BN5's 30-frame post-Navi pause.

BN5 then performs its own mode-0 return transition and waits another 30
frames. BN6's outer time-freeze controller automatically performs the same
return transition after the completion byte is released. The port therefore
releases completion immediately after the post-Navi pause and lets that outer
controller own the transition. Retaining BN5's cleanup wait or explicitly
starting a second mode-0 transition here causes the long extra pause and the
double MegaMan spawn-out/spawn-in sequence.

DeathPhoenix is installed at chip ID `0x134` in both versions. Falzar repoints
the three menu-art fields to BN5's assets; Gregar leaves those fields
byte-for-byte equal to the original CrossDiv record.

## BlackWeapon port

Blue Moon's BlackWeapon is chip record `0x121` (index 289). Family `0x0C`,
subfamily `0x4B` enters wrapper `0x080EAC82`, creates type-4 controller `0x7E`
at `0x080EAC08`, and opens type-1 actor `0x5A` at `0x080CC948`. The actor's
init is `0x080CC96C`; its two animation phases are `0x080CCA18` and
`0x080CCA54`.

The final phase writes four exact BN4 Navi-status values: offset `0x0E = 6`,
offset `0x2B = 1`, and offsets `0x06/0x07 = 4`. The first value makes BN4's
native battle loop remove 1 HP every 6 active frames while HP is above 1;
`0x2B` makes the ordinary Buster damage calculation return 10; and `0x06/0x07`
are the maximum Buster Speed and Charge values. The port translates those
semantics to BN6 status offsets `0x01 = 9`, `0x02/0x03 = 4`, and `0x18 = 8`.
BN6 normally indexes status `0x18` through the period table addressed by the
pointer at `0x080102A0`; the replacement table leaves native entries 0 through
7 (`0,40,35,30,25,20,15,10`) unchanged and adds private entry 8 with period 6.
This lets native HP-bug processing provide BN4's cadence without changing any
ordinary BN6 bug level.

Cross charged Buster attacks and chargeable Cross chip attacks share the
base-plus-per-level damage scaler at `0x08012642`. Its helper at `0x0801265A`
already calculates Attack levels through 10, but the caller reduces every
value above 5 back to 5 before multiplying it by the attack's native increment.
The patch removes only that second ceiling, leaving the helper's level-10 cap
in place. Attack levels 1 through 5 remain byte-for-byte equivalent, while
levels 6 through 10 from BlackWeapon or stacked BusterUp chips continue the
same attack-specific damage progression for both charge paths. The charged-
chip path at `0x0800FB54` dispatches its Cross power-
attack ID through `0x080117BA`, so it reaches this same scaler rather than a
separate damage calculation. Power-attack IDs 3 and 4 are the Gregar and Falzar
Beast Out rapid-Buster variants; the patch redirects their table entries at
`0x080117E0` and `0x080117E4` so those two attacks also retain every effective
Attack level from 6 through 10 regardless of whether BlackWeapon or BusterUp
supplied it.

The visual controller keeps the live player sprite visible so Cross and Beast
forms retain their exact current appearance. Its 60-frame phase alternates
every two frames between the player's assigned palette bank and a reserved
dark bank, followed by a 30-frame hold before applying the status values. In
Blue Moon's MegaMan archive, slot 1 is a near-black silhouette with dark
red/purple highlights.
Blue Moon sends those values to its object-palette setter at `0x08002DEC`, not
its color-effect setter. BN6's corresponding current-Navi slot is a white-hit
palette, so merely repeating the slot number gives the wrong appearance. The
The port queues Blue Moon's 32-byte dark palette from ROM offset `0x21B7F4`
into the reserved OBJ-palette bank on dark frames while leaving the current
player sprite and form intact. The native BN6 sprite archive and global
white-hit palette share that staging bank, so the controller saves the native
palette before its first dark frame and restores it during both normal and
forced teardown. There is no sound request anywhere in the BlackWeapon
controller or visual path.

Blue Moon contains only placeholder library art for the operation-battle chip.
EXE4.5's complete BlackWeapon icon, 56x48 library image, and palette occupy ROM
offsets `0x7640B0`, `0x755CF0`, and `0x75CEF0`, with lengths `0x80`, `0x540`,
and `0x20`. Gregar copies those three assets without palette conversion.

DeltaRay is chip ID `0x12F` in both editions. Only that record is replaced;
Bass at `0x12D` and BassAnly at `0x132` are untouched. The DeltaRay record
becomes a 64 MB, code-B Null GigaChip, preserves its original
version-availability split, and uses the Blue Moon behavior and animation in
both ROMs. Gregar repoints its three menu-art fields to EXE4.5's assets, while
Falzar leaves those fields byte-for-byte equal to the original DeltaRay
record.

## Exe6Runtime QA

The exact emulator procedure for selecting BN5 DeathPhoenix (`0x13A`) and
the patched BN6 replacement slot (`0x134`) without editing a folder is kept in
[`RUNTIME_QA.md`](RUNTIME_QA.md). It also records the Custom cache hook,
ownership-validator exception, matching-save requirement, and stale card-art
caveat so this setup does not need to be rediscovered.

The SignalRed runtime probe uses a deterministic clear field, with no rock on
the spawn block. In both versions it observes the light on the block directly
in front of the user, flag `0x08` cleared throughout red, restored during
green, an opponent Cannon held during red and released only after green opens,
then the flag cleared again when the cycle returns to red.

The emulator hit probe additionally exercises both contact and miss
paths. On contact it observes five one-update shot objects, effect `0` on
shots one through four, effect `9` only on shot five, one delete only after
that fifth contact, and a zero counter timer. On a miss, all five hit
results stay zero, neither HP nor delete state changes, and every shot still
receives exactly one update in both Gregar and Falzar.

## LaserMan port

Blue Moon's LaserMan chip creates type-1 object `0x4D` through the wrapper at
`0x080CABC4`. Its main, init, and update routines are at `0x080CABF0`,
`0x080CAC34`, and `0x080CAC96`. Exe6Runtime tracing of the real chip confirms that
the actor loads sprite group `0x08`, index `0x16`, plays summon SFX `0xB0`, and
uses animations `0`, `2`, `3`, and `4` for its idle, raised-arms, firing, and
recovery poses.

The firing state creates Blue Moon type-4 object `0xA0` through
`0x080E0FE6` and plays SFX `0x103`. Its init at `0x080E0E3C` loads that same
group-`0x08`/index-`0x16` archive and uses animations `17`, `18`, and `19` for
the thin lead-in, full blue-white row beam, and white tail. The shared archive
is LZ77-compressed at ROM offset `0x339B6C`; the source stream occupies
`0x395C` bytes. Asset extraction expands it to an ordinary archive before
assembly, so the package uses the same uncompressed sprite registry path as
every other imported sprite.

The actor's command parser at `0x080CAD1C` reads the held-key halfword and
tests Up (`0x40`), Down (`0x80`), Right (`0x10`), then Left (`0x20`). It stores
command IDs `1`-`4` in that priority order. The beam dispatch table at
`0x080E0F58` selects the following halfword streams: Up
`5,6,7,FD`; Down `1,2,3,4,FF0C,FD`; Right `010A,FD`; and Left `FE,FD`.
`FD` is the normal damage hit. `FE` dynamically packs the target's decremented
Custom Screen count with property ID `0x12` and clamps the count at two. The
port applies that direct Cust -1 behavior through BN6's Custom Level property
`0x0A`; it does not translate the command into the separate Custom bug property
`0x63`. Properties are read through the
per-side accessor at `0x080136CC` and written through the setter at
`0x080136B0`. The Up property IDs `5,6,7` translate to BN6 Attack, Rapid, and
Charge (`1,2,3`), while `010A` restores the B button and power attack to the
default values `0` and `1`. The port preserves Down's original five-event beam
delay, but routes its effect through BN6's native Uninstall routine instead of
translating and applying those properties separately.
Internal Buster levels are zero-based, so the three zeroes produced by Up are
the displayed level 1 values.
After choosing palette 0 for normal/Down and palette 10 for the other commands,
the source calls sprite-property setter `0x08002F22` with table values
`0, B060, A80A, 0, B9C0`. BN6's structural setter is `0x08002ED0`; the port
preserves all five values so Up, Down, and Left retain their distinct beam
transforms rather than collapsing to the yellow palette-only appearance.

The BN6 port retains HeatMan's IDs `0xE3`-`0xE5` and uses a compiler-assigned
family-`0x1B` subfamily. The actor and visible beam receive separate class-1
IDs. Both use BN6's standard battle OAM priority 2, keeping them behind the
Custom gauge and HUD; the earlier-allocated actor still wins their
same-priority overlap at the muzzle. LaserMan's row-hit objects receive their
own class-3 ID. Blue Moon's hit region `0x0B` is not a
compatible attack mask in BN6, so the port seeds each hit with BN6's proven
normal attack region 25 before using the native hit helpers. The final
`FD` event uses SearchMan's exact working region-25 hit initialization
but LaserMan's quiet cleanup, avoiding the
six random miss-impact sparkles that SearchMan's own cleanup would create for
the six block hits. Only the final `FD` damage event creates six block-contact
objects. The chosen direction is latched in each hit object's work area instead
of replaying the original property stream. Its command effect is applied
exactly once only when BN6's hit result at `+0x70` reports contact and that row
object occupies the opposing Navi's current block. Each object snapshots the
target's HP when its hit record opens, then closes that record on contact and
waits up to eight object updates for native damage processing. It applies the
selected command only after observing the target's HP decrease. Taking the HP
snapshot during initialization is required because BN6 can apply damage before
the shell receives its contact result; snapshotting on contact would record the
already-reduced HP and prevent every command effect from firing.
Consequently, a barrier, aura, AntiDamage trap, miss, or other zero-damage
outcome cannot authorize the effect. The Up, Right, and Left property words are
not passed through BN6's incompatible extended-effect IDs, while Down calls
the native Uninstall routine. Thus no direction has no extra effect, while a
held direction applies only its documented change after a damaging hit. The
compiler-generated
`LASERMAN_BATTLE_SPRITE_GROUP` / `LASERMAN_BATTLE_SPRITE` pair points directly
at the expanded shared archive. The reused BN6 object tails still require the
beam Z word to be cleared explicitly for the native actor and laser to render
in game.

Blue Moon creates the laser one block in front of the actor and then offsets
its sprite origin another 64 pixels in the owning side's direction. The port
reproduces both operations, which keeps the beam emitter aligned with
LaserMan's hand. It also samples and latches commands during the raised-arms
pose, and now takes an initial sample when the time-freeze summon is created so
a direction held with the chip-use input is not lost during the cut-in delay.
Blue Moon gated that parser away from Base; the BN6 port intentionally enables
it for Base, EX, and SP. On each confirmed Down-command contact, the port calls
BN6's native Uninstall routine at `0x080140EE` with the target as the current
player object. This preserves the native Link Navi eligibility guard, live
ability refresh, active-Cross behavior, and the shared StatGrd removal hook;
LaserMan no longer maintains a parallel set of property writes and cached-flag
repairs. The native routine includes the original Down command's B+Left effect:
it writes status property `0x07` to `FF` and, in base form, resets the live
B+Left cache as well. Right still sets the base Buster properties to `0` and
`1`, then restores the active Cross's native runtime power attack. Emulator
traces give power-attack IDs
`6, 11, 18, 20, 39, 12, 22, 15, 25, 40` for Cross IDs `1`-`10`; this removes
temporary charge-shot replacements without turning the active Cross's charge
shot into a MegaBuster. Outside those Crosses, the original cache comparison
still avoids overwriting an unrelated temporary form override.
The port reduces the original streams to command-dependent hit delays on the
same six-frame cadence: none is immediate, Up waits three events, Down five,
and Right/Left one. Because the imported full-width beam is stationary in the
BN6 object model, the sole `FD` damage event is represented on all six blocks;
only one six-object damage event is alive at a time.

Blue Moon provides Base, SP, and DS chip records. As with SearchMan, the BN6
series maps their object-spawn variants to Base/EX/SP values `0/3/4`. Base
uses the native red-background palette. The LaserMan artwork assigns its
variant background to palette indices `1`-`5`, so EX replaces only those five
entries with green shades and retains the base foreground at indices `6`-`15`.
SP uses the native yellow-background palette. The in-battle actor always
selects palette 0. Base is available in `L` and `*`, while EX and SP remain
`L`-only. HeatMan's version-specific library-order metadata remains intact in
both Gregar and Falzar; the final ROM pass regenerates alphabetical keys for
the complete sortable chip table from the relocated names. The relocated sound
table uses the common package's imported BN4 summon SFX `0xB0` directly and
adds the exact BN4 SFX `0x103` track, voicegroup, and PCM sample for the firing
cue.
