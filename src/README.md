# Source registry

Gameplay features may be organized in any subdirectory here. Each feature has
one required implementation and one optional text file:

- `<name>.c` owns the implementation, chip records, resources, and registry
  declarations.
- `<name>.text.toml` optionally describes text archive replacements.

There are no package manifests. The C implementation is the source of truth
for chip records, attacks, objects, sprites, songs, dependencies, and fixed
patches. A generated header supplies allocated attack constants to the final C
compilation.

The shared native-call ABI veneers are in `abi.c`; direct runtime helpers are
in `runtime.c`. Global hooks are in `hooks.c` and `hooks.asm`. The linker layout
is `link.ld`, and public
ABI/runtime declarations are `abi.h` and `runtime.h` here too.
Native BN6 ABI names use the `EXE6_`/`Exe6`/`exe6_` namespace; the BN67
registry and resource macros in `runtime.h` use `BN67_`.
Native veneer names follow the recovered
[`MEGAMAN6_GXX_BR5E00.sym`](https://github.com/StraDaMa/Mega-Man-Battle-Network-6-Symbols/blob/main/MEGAMAN6_GXX_BR5E00.sym)
labels, with source-file prefixes removed and CamelCase normalized to the
project's `exe6_lower_snake_case` convention.

## Battle-sprite priority

`exe6_obj_char_init()` initializes sprites at OAM priority 2, the native battle
layer behind the Custom gauge and HUD. Any code that sets a battle sprite's
priority explicitly must use `EXE6_OBJ_PRIORITY_BATTLE`; priorities 0 and 1 can
draw over battle UI.

## Hit-effect visuals

`exe6_battle_hit_hit_mark_set()` writes the one-byte visual selector at
`Exe6HitFields.hit_effect` (`+0x09`). On contact,
`exe6_battle_hit_hit_mark_check()` reads it and creates the corresponding
impact animation. The selector does **not** define damage, element, status, or
special hit behavior such as SearchMan's trap deletion; those properties
come from the hit setup and the attack object's other fields.

The native BN6 hit-effect table contains these selectors in both editions:

| ID | ABI name | Impact animation |
| ---: | --- | --- |
| `0x00` | `EXE6_HIT_EFFECT_NORMAL` | normal white impact |
| `0x01` | `EXE6_HIT_EFFECT_FIRE` | fire impact |
| `0x02` | `EXE6_HIT_EFFECT_AQUA` | aqua impact |
| `0x03` | `EXE6_HIT_EFFECT_ELEC` | electric impact |
| `0x04` | `EXE6_HIT_EFFECT_WOOD` | wood impact |
| `0x05` | `EXE6_HIT_EFFECT_CHARGE_SHOT` | charge-shot spark |
| `0x06` | `EXE6_HIT_EFFECT_SMALL_IMPACT` | small orange/yellow impact |
| `0x07` | `EXE6_HIT_EFFECT_EXPLOSION` | explosion with debris |
| `0x08` | `EXE6_HIT_EFFECT_PING` | cyan ring/bubble |
| `0x09` | `EXE6_HIT_EFFECT_CHIP_DELETE` | chip-delete ping/slash |
| `0x0A` | `EXE6_HIT_EFFECT_BREAK` | break impact |
| `0x0B` | `EXE6_HIT_EFFECT_LARGE_EXPLOSION` | large explosion |
| `0x0C` | `EXE6_HIT_EFFECT_CHARGE_SHOT_PRIORITY_2` | charge-shot spark at native priority/layer 2 |
| `0x0D` | `EXE6_HIT_EFFECT_BAT` | bat burst |
| `0x0E` | `EXE6_HIT_EFFECT_UNINSTALL` | Uninstall shatter |
| `0x0F` | `EXE6_HIT_EFFECT_UNINSTALL_ALT` | native visual alias of `0x0E` |
| `0xFF` | `EXE6_HIT_EFFECT_NONE` | no contact animation |

## C declarations

Put declarations beside the implementation they register:

```c
#include "runtime.h"

BN67_USE_SONG(common_navi_summon_song);
BN67_SPRITE(searchman_battle_sprite, "build/searchman_battle_sprite.bin");
BN67_SONG(
    searchman_fire_song,
    BN67_PCM(
        searchman_fire,
        0x40,
        0x08,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xB1\n",
        "build/searchman_fire_sample.bin"
    )
);
BN67_PATCH_POINTER(0x08012010, searchman_data);

BN67_ENEMY(searchman_actor_main)
{
    /* `self` is the current object. */
}

BN67_SUMMON_ATTACK(0x107, searchman_attack_main)
{
    /* Native attack arguments are available by name here. */
}
```

`BN67_DUST_SPRITE(archive)` adds a registered sprite to the compiler-owned
DustCross ammo selector table. Use `BN67_DUST_KIND(archive)` when calling
`exe6_cube_set_dust_suikomi_efc`; the compiler allocates its four-bit ammo kind.
Target-specific table references and any safely reclaimed native aliases live
in the edition config instead of individual chip sources. The sprite may be
declared with either `BN67_SPRITE` or `BN67_FIXED_SPRITE`.

```c
BN67_SPRITE(rook_battle_sprite, "build/rook_battle_sprite.bin");
BN67_DUST_SPRITE(rook_battle_sprite);

exe6_cube_set_dust_suikomi_efc(BN67_DUST_KIND(rook_battle_sprite));
```

Use `BN67_FIXED_DUST_SPRITE(kind, archive)` when a restored sprite must replace
an existing native ammo kind without changing the code that selects that kind.

`BN67_FIELD_OBJECT(archive, animation, palette, shadow)` allocates the separate
native `NameID` used when chips such as JunkMan and BlizzardBall inspect or
reconstruct an obstacle. The declaration must refer to a `BN67_SPRITE` in the
same source package. Its final three arguments are byte literals describing the
sprite state JunkMan should throw. Store the generated ID in the persistent
object after it is created:

```c
BN67_FIELD_OBJECT(rook_battle_sprite, 4, 0, 1);

obj->name_id = (uint16_t)BN67_FIELD_OBJECT_ID(rook_battle_sprite);
```

Field-object IDs are not object-class lifecycle IDs. The compiler preserves the
native `0xCD`-through-`0xEB` render records, allocates imported obstacles from
`0xEC` upward, and relocates the engine lookup table configured for each target.

Target-specific pointer addresses use `BN67_PATCH_POINTER` behind the normal C
preprocessor; the metadata pass receives the same definition as the final C
build.

Use `BN67_PATCH_THUMB_POINTER(address, symbol)` for native function-pointer
tables; it writes `symbol + 1` so the indirect branch remains in Thumb state.

`BN67_PATCH_SECTION(address, relay_address, symbol)` replaces six bytes of
native instructions with a Thumb call through an eight-byte, word-aligned relay
at `relay_address`. That relay must be dead ROM within Thumb `bl` range of the
patch site. The target is entered with the original `r1` pushed on the stack;
it must pop `r1`, own any displaced instructions, and continue or return
according to that native call site. FolderBack uses the original class-1 table
after the registry relocates that table and its reference.

`BN67_PATCH_LINKED_CALL(source, offset, target)` replaces a Thumb call inside a
routine linked into the expanded gameplay image. Both symbols must live in that
image and remain within Thumb `bl` range. Unlike a fixed section patch, this
form needs no relay and does not alter registers or preserve displaced code.

The attack macros name the native family lifecycle they register:

- `BN67_PERSISTENT_ATTACK` uses family `0x15`. Its C function returns the
  spawned controller/effect object (or `NULL`), which the native wrapper can
  track after the callback. Its sixth argument is packed `chip_data`.
- `BN67_SUMMON_ATTACK` uses family `0x1B` and receives the summon manager's
  completion pointer as its sixth argument.
- `BN67_EPHEMERAL_ATTACK` uses family `0x1C`. Its callback return is ignored,
  its `attack` argument is already resolved with the activation bonus, and its
  sixth argument is the owner's signed 16.16 `z` coordinate.

`BN67_ATTACK` remains a source-compatible alias for
`BN67_PERSISTENT_ATTACK`. Here, persistent and ephemeral describe the native
wrapper lifecycle; an ephemeral callback may still spawn an independently
managed object.

`BN67_FIXED_OBJECT(class, id, main)` and
`BN67_FIXED_ATTACK(family, subfamily, main)` replace entries inside extracted
native table prefixes instead of appending new IDs. They are reserved for
restoring removed routines whose machine code still depends on its original
hard-coded object or attack selector. `BN67_FIXED_SPRITE(group, index, archive,
path)` does the same for a removed native sprite slot. Use
`BN67_FIXED_COMPRESSED_SPRITE` when the original table pointer has bit 31 set;
the engine uses that flag to decompress the archive before loading it.

The family selects one of these native ABIs and the subfamily is an 8-bit
index into that family's function table. The compiler relocates each
configured native prefix, appends registered attacks, and writes the resulting
family/subfamily selectors into the generated C constants. The first attack
macro argument is a representative chip ID that explicitly connects the attack
to a `BN67_CHIP_RECORD` in the same package; the compiler rejects a missing ID.

Object and attack entry points follow `<package>_<name>_main`; sprite archives
follow `<package>_<name>_sprite`; and songs follow `<package>_<name>_song`. The
compiler rejects labels that do not match those conventions. Object and attack
macros combine registration, the native ABI veneer, and the C implementation.
Every object and attack macro exposes the native `r4` value by value as
`Exe6ObjSpawnParameters spawn_parameters`. It is a four-byte struct, not a
pointer: its `variant`, `subvariant`, `animation_state`, and `removal_state`
fields are copied to new-object offsets `+0x04` through `+0x07`.
`BN67_SPRITE` and `BN67_SONG` likewise combine registration with the resource
definition; `BN67_PCM` supplies the standard PCM song body.

Use the allocated values in ordinary C expressions:

```c
Exe6Obj *reticle = exe6_efc_open(
    BN67_OBJ_ID(searchman_reticle_main),
    exe6_obj_spawn_with_variant(actor->variant)
);
exe6_obj_char_init(
    0x80,
    BN67_SPRITE_GROUP(searchman_battle_sprite),
    BN67_SPRITE_ID(searchman_battle_sprite)
);
exe6_sound_req(BN67_SONG_ID(searchman_fire_song));
```

Shared resources are registered by their implementation source.
`BN67_USE_SONG(common_navi_summon_song)` declares the link-time selector in a
consumer without adding another song-table entry.

## ELF metadata and link-time values

`compile_c_metadata.py` compiles every gameplay package source with
`BN67_METADATA_ONLY` and the target's normal preprocessor definitions. The
registration and definition macros emit ordered `__bn67_meta__...` symbols into
each ELF object. The script reads those symbols with `arm-none-eabi-nm`; it does
not parse C source text.

`compile_registry.py` receives one config file and one extracted symbol list.
It does not know target or edition names. Each invocation validates declarations,
allocates that config's registry slots, and writes separate target artifacts:

- `build/registry-<target>.generated.asm` for ROM hooks and tables;
- `build/text-replacements-<target>.generated.json` for text edits;
- `build/registry-values-<target>.generated.ld` for C-visible absolute selector
  symbols;
- `build/registry-values-<target>.generated.h` for C integer constants used by
  embedded chip records.

An object's class selects its native allocator and lifecycle table; its ID is
the 8-bit index within that table. The compiler relocates configured class
tables to 256 entries and resolves `BN67_OBJ_ID` directly, so custom objects
do not need an extra runtime discriminator field.

The final C link resolves `BN67_OBJ_ID`, `BN67_SPRITE_ID`,
`BN67_SPRITE_GROUP`, `BN67_DUST_KIND`, `BN67_FIELD_OBJECT_ID`, `BN67_SONG_ID`,
and `BN67_SONG_GROUP` from that linker file. Metadata records are not included
in the final gameplay binary. NCPs use their declared numeric offsets directly
and therefore do not need generated selector symbols.

## Chip definitions

`BN67_CHIP_RECORD(chip_id)` declares a complete `Exe6ChipRecord` C initializer.
The macro emits registration metadata and keeps the 44-byte record in its own
linked read-only section. The generated Armips registry copies those bytes from
`gameplay-<target>.bin` over `chip_table + chip_id * 0x2C` during final ROM
assembly. Artwork pointers therefore receive normal C/ELF relocations.

Use `BN67_ATTACK_FAMILY(main)` and `BN67_ATTACK_SUBFAMILY(main)` for a custom
attack's allocated selector bytes. `Exe6ChipRecord.behavior.object_spawn` has
the named `variant`, `subvariant`, `animation_state`, and `removal_state`
fields. Use `#if FALZAR` for edition-specific values. Every field must be
specified deliberately because this is a complete replacement, not a partial
native-record patch. `alphabetical_sort` may be zero: the final sort pass
regenerates it from the completed name archives.

Text replacements live in an optional sibling `<name>.text.toml`. Archive names
are direct top-level tables; there is no `text.` prefix:

```toml
[chip-names-1]
"0x31" = "BugCharg"

[chip-descriptions-1]
"0x31" = "All your\nbugs will\nattack!"
```

NCP descriptions use the NaviCust menu's three-line script format and are
recompressed automatically. Program names should fit the menu's eight-glyph
field.

NCP piece and effect slots are compiler-owned too. Like `BN67_CHIP_RECORD`,
`BN67_NCP` takes an explicit table offset. Each declaration supplies the effect
handler, bug type, plus-part flag, and four physical color slots (`0xFF`
reserves an unused slot). Shape resources follow the
`<label>_uncompressed_shape` and `<label>_compressed_shape` convention. Native
replacements are written into the original table in place so game tools that
read the canonical table address see the replacement; offsets in the custom
range cause the compiler to relocate and extend the table.

NCP names and descriptions use that same numeric offset, just as chip text uses
numeric archive offsets. The registry requires both entries at the declared NCP
offset:

```toml
[ncp-names]
"0x16" = "BeastT+1"

[ncp-descriptions]
"0x16" = "BeastOut\neffect\n+1 turn"
```

Allocation is deterministic: chips and NCPs use explicit offsets, attacks are
ordered by their representative chip IDs, and dynamically allocated resources
sort source paths while retaining ELF declaration order. Capacity overflow,
duplicate records, missing text, and exhausted tables are build errors.

Run both compiler stages directly with:

```sh
python3 compile_c_metadata.py --define FALZAR=0 \
  --output build/registry-metadata-gregar.generated.json
python3 compile_registry.py config.gregar.toml \
  --metadata build/registry-metadata-gregar.generated.json
```

The normal Make pipeline runs them automatically.
