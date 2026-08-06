#ifndef RUNTIME_H
#define RUNTIME_H

#include "abi.h"

Object *volatile *bn6_battle_units_for_side(uint32_t side);
Object *volatile *bn6_active_units_for_side(uint32_t side);
BattleContext *bn6_battle_context(void);
volatile uint8_t *bn6_battle_state(void);
volatile uint8_t *bn6_chip_queue(void);

#define CUSTOM_TYPE1_ID 0x31u
#define CUSTOM_TYPE3_ID 0x2Cu
#define CUSTOM_TYPE4_ID 0x17u

#define BN6_LINK_OBJECT(main) BN6_JOIN(__bn6_object_kind_, main)
#define BN6_LINK_SPRITE_ID(archive) BN6_JOIN(__bn6_sprite_id_, archive)
#define BN6_LINK_SPRITE_GROUP(archive) BN6_JOIN(__bn6_sprite_group_, archive)
#define BN6_LINK_SONG_ID(archive) BN6_JOIN(__bn6_song_id_, archive)
#define BN6_LINK_SONG_GROUP(archive) BN6_JOIN(__bn6_song_group_, archive)

#define BN6_OBJECT_KIND(main) \
    __extension__ ({ \
        extern const uint8_t BN6_LINK_OBJECT(main)[]; \
        (uint32_t)(uintptr_t)BN6_LINK_OBJECT(main); \
    })
#define BN6_SPRITE_ID(archive) \
    ((uint32_t)(uintptr_t)BN6_LINK_SPRITE_ID(archive))
#define BN6_SPRITE_GROUP(archive) \
    ((uint32_t)(uintptr_t)BN6_LINK_SPRITE_GROUP(archive))
#define BN6_SONG_ID(archive) \
    ((uint32_t)(uintptr_t)BN6_LINK_SONG_ID(archive))
#define BN6_SONG_GROUP(archive) \
    ((uint32_t)(uintptr_t)BN6_LINK_SONG_GROUP(archive))

/*
 * Package registries live in C.  The metadata build turns each declaration
 * below into a one-byte, ordered ELF symbol.  The final gameplay build already
 * has the generated selectors, so the declarations intentionally emit no ROM
 * data there.
 */
#ifdef BN6_METADATA_ONLY
#define BN6_METADATA_RECORD(kind, payload) \
    __asm__( \
        ".section .bn6_metadata,\"a\",%progbits\n" \
        ".global __bn6_meta__" kind "__" payload "\n" \
        ".type __bn6_meta__" kind "__" payload ",%object\n" \
        "__bn6_meta__" kind "__" payload ":\n" \
        ".byte 0\n" \
        ".size __bn6_meta__" kind "__" payload ",1\n" \
        ".previous\n" \
    )
#else
#define BN6_METADATA_RECORD(kind, payload)
#endif

#define BN6_OBJECT_BODY(object_class, main) \
    BN6_METADATA_RECORD( \
        "object", \
        BN6_STRINGIFY(object_class) "__" BN6_STRINGIFY(main) \
    ); \
    static USED void BN6_JOIN(main, _fn)( \
        Object *self, \
        uint32_t spawn_argument __attribute__((unused)) \
    ); \
    BN6_EXPORT_OBJECT(main, BN6_JOIN(main, _fn)) \
    static USED void BN6_JOIN(main, _fn)( \
        Object *self, \
        uint32_t spawn_argument __attribute__((unused)) \
    )

#define BN6_OBJECT1(main) BN6_OBJECT_BODY(1, main)
#define BN6_OBJECT3(main) BN6_OBJECT_BODY(3, main)
#define BN6_OBJECT4(main) BN6_OBJECT_BODY(4, main)

#define BN6_SONG(archive) \
    extern const uint8_t BN6_LINK_SONG_ID(archive)[]; \
    extern const uint8_t BN6_LINK_SONG_GROUP(archive)[]; \
    BN6_METADATA_RECORD("song", BN6_STRINGIFY(archive))

#define BN6_USE_SONG(archive) \
    extern const uint8_t BN6_LINK_SONG_ID(archive)[]; \
    extern const uint8_t BN6_LINK_SONG_GROUP(archive)[]

#define BN6_ATTACK_BODY(chip_id, main, context_type, context_name) \
    BN6_METADATA_RECORD( \
        "attack", \
        BN6_STRINGIFY(chip_id) "__" BN6_STRINGIFY(main) \
    ); \
    static USED void BN6_JOIN(main, _fn)( \
        uint32_t panel_x, \
        uint32_t panel_y, \
        uint32_t parameter, \
        Object *owner, \
        uint32_t attack, \
        context_type context_name, \
        uint32_t spawn_argument \
    ); \
    BN6_EXPORT_ATTACK(main, BN6_JOIN(main, _fn)) \
    static USED void BN6_JOIN(main, _fn)( \
        uint32_t panel_x, \
        uint32_t panel_y, \
        uint32_t parameter, \
        Object *owner, \
        uint32_t attack, \
        context_type context_name, \
        uint32_t spawn_argument \
    )

