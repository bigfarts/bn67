# Source registry

Every gameplay feature is a flat pair of files in this directory:

- `<name>.c` owns the implementation, resources, and registry declarations.
- `<name>.defs.toml` optionally describes semantic chip-record and text edits.

There are no package manifests or generated C headers. The C implementation is
the source of truth for attacks, objects, sprites, songs, dependencies, and
pointer patches.

The shared native-call ABI veneers are in `abi.c`; direct runtime helpers are
in `runtime.c`. The linker layout is `link.ld`, and public
ABI/runtime declarations are `abi.h` and `runtime.h` here too.

## C declarations

Put declarations beside the implementation they register:

```c
#include "runtime.h"

BN6_INCLUDE(common);
BN6_USE_SONG(CommonNaviSummonSong);
BN6_SPRITE(searchman_battle_sprite, "build/searchman-battle-sprite.bin");
BN6_SONG(SearchmanFireSong);
BN6_POINTER_PATCH(0x08012010, SearchmanData);

BN6_OBJECT1(searchman_actor_main)
{
    /* `self` is the current object. */
}

BN6_ATTACK(0x107, searchman_attack_main)
{
    /* Native attack arguments are available by name here. */
}
```

`BN6_ATTACK` names the representative chip whose family, subfamily, and
counter marker select the attack route. Other versions of the same attack can
share that route without being repeated in C.

Object and attack entry points follow `<package>_<name>_main`; sprite archives
follow `<package>_<name>_sprite`; and songs follow `<Package><Name>Song`. The
compiler rejects labels that do not match those conventions. Object and attack
macros combine registration, the native ABI veneer, and the C implementation.
Every object macro exposes the native `r4` value as `spawn_argument`.

Use the allocated values in ordinary C expressions:

```c
self->kind = BN6_OBJECT_KIND(searchman_reticle_main);
bn6_self_sprite_load(
    0x80,
    BN6_SPRITE_GROUP(searchman_battle_sprite),
    BN6_SPRITE_ID(searchman_battle_sprite)
);
bn6_play_sound(BN6_SONG_ID(SearchmanFireSong));
```

Shared resources are registered once. `BN6_INCLUDE(common)` orders the shared
implementation first, while `BN6_USE_SONG(CommonNaviSummonSong)` declares the
link-time selector without adding another song-table entry.

## ELF metadata and link-time values

`compile_c_metadata.py` compiles every gameplay package source with
`BN6_METADATA_ONLY`. The registration and definition macros emit ordered
`__bn6_meta__...` symbols into each ELF object. The script reads those symbols
with `arm-none-eabi-nm`; it does not parse C source text.

`compile_registry.py` consumes the extracted symbol list, checks dependencies,
allocates registry slots, and writes:

- `build/registry.generated.asm` for ROM hooks and tables;
- `build/text-replacements.generated.json` for text edits;
- `build/registry-values.generated.ld` for C-visible absolute selector symbols.

The final C link resolves `BN6_OBJECT_KIND`, `BN6_SPRITE_ID`,
`BN6_SPRITE_GROUP`, `BN6_SONG_ID`, and `BN6_SONG_GROUP` from that linker file.
Metadata records are not included in the final gameplay binary.

## Chip definitions

The optional `<name>.defs.toml` is deliberately un-namespaced. Its top-level
keys are complete BN6 chip IDs:

```toml
[chips."0x131"]
name = "BugCharg"
description = ["All your", "bugs will", "attack!"]
codes = ["B"]
rarity = 4
element = "null"
class = "giga"
mb = 77
power = 200
behavior = { counter_settings = 0x8B, family = 0x15, subfamily = 0x26, parameters = [0, 0, 0, 0] }
gregar = { behavior = { effect_flags = 0x41 } }
falzar = { behavior = { effect_flags = 0x01 } }
```

Unrelated text replacements use a separate archive/index namespace:

```toml
[text.chip-names-1]
"0x31" = "BugCharg"

[text.chip-descriptions-1]
"0x31" = ["All your", "bugs will", "attack!"]
```

Supported common fields are `codes`, `attack_element`, `rarity`, `element`,
`class`, `mb`, `power`, `behavior`, `library`, and `artwork`. The `gregar` and
`falzar` tables override only edition-specific values. Omitted values preserve
the native chip record.

Allocation is deterministic: included sources are visited first, source paths
are sorted, and declarations retain their ELF symbol order. Capacity overflow,
duplicate registrations, missing definitions, invalid routes, and include
cycles are build errors.

Run both compiler stages directly with:

```sh
python3 compile_c_metadata.py
python3 compile_registry.py
```

The normal Make pipeline runs them automatically.
