#ifndef RUNTIME_H
#define RUNTIME_H

#include "abi.h"

#define EXE6_BATTLE_STATE \
    ((Exe6BattleState *)(uintptr_t)0x0203CA70u)
#define EXE6_CHIP_QUEUE \
    ((Exe6ChipQueue *)(uintptr_t)0x0203CDB0u)

#define EXE6_LINK_OBJ_ID(main) EXE6_JOIN(__exe6_object_id_, main)
#define EXE6_LINK_SPRITE_ID(archive) EXE6_JOIN(__exe6_sprite_id_, archive)
#define EXE6_LINK_SPRITE_GROUP(archive) EXE6_JOIN(__exe6_sprite_group_, archive)
#define EXE6_LINK_SONG_ID(archive) EXE6_JOIN(__exe6_song_id_, archive)
#define EXE6_LINK_SONG_GROUP(archive) EXE6_JOIN(__exe6_song_group_, archive)

/* Metadata compilation runs before attack slots are allocated. */
#ifndef EXE6_ATTACK_FAMILY
#define EXE6_ATTACK_FAMILY(main) 0
#define EXE6_ATTACK_SUBFAMILY(main) 0
#endif

#define EXE6_OBJ_ID(main) \
    __extension__ ({ \
        extern const uint8_t EXE6_LINK_OBJ_ID(main)[]; \
        (uint32_t)(uintptr_t)EXE6_LINK_OBJ_ID(main); \
    })
#define EXE6_SPRITE_ID(archive) \
    ((uint32_t)(uintptr_t)EXE6_LINK_SPRITE_ID(archive))
#define EXE6_SPRITE_GROUP(archive) \
    ((uint32_t)(uintptr_t)EXE6_LINK_SPRITE_GROUP(archive))
#define EXE6_SONG_ID(archive) \
    ((uint32_t)(uintptr_t)EXE6_LINK_SONG_ID(archive))
#define EXE6_SONG_GROUP(archive) \
    ((uint32_t)(uintptr_t)EXE6_LINK_SONG_GROUP(archive))

/*
 * Package registries live in C.  The metadata build turns each declaration
 * below into a one-byte, ordered ELF symbol.  The final gameplay build already
 * has the generated selectors, so the declarations intentionally emit no ROM
 * data there.
 */
#ifdef EXE6_METADATA_ONLY
#define EXE6_METADATA_RECORD(kind, payload) \
    __asm__( \
        ".section .exe6_metadata,\"a\",%progbits\n" \
        ".global __exe6_meta__" kind "__" payload "\n" \
        ".type __exe6_meta__" kind "__" payload ",%object\n" \
        "__exe6_meta__" kind "__" payload ":\n" \
        ".byte 0\n" \
        ".size __exe6_meta__" kind "__" payload ",1\n" \
        ".previous\n" \
    )
#else
#define EXE6_METADATA_RECORD(kind, payload)
#endif

#define EXE6_OBJ_BODY(obj_class, main) \
    EXE6_METADATA_RECORD( \
        "object", \
        EXE6_STRINGIFY(obj_class) "__" EXE6_STRINGIFY(main) \
    ); \
    static USED void EXE6_JOIN(main, _fn)( \
        Exe6Obj *self, \
        Exe6ObjSpawnParameters spawn_parameters __attribute__((unused)) \
    ); \
    EXE6_EXPORT_OBJ(main, EXE6_JOIN(main, _fn)) \
    static USED void EXE6_JOIN(main, _fn)( \
        Exe6Obj *self, \
        Exe6ObjSpawnParameters spawn_parameters __attribute__((unused)) \
    )

#define EXE6_ENEMY(main) EXE6_OBJ_BODY(1, main)
#define EXE6_SHELL(main) EXE6_OBJ_BODY(3, main)
#define EXE6_EFFECT(main) EXE6_OBJ_BODY(4, main)

#define EXE6_USE_SONG(archive) \
    extern const uint8_t EXE6_LINK_SONG_ID(archive)[]; \
    extern const uint8_t EXE6_LINK_SONG_GROUP(archive)[]

#define EXE6_ATTACK_BODY( \
    kind, chip_id, main, export, return_type, context_type, context_name \
) \
    EXE6_METADATA_RECORD( \
        kind, \
        EXE6_STRINGIFY(chip_id) "__" EXE6_STRINGIFY(main) \
    ); \
    static USED return_type EXE6_JOIN(main, _fn)( \
        uint32_t block_x, \
        uint32_t block_y, \
        uint32_t parameter, \
        Exe6Obj *owner, \
        uint32_t attack, \
        context_type context_name, \
        Exe6ObjSpawnParameters spawn_parameters \
    ); \
    export(main, EXE6_JOIN(main, _fn)) \
    static USED return_type EXE6_JOIN(main, _fn)( \
        uint32_t block_x, \
        uint32_t block_y, \
        uint32_t parameter, \
        Exe6Obj *owner, \
        uint32_t attack, \
        context_type context_name, \
        Exe6ObjSpawnParameters spawn_parameters \
    )

#define EXE6_PERSISTENT_ATTACK(chip_id, main) \
    EXE6_ATTACK_BODY( \
        "persistent_attack", \
        chip_id, \
        main, \
        EXE6_EXPORT_ATTACK, \
        Exe6Obj *, \
        uint32_t, \
        chip_data \
    )