#define BN6_ATTACK(chip_id, main) \
    BN6_ATTACK_BODY(chip_id, main, uint32_t, chip_data)

#define BN6_SUMMON_ATTACK(chip_id, main) \
    BN6_ATTACK_BODY(chip_id, main, volatile uint8_t *, completion)

#define BN6_INCLUDE(package) \
    BN6_METADATA_RECORD("include", BN6_STRINGIFY(package))

#define BN6_POINTER_PATCH(address, symbol) \
    BN6_METADATA_RECORD( \
        "pointer", \
        "both__" BN6_STRINGIFY(address) "__" BN6_STRINGIFY(symbol) \
    )

#define BN6_GREGAR_POINTER_PATCH(address, symbol) \
    BN6_METADATA_RECORD( \
        "pointer", \
        "gregar__" BN6_STRINGIFY(address) "__" BN6_STRINGIFY(symbol) \
    )

#define BN6_FALZAR_POINTER_PATCH(address, symbol) \
    BN6_METADATA_RECORD( \
        "pointer", \
        "falzar__" BN6_STRINGIFY(address) "__" BN6_STRINGIFY(symbol) \
    )

#ifdef BN6_METADATA_ONLY
#define BN6_ASM_RESOURCE(name, contents) extern const uint8_t name[]
#define BN6_INCBIN(name, path) extern const uint8_t name[]
#define BN6_RESOURCE_ALIAS(alias, target) extern const uint8_t alias[]
#define BN6_PCM_SONG(archive, prefix, priority, voice, track, sample_path) \
    extern const uint8_t archive[]
#else
/* Keep package-owned binary data next to the C code that uses it. */
#define BN6_ASM_RESOURCE(name, contents) \
    extern const uint8_t name[]; \
    __asm__( \
        ".section .rodata." BN6_STRINGIFY(name) ",\"a\",%progbits\n" \
        ".balign 4\n" \
        ".global " BN6_STRINGIFY(name) "\n" \
        ".type " BN6_STRINGIFY(name) ",%object\n" \
        BN6_STRINGIFY(name) ":\n" \
        contents \
        ".size " BN6_STRINGIFY(name) ",.-" BN6_STRINGIFY(name) "\n" \
        ".previous\n" \
    )

#define BN6_INCBIN(name, path) \
    BN6_ASM_RESOURCE(name, ".incbin \"" path "\"\n")

#define BN6_RESOURCE_ALIAS(alias, target) \
    extern const uint8_t alias[]; \
    __asm__( \
        ".global " BN6_STRINGIFY(alias) "\n" \
        ".type " BN6_STRINGIFY(alias) ",%object\n" \
        ".set " BN6_STRINGIFY(alias) "," BN6_STRINGIFY(target) "\n" \
    )

#define BN6_PCM_SONG(archive, prefix, priority, voice, track, sample_path) \
    BN6_ASM_RESOURCE(archive, \
        ".byte 1,0," BN6_STRINGIFY(priority) ",0\n" \
        ".long " BN6_STRINGIFY(BN6_JOIN(prefix, Voicegroup)) "\n" \
        ".long " BN6_STRINGIFY(BN6_JOIN(prefix, Track)) "\n" \
        ".global " BN6_STRINGIFY(BN6_JOIN(prefix, Voicegroup)) "\n" \
        BN6_STRINGIFY(BN6_JOIN(prefix, Voicegroup)) ":\n" \
        ".byte " BN6_STRINGIFY(voice) ",0x3C,0,0\n" \
        ".long " BN6_STRINGIFY(BN6_JOIN(prefix, Sample)) "\n" \
        ".byte 0xFF,0,0xFF,0\n" \
        ".global " BN6_STRINGIFY(BN6_JOIN(prefix, Track)) "\n" \
        BN6_STRINGIFY(BN6_JOIN(prefix, Track)) ":\n" \
        track \
        ".balign 4\n" \
        ".global " BN6_STRINGIFY(BN6_JOIN(prefix, Sample)) "\n" \
        BN6_STRINGIFY(BN6_JOIN(prefix, Sample)) ":\n" \
        ".incbin \"" sample_path "\"\n" \
    )
#endif

#define BN6_SPRITE(archive, path) \
    extern const uint8_t BN6_LINK_SPRITE_ID(archive)[]; \
    extern const uint8_t BN6_LINK_SPRITE_GROUP(archive)[]; \
    BN6_INCBIN(archive, path); \
    BN6_METADATA_RECORD("sprite", BN6_STRINGIFY(archive))

#endif
