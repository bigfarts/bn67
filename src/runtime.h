#ifndef BN67_RUNTIME_H
#define BN67_RUNTIME_H

#include "abi.h"

#define BN67_JOIN_INNER(left, right) left##right
#define BN67_JOIN(left, right) BN67_JOIN_INNER(left, right)
#define BN67_STRINGIFY_INNER(value) #value
#define BN67_STRINGIFY(value) BN67_STRINGIFY_INNER(value)

#define BN67_LINK_OBJ_ID(main) BN67_JOIN(__bn67_object_id_, main)
#define BN67_LINK_SPRITE_ID(archive) BN67_JOIN(__bn67_sprite_id_, archive)
#define BN67_LINK_SPRITE_GROUP(archive) BN67_JOIN(__bn67_sprite_group_, archive)
#define BN67_LINK_DUST_KIND(archive) BN67_JOIN(__bn67_dust_kind_, archive)
#define BN67_LINK_FIELD_OBJECT_ID(archive) \
    BN67_JOIN(__bn67_field_object_id_, archive)
#define BN67_LINK_SONG_ID(archive) BN67_JOIN(__bn67_song_id_, archive)
#define BN67_LINK_SONG_GROUP(archive) BN67_JOIN(__bn67_song_group_, archive)

/* Metadata compilation runs before attack slots are allocated. */
#ifndef BN67_ATTACK_FAMILY
#define BN67_ATTACK_FAMILY(main) 0
#define BN67_ATTACK_SUBFAMILY(main) 0
#endif

#define BN67_OBJ_ID(main) \
    __extension__ ({ \
        extern const uint8_t BN67_LINK_OBJ_ID(main)[]; \
        (uint32_t)(uintptr_t)BN67_LINK_OBJ_ID(main); \
    })
#define BN67_SPRITE_ID(archive) \
    ((uint32_t)(uintptr_t)BN67_LINK_SPRITE_ID(archive))
#define BN67_SPRITE_GROUP(archive) \
    ((uint32_t)(uintptr_t)BN67_LINK_SPRITE_GROUP(archive))
#define BN67_DUST_KIND(archive) \
    ((uint32_t)(uintptr_t)BN67_LINK_DUST_KIND(archive))
#define BN67_FIELD_OBJECT_ID(archive) \
    ((uint32_t)(uintptr_t)BN67_LINK_FIELD_OBJECT_ID(archive))
#define BN67_SONG_ID(archive) \
    ((uint32_t)(uintptr_t)BN67_LINK_SONG_ID(archive))
#define BN67_SONG_GROUP(archive) \
    ((uint32_t)(uintptr_t)BN67_LINK_SONG_GROUP(archive))

/*
 * Package registries live in C.  The metadata build turns each declaration
 * below into a one-byte, ordered ELF symbol.  The final gameplay build already
 * has the generated selectors, so the declarations intentionally emit no ROM
 * data there.
 */
#ifdef BN67_METADATA_ONLY
#define BN67_METADATA_RECORD(kind, payload) \
    __asm__( \
        ".section .bn67_metadata,\"a\",%progbits\n" \
        ".global __bn67_meta__" kind "__" payload "\n" \
        ".type __bn67_meta__" kind "__" payload ",%object\n" \
        "__bn67_meta__" kind "__" payload ":\n" \
        ".byte 0\n" \
        ".size __bn67_meta__" kind "__" payload ",1\n" \
        ".previous\n" \
    )
#else
#define BN67_METADATA_RECORD(kind, payload)
#endif

#define BN67_OBJ_BODY(obj_class, main) \
    BN67_METADATA_RECORD( \
        "object", \
        BN67_STRINGIFY(obj_class) "__" BN67_STRINGIFY(main) \
    ); \
    static USED void BN67_JOIN(main, _fn)( \
        Exe6Obj *self, \
        Exe6ObjSpawnParameters spawn_parameters __attribute__((unused)) \
    ); \
    EXE6_EXPORT_OBJ(main, BN67_JOIN(main, _fn)) \
    static USED void BN67_JOIN(main, _fn)( \
        Exe6Obj *self, \
        Exe6ObjSpawnParameters spawn_parameters __attribute__((unused)) \
    )

