# Source registry

Gameplay features may be organized in any subdirectory here. Each feature has
one required implementation and one optional text file:

- `<name>.c` owns the implementation, chip records, resources, and registry
  declarations.
- `<name>.text.toml` optionally describes text archive replacements.

There are no package manifests. The C implementation is the source of truth
for chip records, attacks, objects, sprites, songs, dependencies, and pointer
patches. A generated header supplies allocated attack constants to the final C
compilation.

The shared native-call ABI veneers are in `abi.c`; direct runtime helpers are
in `runtime.c`. Global hooks are in `hooks.c` and `hooks.asm`. The linker layout
is `link.ld`, and public
ABI/runtime declarations are `abi.h` and `runtime.h` here too.
Native veneer names follow the recovered
[`MEGAMAN6_GXX_BR5E00.sym`](https://github.com/StraDaMa/Mega-Man-Battle-Network-6-Symbols/blob/main/MEGAMAN6_GXX_BR5E00.sym)
labels, with source-file prefixes removed and CamelCase normalized to the
project's `exe6_lower_snake_case` convention.

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

EXE6_USE_SONG(common_navi_summon_song);
EXE6_SPRITE(searchman_battle_sprite, "build/searchman-battle-sprite.bin");
EXE6_SONG(
    searchman_fire_song,
    EXE6_PCM(
        searchman_fire,
        0x40,
        0x08,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xB1\n",
        "build/searchman-fire-sample.bin"
    )
);
EXE6_PATCH_POINTER(0x08012010, searchman_data);

EXE6_ENEMY(searchman_actor_main)
{
    /* `self` is the current object. */
}

EXE6_SUMMON_ATTACK(0x107, searchman_attack_main)
{
    /* Native attack arguments are available by name here. */
}
```

Target-specific pointer addresses use the same macro behind the normal C
preprocessor; the metadata pass receives the same definition as the final C
build:

```c
#if FALZAR
EXE6_PATCH_POINTER(0x080E9990, signalred_dust_sprite_table);
#else
EXE6_PATCH_POINTER(0x080EACD0, signalred_dust_sprite_table);
#endif
```

The attack macros name the native family lifecycle they register:

- `EXE6_PERSISTENT_ATTACK` uses family `0x15`. Its C function returns the
  spawned controller/effect object (or `NULL`), which the native wrapper can
  track after the callback. Its sixth argument is packed `chip_data`.
- `EXE6_SUMMON_ATTACK` uses family `0x1B` and receives the summon manager's
  completion pointer as its sixth argument.
- `EXE6_EPHEMERAL_ATTACK` uses family `0x1C`. Its callback return is ignored,
  its `attack` argument is already resolved with the activation bonus, and its
  sixth argument is the owner's signed 16.16 `z` coordinate.

`EXE6_ATTACK` remains a source-compatible alias for
`EXE6_PERSISTENT_ATTACK`. Here, persistent and ephemeral describe the native
wrapper lifecycle; an ephemeral callback may still spawn an independently
managed object.

The family selects one of these native ABIs and the subfamily is an 8-bit
index into that family's function table. The compiler relocates each
configured native prefix, appends registered attacks, and writes the resulting
family/subfamily selectors into the generated C constants. The first attack
macro argument is a representative chip ID that explicitly connects the attack
to an `EXE6_CHIP_RECORD` in the same package; the compiler rejects a missing ID.

Object and attack entry points follow `<package>_<name>_main`; sprite archives
follow `<package>_<name>_sprite`; and songs follow `<package>_<name>_song`. The
compiler rejects labels that do not match those conventions. Object and attack
macros combine registration, the native ABI veneer, and the C implementation.
Every object and attack macro exposes the native `r4` value by value as
`Exe6ObjSpawnParameters spawn_parameters`. It is a four-byte struct, not a
pointer: its `variant`, `subvariant`, `animation_state`, and `removal_state`
fields are copied to new-object offsets `+0x04` through `+0x07`.
`EXE6_SPRITE` and `EXE6_SONG` likewise combine registration with the resource
definition; `EXE6_PCM` supplies the standard PCM song body.

Use the allocated values in ordinary C expressions:

```c
Exe6Obj *reticle = exe6_efc_open(
    EXE6_OBJ_ID(searchman_reticle_main),
    exe6_obj_spawn_with_variant(actor->variant)
);
exe6_obj_char_init(
    0x80,
    EXE6_SPRITE_GROUP(searchman_battle_sprite),
    EXE6_SPRITE_ID(searchman_battle_sprite)
);
exe6_sound_req(EXE6_SONG_ID(searchman_fire_song));
```

Shared resources are registered by their implementation source.
`EXE6_USE_SONG(common_navi_summon_song)` declares the link-time selector in a
consumer without adding another song-table entry.

## ELF metadata and link-time values

`compile_c_metadata.py` compiles every gameplay package source with
`EXE6_METADATA_ONLY` and the target's normal preprocessor definitions. The
registration and definition macros emit ordered `__exe6_meta__...` symbols into
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
tables to 256 entries and resolves `EXE6_OBJ_ID` directly, so custom objects
do not need an extra runtime discriminator field.

The final C link resolves `EXE6_OBJ_ID`, `EXE6_SPRITE_ID`,
`EXE6_SPRITE_GROUP`, `EXE6_SONG_ID`, and `EXE6_SONG_GROUP` from that linker file.
Metadata records are not included in the final gameplay binary.

## Chip definitions

`EXE6_CHIP_RECORD(chip_id)` declares a complete `Exe6ChipRecord` C initializer.
The macro emits registration metadata and keeps the 44-byte record in its own
linked read-only section. The generated Armips registry copies those bytes from
`gameplay-<target>.bin` over `chip_table + chip_id * 0x2C` during final ROM
assembly. Artwork pointers therefore receive normal C/ELF relocations.

Use `EXE6_ATTACK_FAMILY(main)` and `EXE6_ATTACK_SUBFAMILY(main)` for a custom
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
"0x31" = ["All your", "bugs will", "attack!"]
```

Allocation is deterministic: attacks are ordered by their explicit
representative chip IDs, while other resources sort source paths and retain ELF
declaration order. Capacity overflow, duplicate chip records, missing records,
and exhausted tables are build errors.

Run both compiler stages directly with:

```sh
python3 compile_c_metadata.py --define FALZAR=0 \
  --output build/registry-metadata-gregar.generated.json
python3 compile_registry.py config.gregar.toml \
  --metadata build/registry-metadata-gregar.generated.json
```

The normal Make pipeline runs them automatically.
