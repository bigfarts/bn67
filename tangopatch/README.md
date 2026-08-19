# bn67

## Behavior Changes

### Status Bug

The normal- and high-severity green invulnerability outcomes now use their
flashing invulnerability variants instead.

### Heat Cross

Heat Cross's charged Fire Arm keeps its normal behavior.

B+Left performs BN2's Burner: creating a five-panel plus on the user's panel
and the panels in front of, behind, above, and below it. Each flame deals 50
Fire damage, and the flames appear when MegaMan's throwing hand is fully lowered.

## Navi Customizer

### Beast Time +1

Beast Time +1 replaces Millions in its original NaviCust slot. It is available
in white, pink, and yellow as a Plus Part with the Emotion Window bug. Each
installed piece adds one turn to Beast Out's normal 3 turn duration, up to 9
turns.

Compressed:
```
xx
x
```

Uncompressed:
```
xx
xx
```

### Status Guard

BodyPack is renamed StatGrd and grants Status Guard only. It no longer
grants Super Armor, Float Shoes, Air Shoes, or Under Shirt. It also has a
different shape. Uninstall and the Blue Moon phase of SunMoon remove Status
Guard along with their native targets. Tomahawk Cross's separate innate Status
Guard remains active until Cross Out, but StatGrd stays removed underneath it.

Compressed:
```
 x
xxx
```

Uncompressed:
```
 x
xxx
 x
```

## Chips

### Standard Chips

#### Variable Sword

Variable Sword gains BN3's Element Sonic command. Hold A and enter
`B, B, Left, Down, Up` during its normal command window to fire four
160-damage Sonic Booms in Fire, Elec, Wood, then Aqua order. Each wave also
receives the attack bonus applied when Variable Sword was activated.

The first three waves deal damage and stagger without starting flashing
invulnerability. Only the final Aqua wave grants the target its normal
post-hit flashing.

#### Anti Navi

Anti Navi's MB cost is reduced from 50 MB to 33 MB in both versions.

#### Full Custom

Full Custom's MB cost is increased from 50 MB to 51 MB in both versions.

#### Gun del Sol EX

The Japanese-only `GunDelEX G` is restored as an 80 MB, Null-element Standard
chip. It uses its original Japanese icon, library art, palette, and native
spread-sunbeam attack.

#### Otenko

The Japanese-only `Otenko O` is restored as a 66 MB Obstacle-element Standard
chip. Otenko appears with his original battle sprite and raises attack power as
in the Japanese release. Its original menu art and description are restored as
well. Dust Cross also retains Otenko's sprite when it fires him as ammo.

#### Aqua Needle

Aqua Needle hits still stagger, but no longer make the target flash or grant
fixed invulnerability after each hit.

#### Jealousy

**Replaces:** Life Sync

The BN5 Jealousy attack is a 60 MB, Null-element Standard chip in code J with
80 displayed power. It deals 80 damage for each chip loaded by the opponent
and then deletes those loaded chips. All pulses run through normal hit
handling, while the separate deletion phase still runs through traps exactly
as in BN5. Both versions use Jealousy's BN5 icon, library art, palette, and
chip-delete overlay graphics.

#### Bug Chain

**Replaces:** Copy Damage

