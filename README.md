# bn67

## Changes

### Beast Out

Link battles start each player with five Beast Out turns instead of three.

### Title screen

The gold `6` in both title logos now reads `67`. The finished, beveled `7` is
stored at its final screen coordinates in the transparent 240x160
`assets/title-screen-overlay.png`. The builder overlays that complete image at
`(0, 0)` and maps its reference colors to each version's native title palette.
Each edition's complete 256x160 title layer and uniform 32x20 tile map are
rebuilt wholesale rather than patching individual native atlas cells.

### AntiNavi

AntiNavi retains its native BN6 behavior, codes, and metadata, but its MB cost
is reduced from 50 MB to 33 MB in both versions.

### FullCust

FullCust retains its native BN6 behavior, wildcard code, and metadata, but its
MB cost is increased from 50 MB to 51 MB in both versions.

### StatGrd

BodyPack is renamed StatGrd and grants StatusGuard only. It no longer grants
SuperArmor, FloatShoes, AirShoes, or UnderShirt.

### Japanese event chips

Both English editions restore the chips and resources that were available only
in the Japanese releases:

- `GunDelEX G` restores GunDelSol EX's original chip record and Japanese menu
  art. Its spread-sunbeam attack was retained by the English engine and remains
  native.
- `Otenko O` restores the original 66 MB StandardChip, menu art, battle sprite,
  description, and attack-power support effect. DustCross retains Otenko's
  restored sprite when it sucks him in and fires him as ammo.
- `Count H/*`, `Count2 H`, and `Count3 H` restore Count's three MegaChips,
  variant palettes, compressed actor archive, rain attack, lance objects, and
  complete cleanup sequence.
- `Django D/*`, `Django2 D`, and `Django3 D` restore the three removed
  MegaChips with 130, 180, and 260 damage. All three use BN5's base Django
  menu artwork and complete coffin, poofing falling-rock, and sunlight sequence;
  Django2 uses the Japanese BN6 chip's green background, while Django3 uses
  BN5 DjangoSP's palette. Django targets the closest opposing unit in his row
  and burns the centered 3x3 area around it. Crossover separately restores
  Django's original Japanese BN6 actor archive and its GunDelSol effect
  selectors in place of the English placeholders.
- `DoubleBeast` keeps the routine already present in the English engine and
  restores its missing Japanese icon, library art, palette, and description.
- `Gregar X` and `Falzar X` restore the two 99 MB GigaChips with their original
  Japanese records, menu art, compressed summon archives, controllers, child
  attacks, and cleanup routines. They are usable in both editions.

### Falzar

Falzar's 100-damage Strike Feathers now receive the activating chip's attack
bonus, matching its Sonic Wave and Gregar's 100-damage falling rocks.

### VarSwrd

VarSwrd gains BN3's ElementSonic command. Hold A and enter
`B, B, Left, Down, Up` during the native input window to fire four Sonic Booms
in Fire, Elec, Wood, then Aqua order. Every wave uses VarSwrd's 160 base power
and receives the chip's attack bonus. Only the final Aqua wave starts flashing
invulnerability. The five native BN6 commands, including its ordinary single
Sonic Boom, retain their original inputs and behavior.

### ProtoMan

ProtoMan, ProtoMan2, and ProtoMan3 now use BN6's native DeltaRay attack
instead of their normal WideSword summon. Each chip retains its original
codes, MB cost, Navi-chip classification, and artwork. Base, 2, and 3 deal
80, 100, and 200 damage respectively.

### Colonel

Colonel, Colonel2, and Colonel3 now use BN6's native CrossDivide attack
instead of their normal screen-splitting summon. Each chip retains its original
codes, MB cost, Navi-chip classification, artwork, and power.

### Navi chip variants

