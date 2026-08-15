# bn67

## Behavior Changes

### Beast Out

Link battles start each player with five Beast Out turns instead of three.

### Status Bug

The normal- and high-severity green invulnerability outcomes now use their
flashing invulnerability variants instead.

## Chips

### Standard Chips

#### Variable Sword

Variable Sword gains BN3's Element Sonic command. Hold A and enter
`B, B, Left, Down, Up` during its normal command window to fire four
160-damage Sonic Booms in Fire, Elec, Wood, then Aqua order. Each wave also
receives the attack bonus applied when Variable Sword was activated.

The first three waves deal damage and stagger without starting flashing
invulnerability. Only the final Aqua wave grants the target its normal
post-hit flashing. Variable Sword's five native BN6 commands, including the
ordinary single Sonic Boom, retain their original inputs and behavior.

#### Anti Navi

Anti Navi's MB cost is reduced from 50 MB to 33 MB in both versions. Its native
behavior and all other chip metadata are unchanged.

#### Full Custom

Full Custom's MB cost is increased from 50 MB to 51 MB in both versions. Its
native wildcard code, behavior, and all other chip metadata are unchanged.

#### GunDelSol EX

The Japanese-only `GunDelEX G` is restored as an 80 MB, Null-element Standard
chip. It uses its original Japanese icon, library art, palette, and native
spread-sunbeam attack.

#### Otenko

The Japanese-only `Otenko O` is restored as a 66 MB Obstacle-element Standard
chip. Otenko appears with his original battle sprite and raises attack power as
in the Japanese release. Its original menu art and description are restored as
well.

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

BN3 Rook replaces Attack +10 while Anti Sword remains unchanged. The 30 MB,
Obstacle-element Standard chip uses codes * and places a 500-HP Rook on the
block in front of the user after a dimming cut-in and its native summon cue. It
blocks non-Break attacks without losing HP. Break attacks deal normal damage
and must deplete all 500 HP; they do not delete it automatically. Both versions
use Rook's original BN3 chip art, palette, and tower-shaped battle sprite. Dust
Cross can suck it in and fire that same sprite as ammo.

#### Folder Back

**Replaces:** Color Point

BN3 Folder Back is a 99 MB, Null-element Standard chip in code `*`. It restores
every used chip (including itself), clears only the user's current hand,
reshuffles the Folder, resets that user's Mega chip and Giga chip usage totals,
and immediately returns to Custom. A consumed Regular chip returns as an
ordinary chip. The ending uses BN3's original rumble, 70-frame shake and
alternating white flash, briefly fills the Custom Gauge, and opens the native
Custom window with its normal sound. The chip also uses Folder Back's BN3 menu
art and text.

### Mega Chips

#### Count

Count, Count EX, and Count SP are restored from the Japanese releases. Count
rains attacks over the target and follows with his lance sequence. All three
variants use their original records, menu palettes, compressed Count sprite,
lance objects, and complete ending, returning to battle normally after the
attack.

- `Count H/*`: 60 MB
- `Count EX H`: 75 MB
- `Count SP H`: 89 MB

#### ProtoMan

ProtoMan, ProtoMan EX, and ProtoMan SP use BN6's native Delta Ray Edge attack
instead of their normal Wide Sword summon. Their original codes, MB costs,
Navi chip classification, and artwork are unchanged.

- `ProtoMan B/*`: 80 damage
- `ProtoMan EX B`: 100 damage
- `ProtoMan SP B`: 200 damage

#### Colonel

Colonel, Colonel EX, and Colonel SP use BN6's native Cross Divide attack
instead of their normal screen-splitting summon. Their original codes, MB
costs, Navi chip classification, artwork, and power are unchanged. Their chip
descriptions now identify the Cross Divide attack.

#### SearchMan

**Replaces:** CircusMan, CircusMan EX, CircusMan SP

The complete SearchMan chip attack is ported from BN5. Its Cursor-element shots
destroy traps on contact, and a successful delete shot discards the opponent's
entire loaded hand.

- `SearchMan S/*`: 20 damage per shot
- `SearchMan EX S`: 40 damage per shot
- `SearchMan SP S`: 75 damage per shot

#### NumberMan

**Replaces:** ChargeMan, ChargeMan EX, ChargeMan SP

NumberMan uses his BN5 actor and die. He throws the die three panels ahead,
shows a random face from 1 to 6, and then hits the centered 3x3 area for the
rolled face times the listed power.

- `NumberMan N/*`: 30 base power, 30-180 damage
- `NumberMan EX N`: 40 base power, 40-240 damage
- `NumberMan SP N`: 90 base power, 90-540 damage

#### LaserMan

**Replaces:** HeatMan, HeatMan EX, HeatMan SP

BN4 LaserMan raises his arm, points forward, and fires the original piercing
blue-white laser through the complete row.

- `LaserMan L/*`: 100 damage
- `LaserMan EX L`: 150 damage
- `LaserMan SP L`: 200 damage

Hold a direction while LaserMan raises his arms to add the original command
effect. This works for Base, EX, and SP: Up resets Attack, Rapid, and Charge to
level 1; Down disables Super Armor, Air Shoes, Float Shoes, Under Shirt, and B+Left
abilities; Right restores the standard charge shot without overwriting an
active Cross charge shot; Left permanently reduces the target's Custom Screen
selection by one chip, to a minimum of two. Command effects require the beam to
hit the target, so a miss does not alter the target.

Base uses the native red-background menu palette. EX keeps the base LaserMan
foreground and changes only its five background entries to green. SP uses the
native yellow-background SP palette. All variants use the base battle palette.

### Giga Chips

#### Double Beast

Double Beast's behavior was retained by the English engine, but its menu
resources were removed. Both editions restore the Japanese icon, library art,
palette, and description.

#### Gregar

The Japanese-only `Gregar X` is restored as a 99 MB, Null-element Giga chip.
It uses Gregar's original chip art and full summon sprite, including the
falling-rock and scorching-breath sequence, and returns to battle normally.

#### Falzar

The Japanese-only `Falzar X` is restored as a 99 MB, Null-element Giga chip.
It uses Falzar's original chip art, palette, full summon sprite, Strike
Feathers, Sonic Wave, and tornado sequence, and returns to battle normally.
Its 100-damage Strike Feathers also receive the activating chip's attack bonus.

#### Black Weapon

**Replaces:** Delta Ray Edge

**Gregar GigaChip.**

BN4 Black Weapon leaves Bass and Bass Anomaly unchanged. The 64 MB,
Null-element Giga chip uses code B, sets Buster Attack to level 10 and Rapid
and Charge to level 5, and drains 1 HP every 6 active battle frames without
reducing the user below 1 HP.

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

BN5 Bug Charge replaces Bug Rise Sword, and BN6 Bug Fix is restored. The 77
MB, Null-element Giga chip uses code B. It fires one 200-damage Gospel shot plus
one shot per active battle-bug type it clears, including BN6's Custom Screen
bug and its latched runtime state. The stationary Gospel remains for the
complete firing sequence. Gregar uses the imported Bug Charge chip art; Falzar
retains the inaccessible Bug Rise Sword slot art.

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