BN4 Bug Chain is a 59 MB, Null-element Standard chip in codes C/* and keeps
Copy Damage's library slot. In link battles it copies every active bug on the
user to the opposing Navi without removing the user's bugs or weakening
existing target bugs. Both versions use Bug Chain's BN4 icon, library art,
palette, battle aura, and sound effect.

#### Signal Red

**Replaces:** Navi +20

BN4 Signal Red's 100-HP traffic light spawns in front of the user, blocks the
opponent's chips for 420 red frames, then permits them during a 50-frame green
window. It repeats this cycle until destroyed. The light fails cleanly instead
of spawning if the block in front of the user is not solid.

Signal Red is a 61 MB Obstacle-element Standard chip in code S and keeps
Navi +20's library position. Both versions use its BN4 chip art and battle
sprite. Its 100-HP hurtbox remains active throughout the cycle, and only the
opposing player's Dust Cross can suck it in with B+Left. Its placement cue plays
during the activation freeze, and destruction produces a visible break effect.
Its activation flags do not advertise Image Invisibility to Rush, and its passive
field hit does not trigger Beat or traps. Dimming cut-ins do not stall either
peer.

#### Rook

**Replaces:** Attack +10

The 30 MB, Obstacle-element Standard chip uses code * and places a 500-HP Rook
on the block in front of the user after a dimming cut-in and its native summon
cue. It blocks non-Break attacks without losing HP. Break attacks deal normal
damage and must deplete all 500 HP; they do not delete it automatically. Both
versions use Rook's original BN3 chip art, palette, and tower-shaped battle
sprite. Dust Cross can suck it in and fire that same sprite as ammo.

#### Folder Back

**Replaces:** Color Point

BN3 Folder Back is a 99 MB, Null-element Standard chip in code `*`. It restores
every used chip (including itself), clears only the user's current hand,
reshuffles the Folder, resets that user's Mega chip and Giga chip usage totals,
and immediately returns to Custom. A consumed Regular chip returns as an
ordinary chip. The ending uses BN3's original rumble, 70-frame shake and
alternating white flash, briefly fills the Custom Gauge, and opens the native
Custom window with its normal sound. On link battle turn 15, only that rumble,
shake, and flash play; the Folder restore, gauge fill, and Custom transition
are skipped so Damage Judge proceeds normally. The chip also uses Folder Back's
BN3 menu art and text.

### Mega Chips

#### Navi Variants

All Navi chip series use V2 and V3 instead of EX and SP. Native SP damage
scaling is removed and each affected Navi V3 chip always uses its highest damage:
ElecMan V3 210, SlashMan V3 220, EraseMan V3 210, SpoutMan V3 120,
TomahawkMan V3 280, TenguMan V3 160, GroundMan V3 130, DustMan V3 200,
BlastMan V3 250, DiveMan V3 270, JudgeMan V3 190, ElementMan V3 240,
Colonel V3 300, and Count V3 50. Custom Navi
replacements that already have fixed power retain their existing values.

#### Count

Count, Count V2, and Count V3 are restored from the Japanese releases. Count
rains attacks over the target and follows with his lance sequence. All three
variants use their original records, menu palettes, compressed Count sprite,
lance objects, and complete ending, returning to battle normally after the
attack.

- `Count H/*`: 60 MB
- `Count V2 H`: 75 MB
- `Count V3 H`: 89 MB

#### Django

Django, Django V2, and Django V3 restore the three removed Mega Chip slots.
Django targets the closest opposing unit in his row, seals it in a coffin, calls
down rocks through their original poof effects, and burns the centered 3x3 area
with sunlight. All three variants use BN5's base Django icon and library art;
Django V2 uses the Japanese BN6 chip's green background colors, while Django V3
uses BN5 Django SP's palette. The actor, coffin, and sunlight sprites are BN5's
originals. Crossover keeps its native animation sequence and separately restores
the original Japanese BN6 Django actor archive and GunDelSol effect selectors.

- `Django D/*`: 30 MB, 130 damage
- `Django V2 D`: 70 MB, 180 damage
- `Django V3 D`: 90 MB, 260 damage

#### ProtoMan

ProtoMan, ProtoMan V2, and ProtoMan V3 use BN6's native Delta Ray Edge attack
instead of their normal Wide Sword summon.

- `ProtoMan B/*`: 80 damage
- `ProtoMan V2 B`: 100 damage
- `ProtoMan V3 B`: 200 damage

#### Colonel

Colonel, Colonel V2, and Colonel V3 use BN6's native Cross Divide attack
instead of their normal screen-splitting summon. Their chip descriptions
identify the Cross Divide attack.

#### SearchMan

**Replaces:** CircusMan series

The complete SearchMan chip attack is ported from BN5. Its Cursor-element shots
destroy traps on contact, and a successful delete shot discards the opponent's
entire loaded hand.

- `SearchMan S/*`: 20 damage per shot
- `SearchMan V2 S`: 40 damage per shot
- `SearchMan V3 S`: 75 damage per shot

#### NumberMan

**Replaces:** ChargeMan series

NumberMan uses his BN5 actor and die. He throws the die three panels ahead,
shows a random face from 1 to 6, and then hits the centered 3x3 area for the
rolled face times the listed power.

- `NumberMan N/*`: 30 base power, 30-180 damage
- `NumberMan V2 N`: 40 base power, 40-240 damage
- `NumberMan V3 N`: 90 base power, 90-540 damage

#### LaserMan

**Replaces:** HeatMan series

BN4 LaserMan raises his arm, points forward, and fires the original piercing
blue-white laser through the complete row.

- `LaserMan L/*`: 100 damage
- `LaserMan V2 L`: 150 damage
- `LaserMan V3 L`: 200 damage

Hold a direction while LaserMan raises his arms to add the original command
effect. This works for Base, V2, and V3:

- **Up:** Resets Attack, Rapid, and Charge to level 1.
- **Down:** Uninstalls the target's Super Armor, Float Shoes, Air Shoes, Under
  Shirt, B+Left, and tatus Guard for the rest of the battle. Abilities built
  into the target's current Cross remain available until Cross Out, but
  installed copies stay removed underneath the Cross. Link Navis are immune,
  just as they are to the regular Uninstall chip.
- **Right:** Removes charge-shot replacements. It restores an active Cross's
  native charge shot while resetting the base form to the standard MegaBuster.
- **Left:** Permanently reduces the target's Custom Screen selection by one
  chip, to a minimum of two.

Command effects require the beam to reduce the target's HP. Barriers, auras,
AntiDamage, and missed attacks prevent the command effect.

Base uses the native red-background menu palette. LaserMan V2 keeps the base
LaserMan foreground and changes only its five background entries to green.
LaserMan V3 uses the native yellow-background SP palette. All variants use the
base battle palette.

### Giga Chips

#### Double Beast

Both editions use Double Beast's Japanese icon, library art, palette, and
description.

#### Gregar

**Gregar GigaChip.**

The Japanese-only `Gregar X` is restored as a 99 MB, Null-element Giga chip.
It uses Gregar's original chip art and full summon sprite, including the
falling-rock and scorching-breath sequence, and returns to battle normally.

#### Falzar

**Falzar GigaChip.**

The Japanese-only `Falzar X` is restored as a 99 MB, Null-element Giga chip.
It uses Falzar's original chip art, palette, full summon sprite, Strike
Feathers, Sonic Wave, and tornado sequence, and returns to battle normally.
Its 100-damage Strike Feathers also receive the activating chip's attack bonus.

#### Dark Aura

**Replaces:** Bug Death Thunder

**Falzar GigaChip.**

BN3 DarkAura is an 89 MB, Null-element Giga chip in code A. It lasts for 3,000
active battle frames, repels attacks below 300 damage, and breaks when hit for
300 or more. It uses DarkAura's original BN3 icon, center-cropped library art,
name, description, centered aura animation, and dark battle palette. The
battle aura displays no damage number.

#### Black Weapon

**Replaces:** Delta Ray Edge

**Gregar GigaChip.**

The 64 MB, Null-element Giga chip uses code B, sets Buster Attack to level 10
and Rapid and Charge to level 5, and drains 1 HP every 6 active battle frames
without reducing the user below 1 HP.

Cross-charged Buster attacks, chargeable Cross chip attacks, and both Beast Out
rapid Buster variants scale through all 10 Buster Attack levels. The activation
animation preserves the user's current Cross or Beast form. Gregar uses
Black Weapon's EXE4.5 chip art; Falzar keeps the original Delta Ray Edge slot
art.

#### Chaos Lord

**Replaces:** Big Hook

**Gregar GigaChip.**

The BN5 Chaos Lord attack is Null element, code S, and has 500 displayed
power. Gregar uses Chaos Lord chip art while Falzar keeps the original art for
that slot.

#### Bug Charge

**Replaces:** Bug Rise Sword

**Gregar GigaChip.**

The 77 MB, Null-element Giga chip uses code B. It fires one 200-damage Gospel
shot plus one shot per active battle-bug type it clears, including the Custom
Screen bug and its latched runtime state. The stationary Gospel remains for
the complete firing sequence. Gregar uses the imported Bug Charge chip art;
Falzar retains the inaccessible Bug Rise Sword slot art.

#### Duo

**Replaces:** Meteor Knuckle

**Falzar GigaChip.**

The complete BN4 Duo attack is a 99 MB, Null-element Giga chip in code D and
deals 200 damage per fist. Duo destroys every obstacle, flashes the user's rear
panels, enters with his original sound and screen effects, then launches 17
alternating fists at panels around the opposing units. Each impact cracks its
panel. Both versions use Duo's BN4 battle sprite and sounds; Falzar also imports
its chip art.

#### Death Phoenix

**Replaces:** Cross Divide

**Falzar GigaChip.**

BN5 Death Phoenix fires twelve Death Phoenix fireballs and then recycles the
last Navi chip used. Falzar uses Death Phoenix chip art; Gregar retains the
original Cross Divide chip art.

## Visual Changes

### Title Screen

The gold `6` in both title logos now reads `67`.