Every Navi chip series now uses `2` and `3` instead of EX and SP in its names.
The native time-based SP scaling sentinels are replaced with fixed damage at
the highest value in each Navi's table. This fixes ElecMan3 at 210, SlashMn3
at 220, EraseMn3 at 210, SpoutMn3 at 120, TmhkMan3 at 280, TenguMn3 at 160,
GrndMan3 at 130, DustMan3 at 200, BlastMn3 at 250, DiveMan3 at 270, JudgeMn3
at 190, ElmntMn3 at 240, Colonel3 at 300, and Count3 at 50. Navi replacements
that already use deliberately fixed power retain those values.

### BlakWeap

Replaces DeltaRay (`0x12F`) in both versions with Blue Moon's BlackWeapon
behavior; Bass and BassAnly are unchanged. The 64 MB, Null-element GigaChip
uses code B, sets Buster Attack to level 10 while Rapid and Charge remain at
level 5, and drains 1 HP every 6
active battle frames without reducing the user below 1 HP. Cross charged
Buster attacks, chargeable Cross chip attacks, and the two Beast Out rapid-
Buster variants now scale continuously through all 10 Buster Attack levels;
this includes levels 6 through 10 reached by stacking BusterUp. Its original
animation flickers the user's current form for 60 frames and holds for 30;
active Cross and Beast forms remain visible throughout. The effect has no
dedicated sound. Gregar imports BlackWeapon's complete
icon, library art, and palette from EXE4.5, while Falzar retains the native
DeltaRay-slot art.

### SearchMan

Replaces the CircusMan series with the complete SearchMan
chip attack ported from Battle Network 5. Its shots retain Cursor element and
destroy traps on contact. A successful delete shot discards the opponent's
entire loaded hand.

- `SerchMan S/*`: 20 damage per shot
- `SerchMn2 S`: 40 damage per shot
- `SerchMn3 S`: 75 damage per shot

### NumberMan

Replaces the ChargeMan series with NumberMan from Battle
Network 5. He throws a die three panels ahead; the displayed face from 1 to 6
multiplies the chip's power, then the die explodes over the centered 3x3 area.

- `NumbrMan N/*`: 30 base power, 30-180 damage
- `NumbrMn2 N`: 40 base power, 40-240 damage
- `NumbrMn3 N`: 90 base power, 90-540 damage

### LaserMan

Replaces the HeatMan series with Blue Moon's LaserMan. The
summoned Navi raises his arms, points forward, and fires Blue Moon's piercing
blue-white laser through the complete row.

- `LaserMan L/*`: 100 damage
- `LaserMn2 L`: 150 damage
- `LaserMn3 L`: 200 damage

Holding a direction while LaserMan raises his arms enables Blue Moon's command
effect on Base, 2, and 3:

- Up resets Attack, Rapid, and Charge to level 1.
- Down disables the base form's SuperArmor, AirShoes, FloatShoes, Undershirt,
  StatusGuard, and B+Left abilities. An active Cross keeps its live versions
  until Cross Out, when the cleared base-form configuration takes effect.
- Right removes charge-shot replacements. It restores an active Cross's native
  charge shot while resetting the base form to the standard MegaBuster.
- Left permanently reduces the target's Custom Screen selection by one chip,
  to a minimum of two chips.

Command effects require the beam to contact the target. A missed LaserMan does
not alter stats, abilities, charge shots, or Custom behavior.

All three are Null-element MegaChips. Base uses Blue Moon's base art palette,
LaserMan2 preserves every base LaserMan foreground color while changing only
the five red-background palette entries to green, and LaserMan3 uses the
native yellow-background Blue Moon SP palette. All three use LaserMan's base
battle palette.

### ChaosLrd

Replaces BigHook with the BN5 ChaosLrd attack. Bass, Ball Bass, the fireball,
Nebula Gray apparition, white-flash impact, and teardown sprites are ported from
BN5. The chip is Null element, code S, and has 500 displayed power. Gregar
uses ChaosLrd menu art; Falzar intentionally retains its original slot art.

### Jealousy