#define EXE6_SUMMON_ATTACK(chip_id, main) \
    EXE6_ATTACK_BODY( \
        "summon_attack", \
        chip_id, \
        main, \
        EXE6_EXPORT_ATTACK, \
        void, \
        uint8_t *, \
        completion \
    )

#define EXE6_EPHEMERAL_ATTACK(chip_id, main) \
    EXE6_ATTACK_BODY( \
        "ephemeral_attack", \
        chip_id, \
        main, \
        EXE6_EXPORT_EPHEMERAL_ATTACK, \
        void, \
        int32_t, \
        z \
    )

/* Source compatibility for packages written before the ABI names were split. */
#define EXE6_ATTACK(chip_id, main) EXE6_PERSISTENT_ATTACK(chip_id, main)

#define EXE6_PATCH_POINTER(address, symbol) \
    EXE6_METADATA_RECORD( \
        "pointer", \
        EXE6_STRINGIFY(address) "__" EXE6_STRINGIFY(symbol) \
    )

#define EXE6_PATCH_SECTION(address, symbol) \
    EXE6_METADATA_RECORD( \
        "section", \
        EXE6_STRINGIFY(address) "__" EXE6_STRINGIFY(symbol) \
    )

/*
 * A complete native 0x2C chip record. The linked record remains in the C
 * image; the final ROM assembly copies it over the chip ID's native slot.
 */
#define EXE6_CHIP_RECORD_SYMBOL(chip_id) \
    EXE6_JOIN(exe6_chip_record_, chip_id)

#define EXE6_CHIP_RECORD(chip_id) \
    EXE6_METADATA_RECORD( \
        "chip", \
        EXE6_STRINGIFY(chip_id) \
    ); \
    USED const Exe6ChipRecord EXE6_CHIP_RECORD_SYMBOL(chip_id) \
        __attribute__((section(".rodata." EXE6_STRINGIFY(chip_id)), aligned(4))) =

#ifdef EXE6_METADATA_ONLY
#define EXE6_ASM_RESOURCE(name, contents) extern const uint8_t name[]
#define EXE6_INCBIN(name, path) extern const uint8_t name[]
#define EXE6_RESOURCE_ALIAS(alias, target) extern const uint8_t alias[]
#else
/* Keep package-owned binary data next to the C code that uses it. */
#define EXE6_ASM_RESOURCE(name, contents) \
    extern const uint8_t name[]; \
    __asm__( \
        ".section .rodata." EXE6_STRINGIFY(name) ",\"a\",%progbits\n" \
        ".balign 4\n" \
        ".global " EXE6_STRINGIFY(name) "\n" \
        ".type " EXE6_STRINGIFY(name) ",%object\n" \
        EXE6_STRINGIFY(name) ":\n" \
        contents \
        ".size " EXE6_STRINGIFY(name) ",.-" EXE6_STRINGIFY(name) "\n" \
        ".previous\n" \
    )

#define EXE6_INCBIN(name, path) \
    EXE6_ASM_RESOURCE(name, ".incbin \"" path "\"\n")

#define EXE6_RESOURCE_ALIAS(alias, target) \
    extern const uint8_t alias[]; \
    __asm__( \
        ".global " EXE6_STRINGIFY(alias) "\n" \
        ".type " EXE6_STRINGIFY(alias) ",%object\n" \
        ".set " EXE6_STRINGIFY(alias) "," EXE6_STRINGIFY(target) "\n" \
    )

#endif

#define EXE6_PCM(prefix, priority, voice, track, sample_path) \
    ".byte 1,0," EXE6_STRINGIFY(priority) ",0\n" \
    ".long " EXE6_STRINGIFY(EXE6_JOIN(prefix, _voicegroup)) "\n" \
    ".long " EXE6_STRINGIFY(EXE6_JOIN(prefix, _track)) "\n" \
    ".global " EXE6_STRINGIFY(EXE6_JOIN(prefix, _voicegroup)) "\n" \
    EXE6_STRINGIFY(EXE6_JOIN(prefix, _voicegroup)) ":\n" \
    ".byte " EXE6_STRINGIFY(voice) ",0x3C,0,0\n" \
    ".long " EXE6_STRINGIFY(EXE6_JOIN(prefix, _sample)) "\n" \
    ".byte 0xFF,0,0xFF,0\n" \
    ".global " EXE6_STRINGIFY(EXE6_JOIN(prefix, _track)) "\n" \
    EXE6_STRINGIFY(EXE6_JOIN(prefix, _track)) ":\n" \
    track \
    ".balign 4\n" \
    ".global " EXE6_STRINGIFY(EXE6_JOIN(prefix, _sample)) "\n" \
    EXE6_STRINGIFY(EXE6_JOIN(prefix, _sample)) ":\n" \
    ".incbin \"" sample_path "\"\n"

#define EXE6_SONG(archive, contents) \
    extern const uint8_t EXE6_LINK_SONG_ID(archive)[]; \
    extern const uint8_t EXE6_LINK_SONG_GROUP(archive)[]; \
    EXE6_ASM_RESOURCE(archive, contents); \
    EXE6_METADATA_RECORD("song", EXE6_STRINGIFY(archive))

#define EXE6_SPRITE(archive, path) \
    extern const uint8_t EXE6_LINK_SPRITE_ID(archive)[]; \
    extern const uint8_t EXE6_LINK_SPRITE_GROUP(archive)[]; \
    EXE6_INCBIN(archive, path); \
    EXE6_METADATA_RECORD("sprite", EXE6_STRINGIFY(archive))

#endif