#define BN67_ENEMY(main) BN67_OBJ_BODY(1, main)
#define BN67_SHELL(main) BN67_OBJ_BODY(3, main)
#define BN67_EFFECT(main) BN67_OBJ_BODY(4, main)

/* Restore a removed native object-table slot with a linked Thumb routine. */
#define BN67_FIXED_OBJECT(obj_class, object_id, main) \
    BN67_METADATA_RECORD( \
        "fixed_object", \
        BN67_STRINGIFY(obj_class) "__" BN67_STRINGIFY(object_id) "__" \
            BN67_STRINGIFY(main) \
    )

#define BN67_USE_SONG(archive) \
    extern const uint8_t BN67_LINK_SONG_ID(archive)[]; \
    extern const uint8_t BN67_LINK_SONG_GROUP(archive)[]

#define BN67_ATTACK_BODY( \
    kind, chip_id, main, export, return_type, context_type, context_name \
) \
    BN67_METADATA_RECORD( \
        kind, \
        BN67_STRINGIFY(chip_id) "__" BN67_STRINGIFY(main) \
    ); \
    static USED return_type BN67_JOIN(main, _fn)( \
        uint32_t block_x, \
        uint32_t block_y, \
        uint32_t parameter, \
        Exe6Obj *owner, \
        uint32_t attack, \
        context_type context_name, \
        Exe6ObjSpawnParameters spawn_parameters \
    ); \
    export(main, BN67_JOIN(main, _fn)) \
    static USED return_type BN67_JOIN(main, _fn)( \
        uint32_t block_x, \
        uint32_t block_y, \
        uint32_t parameter, \
        Exe6Obj *owner, \
        uint32_t attack, \
        context_type context_name, \
        Exe6ObjSpawnParameters spawn_parameters \
    )

#define BN67_PERSISTENT_ATTACK(chip_id, main) \
    BN67_ATTACK_BODY( \
        "persistent_attack", \
        chip_id, \
        main, \
        EXE6_EXPORT_ATTACK, \
        Exe6Obj *, \
        uint32_t, \
        chip_data \
    )

#define BN67_SUMMON_ATTACK(chip_id, main) \
    BN67_ATTACK_BODY( \
        "summon_attack", \
        chip_id, \
        main, \
        EXE6_EXPORT_ATTACK, \
        void, \
        uint8_t *, \
        completion \
    )

#define BN67_EPHEMERAL_ATTACK(chip_id, main) \
    BN67_ATTACK_BODY( \
        "ephemeral_attack", \
        chip_id, \
        main, \
        EXE6_EXPORT_EPHEMERAL_ATTACK, \
        void, \
        int32_t, \
        z \
    )

/* Source compatibility for packages written before the ABI names were split. */
#define BN67_ATTACK(chip_id, main) BN67_PERSISTENT_ATTACK(chip_id, main)

/* Restore a removed entry inside one of the compiler-owned native attack
 * tables. The linked symbol is the original routine's Thumb entry point. */
#define BN67_FIXED_ATTACK(family, subfamily, main) \
    BN67_METADATA_RECORD( \
        "fixed_attack", \
        BN67_STRINGIFY(family) "__" BN67_STRINGIFY(subfamily) "__" \
            BN67_STRINGIFY(main) \
    )

/* Compile one NaviCust effect at an explicit logical table offset, matching
 * chip-record declarations. Each offset owns four physical pieces; 0xFF
 * reserves an unused color slot. */
#define BN67_NCP( \
    ncp_id, label, main, bug, plus_part, color0, color1, color2, color3 \
) \
    BN67_METADATA_RECORD( \
        "ncp", \
        BN67_STRINGIFY(ncp_id) "__" BN67_STRINGIFY(label) "__" \
            BN67_STRINGIFY(main) "__" BN67_STRINGIFY(bug) "__" \
            BN67_STRINGIFY(plus_part) "__" BN67_STRINGIFY(color0) "__" \
            BN67_STRINGIFY(color1) "__" BN67_STRINGIFY(color2) "__" \
            BN67_STRINGIFY(color3) \
    ); \
    static USED void BN67_JOIN(main, _fn)(void); \
    EXE6_EXPORT_NCP(main, BN67_JOIN(main, _fn)) \
    static USED void BN67_JOIN(main, _fn)(void)