Replaces LifeSync with the complete BN5 Jealousy attack. It is a 60 MB,
Null-element StandardChip in code J with 80 displayed power. For each chip
loaded by the opponent, Jealousy produces one 80-damage full-field pulse, then
runs BN5's chip-delete overlay and cleanup sequence. Every pulse fires through
normal hit handling, while the separate deletion phase still runs through
traps exactly as it does in BN5. Its icon, library art, palette, and delete-overlay graphics are imported
from BN5 in both versions.

### BugChain

Replaces CopyDamage with Blue Moon's BugChain. It is a 59 MB, Null-element
StandardChip in codes C/* and keeps CopyDamage's library slot. In link battles,
BugChain waits through Blue Moon's 60-frame time-freeze beat and copies every
active bug on the user to the opposing Navi without removing the user's bugs
or weakening bugs already present on the target. Its icon, library art, and
palette are imported from Blue Moon in both versions, along with the original
50-frame bug aura and SFX `0x15D` displayed and played by both Navis.

### BugCharg

Replaces the former SignalRed/BugRSword slot with BN5 Colonel's BugCharge. The
77 MB, Null-element GigaChip uses code B and fires one 200-damage Gospel shot
plus one additional shot for every active battle-bug type it clears. It
consumes the complete battle-bug property set, including the Custom-screen
bug, and resets the latched runtime bug state. The stationary Gospel uses
BN5's 24-pixel forward offset and remains present through the full bug-scaled
firing sequence; each moving head begins on the front block with BN5's native
hit timing. Gregar uses the imported BugCharge menu art; Falzar retains the
inaccessible BugRSword-slot art.

### Rook

Replaces Attack+10 with BN3 Rook while leaving AntiSword unchanged. Rook is a
30 MB Obstacle-element StandardChip in `*` code only. It places Rook on the
block in front of the user with 500 HP after a dimming cut-in and its native
summon cue. Non-Break attacks are stopped without reducing that HP. AirShot,
WindRack, and Tengu Racket are the push exceptions: their damage is still
blocked, but they move Rook one panel away from the attacker at AirShot's
native 10-pixel-per-frame knockback speed. An invalid panel, Navi, virus, or
blocking object prevents the move. An invalid panel leaves Rook in place, but
a Navi, virus, or blocking object behind it destroys Rook immediately without
dealing collision damage. Rook stops Tengu Racket's traveling gust and Tengu's
B+Left wind from passing through its panel; B+Left does not move Rook. Its
owner's attacks still pass through it.
Break attacks deal their normal damage, so they must deplete the full 500 HP
rather than deleting Rook in one hit. Like BN3's original, Rook lasts 1,800
active frames and blinks during its final 180 before expiring. Both versions
import Rook's BN3 icon,
center-cropped library art,
palette, and original tower-shaped battle sprite. DustCross can suck Rook in
and fire that same sprite as ammo, using the registry's shared DustCross
selector table.

### SignlRed

Replaces Navi+20 with the complete Blue Moon SignalRed behavior. The 100-HP
traffic light appears on the block in front of the user, disables the opposing
side's BattleChip use for 420 red frames, opens a 50-frame green window, and
repeats until destroyed. It is a 61 MB Obstacle-element StandardChip in code
S and keeps Navi+20's library position. Both versions import the Blue Moon menu
art and battle sprite. Its placement cue imports Blue Moon's original sample
and sequence; the green-light cue uses its matching BN6 sound,
and the light is registered as a normal deployable so DustCross can suck it in
with B+Left. Its 100-HP hurtbox remains active every frame, but only the opposing
player's DustCross can suck it in. The placement cue plays during the activation
freeze, and destroying the light produces its break effect. Its native
owner-specific passive hit is enabled
after the cut-in ends, so it does not trigger Beat or traps. Its chip-activation
flags also clear BN6's Image Invis classification bit, so playing it does not
wake Rush. Dimming cut-ins also pause its red/green state transition without
stalling either peer.

### DethPhnx

Replaces CrossDiv with BN5 DeathPhoenix in both versions. The phoenix attacks
with twelve fireballs and then replays the last used Navi chip, matching BN5's
recycle tail. Falzar imports the DeathPhoenix icon, library art, and palette;
Gregar deliberately keeps CrossDiv's original menu art.

### FoldrBak

Replaces ColorPoint with BN3 FolderBack in both versions.
It restores every used chip to the user's equipped Folder, including
FolderBack itself, clears only the user's current hand, reshuffles once through
BN6's native Folder setup, resets that user's accumulated chip-class usage so
the restored Mega and Giga chips remain selectable, and immediately reopens
Custom. A consumed Regular
Chip returns as an ordinary chip rather than regaining its Regular
designation. The ending ports BN3's SFX 0x120 rumble, 0x46-frame shake and
alternating white flash,
then shows a full Custom Gauge for 20 frames before the native Custom window
opens with its normal sound. Its wildcard code, 99 MB cost, icon,
center-cropped library art, palette, name, and description come from BN3 Blue.

## Assets and palettes

The build extracts GunDelEX, Otenko, Count, DoubleBeast, Gregar, and Falzar
resources from the Japanese BN6 releases; BugChain's menu art and aura,
LaserMan's menu art and palettes, shared actor/beam archive, summon and firing
samples, and SignalRed's menu art and battle sprite from Blue Moon; Jealousy's
menu/overlay graphics, Django's menu art and complete battle sequence, and
BugCharge's menu and Gospel-head assets from BN5;
Rook's BN3 menu art and battle sprite; FolderBack's BN3 menu art and original
rumble PCM; plus the full BN5 SearchMan actor archive, both scope/reticle
archives, the chip icon, and the 56x48 library artwork; NumberMan's actor and
die archives and library art; and BlackWeapon's menu art from EXE4.5. The SearchMan
variant library-art palettes are:

- base: BN5 base palette (blue background)
- 2: unchanged SearchMan foreground with a custom yellow background
- 3: BN5 SP palette (red/pink background)

Base, 2, and 3 all select the same in-battle actor palette. The three menu
art palettes remain distinct.

The first 13 nontransparent palette entries used for SearchMan's foreground
are byte-identical in all three variants. Only the three background entries
differ.

NumberMan Base keeps BN5's dark background, NumberMan2 preserves the same
foreground with a blue background, and NumberMan3 uses BN5's SP background
palette. BN6's native
summon, throw, landing, and final-blast cues are byte-identical to BN5. The
slightly different BN5 explosion sequence is imported separately.

See [DISASSEMBLY.md](DISASSEMBLY.md) for the native BN5 object chain and BN6
hook locations.

## Source registry

Gameplay implementations may live anywhere under `src/`. Each implementation
registers its own chip records, attacks, objects, sprites, songs, dependencies,
and pointer patches with `BN67_*` macros. There is no package manifest.

`compile_c_metadata.py` compiles the gameplay package sources in target-specific
metadata mode and extracts their ordered `__bn67_meta__...` symbols from the ELF
objects with `arm-none-eabi-nm`. `compile_registry.py` consumes one config path
per invocation, allocates that config's registry independently, and generates
absolute linker symbols used by expressions such as
`BN67_OBJ_ID(searchman_actor_main)` and
`BN67_SPRITE_ID(searchman_battle_sprite)`.

Complete 44-byte chip records are ordinary C initializers embedded in the
linked gameplay image:

```c
BN67_CHIP_RECORD(0x131) {
    .codes = { EXE6_CHIP_CODE_B, EXE6_CHIP_CODE_NONE,
               EXE6_CHIP_CODE_NONE, EXE6_CHIP_CODE_NONE },
    .behavior = {
        .family = BN67_ATTACK_FAMILY(bugcharge_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(bugcharge_attack_main),
    },
    .power = 200,
    // Remaining Exe6ChipRecord fields omitted here for brevity.
};
```

The final Armips pass copies each linked record over its original table entry.
Optional sibling `<name>.text.toml` files contain only text archive edits, with
archive names directly at the top level:

```toml
[chip-names-1]
"0x31" = "BugCharg"
```

See [src/README.md](src/README.md) for the complete registry, chip-record, and
text reference.

## Build

Requirements: Make, `armips`, Python 3, the Arm GNU toolchain
(`arm-none-eabi-gcc`, `objcopy`, and `nm`), and optionally `flips` for BPS
output.

Pass the eight source ROM paths as Make variables. The Japanese BN6 ROMs
supply the event-chip art, sprites, and original object routines removed from
the English releases:

```sh
make \
  BN5_PROTOMAN_ROM=/path/to/bn5.srl \
  BN6_GREGAR_ROM=/path/to/bn6-gregar.srl \
  BN6_FALZAR_ROM=/path/to/bn6-falzar.srl \
  EXE6_GREGAR_ROM=/path/to/bn6-jp-gregar.srl \
  EXE6_FALZAR_ROM=/path/to/bn6-jp-falzar.srl \
  BN4_BLUE_MOON_ROM=/path/to/bn4-blue-moon.srl \
  BN3_BLUE_ROM=/path/to/bn3-blue.srl \
  EXE45_ROM=/path/to/exe4.5.gba
```

`BN5_COLONEL_ROM` defaults to `exe5k_rom_k_e.srl` beside the Team ProtoMan
ROM. Set it explicitly if the Colonel ROM is elsewhere. Tool paths can
likewise be overridden with `ARMIPS`, `FLIPS`, and `TANGO_PATCH`.

Supported source SHA-256 hashes:

- BN5 Team ProtoMan: `b35f5890f54784c9d90a896dc5ac4831d43acc9f94e8c42816742fcfa6b41a7b`
- BN5 Team Colonel: `d4b7aefc3918c9f801c84cfd1322c2cdbb9d13c2e3271b3c3f8f9927480f2633`
- BN6 Gregar: `572e113eeb53bb29cd9ff8acb9db265cfd48c5e509c8d0e6420b58e71e442cf2`
- BN6 Falzar: `a37c1028adb72082b51e142321fa437967bc54b6f46730a53f6581ad455ad670`
- BN6 Japanese Gregar: `fa6808a5c63c2cc09430ec7ad74c6e02f4f35928448e6ff5f8dbdec0795160cf`
- BN6 Japanese Falzar: `21300170c404371da5cd0c327c3959c0981cf0af6e6bb9189fec4010fc6258a4`
- BN4 Blue Moon: `63ea187c792f4bfcd077f92c3a509fa09ed422993aee9480c39dfdf6a561c5c1`
- BN3 Blue: `8c6767788f99dc9e2af0c9d75513b227c7c42d6d452d6165c8e08850af78e273`
- EXE4.5 English: `588a77da006fb0dca0c8addbcc316d7bd4b1c3a42db24750bcfe17b170ac5ef8`

Patched ROM copies are written under `build/`. If `flips` is available, BPS
patches are written under `dist/`. If `tango-patch` is also available, the two
BPS payloads are packaged together as `bn67-1.0.0.tangopatch`. The
supplied source ROMs are never modified.

As a final build step, every nonzero alphabetical-sort key in the main chip
table is regenerated from the completed relocated name archives. The pass uses
case-insensitive natural ordering, so renamed chips and numbered or EX/SP
series appear in name order in both Gregar and Falzar. It reassigns the ROM's
original sparse key values without inventing a new numeric format; zero-keyed
special, enemy, and unused records remain untouched.

## Text archives

BN6 splits chip names and descriptions into an earlier 256-entry archive for
IDs `0x000`-`0x0FF` and a later archive beginning at ID `0x100`. The Python
builder relocates all four archives (two names and two descriptions). The
generated assembly repoints every configured consumer.

```toml
[chip-names-1]
"0x07" = "SerchMan"

[chip-descriptions-1]
"0x07" = "Aim\nand fire\n5 shots"
```

These tables live in `searchman.text.toml`; untouched entries remain unchanged.