#define BN67_PATCH_POINTER(address, symbol) \
    BN67_METADATA_RECORD( \
        "pointer", \
        BN67_STRINGIFY(address) "__" BN67_STRINGIFY(symbol) \
    )

/* Patch a native function-pointer table with the target's Thumb address. */
#define BN67_PATCH_THUMB_POINTER(address, symbol) \
    BN67_METADATA_RECORD( \
        "thumb_pointer", \
        BN67_STRINGIFY(address) "__" BN67_STRINGIFY(symbol) \
    )

/* Section targets are entered with the original r1 pushed on the stack. */
#define BN67_PATCH_SECTION(address, relay_address, symbol) \
    BN67_METADATA_RECORD( \
        "section", \
        BN67_STRINGIFY(address) "__" BN67_STRINGIFY(relay_address) "__" \
            BN67_STRINGIFY(symbol) \
    )

/* Replace a Thumb call inside a routine linked into the expanded code image. */
#define BN67_PATCH_LINKED_CALL(source, offset, target) \
    BN67_METADATA_RECORD( \
        "linked_call", \
        BN67_STRINGIFY(source) "__" BN67_STRINGIFY(offset) "__" \
            BN67_STRINGIFY(target) \
    )

/* Replace an adjacent `mov r1,group` / `mov r2,index` pair used by a native
 * sprite-load call with the compiler-allocated handle for an imported sprite. */
#define BN67_PATCH_SPRITE_LOAD(address, archive) \
    BN67_METADATA_RECORD( \
        "sprite_load", \
        BN67_STRINGIFY(address) "__" BN67_STRINGIFY(archive) \
    )

/*
 * A complete native 0x2C chip record. The linked record remains in the C
 * image; the final ROM assembly copies it over the chip ID's native slot.
 */
#define BN67_CHIP_RECORD_SYMBOL(chip_id) \
    BN67_JOIN(bn67_chip_record_, chip_id)

#define BN67_CHIP_RECORD(chip_id) \
    BN67_METADATA_RECORD( \
        "chip", \
        BN67_STRINGIFY(chip_id) \
    ); \
    USED const Exe6ChipRecord BN67_CHIP_RECORD_SYMBOL(chip_id) \
        __attribute__((section(".rodata." BN67_STRINGIFY(chip_id)), aligned(4))) =

#ifdef BN67_METADATA_ONLY
#define BN67_ASM_RESOURCE(name, contents) extern const uint8_t name[]
#define BN67_INCBIN(name, path) extern const uint8_t name[]
#define BN67_RESOURCE_ALIAS(alias, target) extern const uint8_t alias[]
#else
/* Keep package-owned binary data next to the C code that uses it. */
#define BN67_ASM_RESOURCE(name, contents) \
    extern const uint8_t name[]; \
    __asm__( \
        ".section .rodata." BN67_STRINGIFY(name) ",\"a\",%progbits\n" \
        ".balign 4\n" \
        ".global " BN67_STRINGIFY(name) "\n" \
        ".type " BN67_STRINGIFY(name) ",%object\n" \
        BN67_STRINGIFY(name) ":\n" \
        contents \
        ".size " BN67_STRINGIFY(name) ",.-" BN67_STRINGIFY(name) "\n" \
        ".previous\n" \
    )

#define BN67_INCBIN(name, path) \
    BN67_ASM_RESOURCE(name, ".incbin \"" path "\"\n")

#define BN67_RESOURCE_ALIAS(alias, target) \
    extern const uint8_t alias[]; \
    __asm__( \
        ".global " BN67_STRINGIFY(alias) "\n" \
        ".type " BN67_STRINGIFY(alias) ",%object\n" \
        ".set " BN67_STRINGIFY(alias) "," BN67_STRINGIFY(target) "\n" \
    )

#endif

#define BN67_PCM(prefix, priority, voice, track, sample_path) \
    ".byte 1,0," BN67_STRINGIFY(priority) ",0\n" \
    ".long " BN67_STRINGIFY(BN67_JOIN(prefix, _voicegroup)) "\n" \
    ".long " BN67_STRINGIFY(BN67_JOIN(prefix, _track)) "\n" \
    ".global " BN67_STRINGIFY(BN67_JOIN(prefix, _voicegroup)) "\n" \
    BN67_STRINGIFY(BN67_JOIN(prefix, _voicegroup)) ":\n" \
    ".byte " BN67_STRINGIFY(voice) ",0x3C,0,0\n" \
    ".long " BN67_STRINGIFY(BN67_JOIN(prefix, _sample)) "\n" \
    ".byte 0xFF,0,0xFF,0\n" \
    ".global " BN67_STRINGIFY(BN67_JOIN(prefix, _track)) "\n" \
    BN67_STRINGIFY(BN67_JOIN(prefix, _track)) ":\n" \
    track \
    ".balign 4\n" \
    ".global " BN67_STRINGIFY(BN67_JOIN(prefix, _sample)) "\n" \
    BN67_STRINGIFY(BN67_JOIN(prefix, _sample)) ":\n" \
    ".incbin \"" sample_path "\"\n"

#define BN67_SONG(archive, contents) \
    extern const uint8_t BN67_LINK_SONG_ID(archive)[]; \
    extern const uint8_t BN67_LINK_SONG_GROUP(archive)[]; \
    BN67_ASM_RESOURCE(archive, contents); \
    BN67_METADATA_RECORD("song", BN67_STRINGIFY(archive))

#define BN67_SPRITE(archive, path) \
    extern const uint8_t BN67_LINK_SPRITE_ID(archive)[]; \
    extern const uint8_t BN67_LINK_SPRITE_GROUP(archive)[]; \
    BN67_INCBIN(archive, path); \
    BN67_METADATA_RECORD("sprite", BN67_STRINGIFY(archive))

/* Replace a removed native sprite slot while preserving its original handle. */
#define BN67_FIXED_SPRITE(group, index, archive, path) \
    BN67_INCBIN(archive, path); \
    BN67_METADATA_RECORD( \
        "fixed_sprite", \
        BN67_STRINGIFY(group) "__" BN67_STRINGIFY(index) "__" \
            BN67_STRINGIFY(archive) \
    )

/* Preserve bit 31, which tells the native loader to decompress the archive. */
#define BN67_FIXED_COMPRESSED_SPRITE(group, index, archive, path) \
    BN67_INCBIN(archive, path); \
    BN67_METADATA_RECORD( \
        "fixed_compressed_sprite", \
        BN67_STRINGIFY(group) "__" BN67_STRINGIFY(index) "__" \
            BN67_STRINGIFY(archive) \
    )

/* Allocate a DustCross ammo kind for an imported or fixed sprite. */
#define BN67_DUST_SPRITE(archive) \
    extern const uint8_t BN67_LINK_DUST_KIND(archive)[]; \
    BN67_METADATA_RECORD("dust_sprite", BN67_STRINGIFY(archive))

/* Replace a native DustCross ammo kind with a registered sprite. */
#define BN67_FIXED_DUST_SPRITE(kind, archive) \
    extern const uint8_t BN67_LINK_DUST_KIND(archive)[]; \
    BN67_METADATA_RECORD( \
        "fixed_dust_sprite", \
        BN67_STRINGIFY(kind) "__" BN67_STRINGIFY(archive) \
    )

/*
 * Allocate the native NameID used by field-object consumers such as JunkMan
 * and BlizzardBall. The final three arguments are the animation, palette,
 * and shadow flag used when the engine reconstructs the object's sprite.
 */
#define BN67_FIELD_OBJECT(archive, animation, palette, shadow) \
    extern const uint8_t BN67_LINK_FIELD_OBJECT_ID(archive)[]; \
    BN67_METADATA_RECORD( \
        "field_object", \
        BN67_STRINGIFY(archive) "__" BN67_STRINGIFY(animation) "__" \
            BN67_STRINGIFY(palette) "__" BN67_STRINGIFY(shadow) \
    )

enum Bn67DeployablePlacementResult {
    BN67_DEPLOYABLE_PLACEMENT_CLEAR,
    BN67_DEPLOYABLE_PLACEMENT_INVALID,
    BN67_DEPLOYABLE_PLACEMENT_OCCUPIED,
};

enum Bn67DeployablePlacementResult bn67_deployable_placement_check(
    uint32_t block_x,
    uint32_t block_y
);

#endif
