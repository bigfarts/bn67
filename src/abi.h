#ifndef ABI_H
#define ABI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NAKED __attribute__((naked, noinline, used))
#define USED __attribute__((used))

typedef struct Exe6ObjFields Exe6Obj;
typedef struct Exe6PlayerRuntimeFields Exe6PlayerRuntime;
typedef struct Exe6NaviSelectChipWorkFields Exe6NaviSelectChipWork;
typedef struct Exe6NaviStatusWorkFields Exe6NaviStatusWork;
typedef struct Exe6HitFields Exe6Hit;
typedef struct Exe6BattleContextFields Exe6BattleContext;
typedef struct Exe6BattleContextFields Exe6BattleState;
typedef struct Exe6ChipQueueFields Exe6ChipQueue;
typedef struct Exe6ChipClassUseCountsFields Exe6ChipClassUseCounts;
typedef struct Exe6RuntimeFields Exe6Runtime;
typedef struct Exe6ObjectSlotFields Exe6ObjectSlot;
typedef struct Exe6EnemyObjectSlotFields Exe6EnemyObjectSlot;
typedef struct Exe6ShellObjectSlotFields Exe6ShellObjectSlot;
typedef struct Exe6BlockFields Exe6Block;

enum Exe6ObjectClass {
    EXE6_OBJECT_CLASS_ENEMY = 1,
    EXE6_OBJECT_CLASS_SHELL = 3,
    EXE6_OBJECT_CLASS_EFFECT = 4,
};

enum Exe6ObjectState {
    EXE6_OBJECT_STATE_INIT = 0,
    EXE6_OBJECT_STATE_ACTIVE = 4,
    EXE6_OBJECT_STATE_DESTROY = 8,
};

enum Exe6EventChipPhase {
    EXE6_EVENT_CHIP_PHASE_FADE = 0,
    EXE6_EVENT_CHIP_PHASE_TELOP = 4,
    EXE6_EVENT_CHIP_PHASE_EFFECT = 8,
    EXE6_EVENT_CHIP_PHASE_OUTRO = 12,
};

#define EXE6_ENEMY_POOL_HEAD \
    ((Exe6EnemyObjectSlot *)(uintptr_t)0x0203A9B0u)
#define EXE6_SHELL_POOL_HEAD \
    ((Exe6ShellObjectSlot *)(uintptr_t)0x0203CFE0u)
#define EXE6_EFFECT_POOL_HEAD \
    ((Exe6ObjectSlot *)(uintptr_t)0x02036870u)
#define EXE6_POOL_SLOT_COUNT ((size_t)32u)
#define EXE6_BLOCKS \
    ((Exe6Block *)(uintptr_t)0x02039AE0u)
#define EXE6_BLOCK_ROW_WIDTH ((size_t)8u)
#define EXE6_OBSTACLE_SLOT_COUNT ((size_t)6u)

#define EXE6_PALETTE_BG_STAGING_00 ((uintptr_t)0x03001550u)
#define EXE6_PALETTE_BG_STAGING_01 ((uintptr_t)0x03001570u)
#define EXE6_PALETTE_BG_STAGING_02 ((uintptr_t)0x03001590u)
#define EXE6_PALETTE_BG_STAGING_03 ((uintptr_t)0x030015B0u)
#define EXE6_PALETTE_BG_STAGING_04 ((uintptr_t)0x030015D0u)
#define EXE6_PALETTE_BG_STAGING_05 ((uintptr_t)0x030015F0u)
#define EXE6_PALETTE_BG_STAGING_06 ((uintptr_t)0x03001610u)
#define EXE6_PALETTE_BG_STAGING_07 ((uintptr_t)0x03001630u)
#define EXE6_PALETTE_BG_STAGING_08 ((uintptr_t)0x03001650u)
#define EXE6_PALETTE_BG_STAGING_09 ((uintptr_t)0x03001670u)
#define EXE6_PALETTE_BG_STAGING_0A ((uintptr_t)0x03001690u)
#define EXE6_PALETTE_BG_STAGING_0B ((uintptr_t)0x030016B0u)
#define EXE6_PALETTE_BG_STAGING_0C ((uintptr_t)0x030016D0u)
#define EXE6_PALETTE_BG_STAGING_0D ((uintptr_t)0x030016F0u)
#define EXE6_PALETTE_BG_STAGING_0E ((uintptr_t)0x03001710u)
#define EXE6_PALETTE_BG_STAGING_0F ((uintptr_t)0x03001730u)

#define EXE6_PALETTE_BG_OUTPUT_00 ((uintptr_t)0x03001750u)
#define EXE6_PALETTE_BG_OUTPUT_01 ((uintptr_t)0x03001770u)
#define EXE6_PALETTE_BG_OUTPUT_02 ((uintptr_t)0x03001790u)
#define EXE6_PALETTE_BG_OUTPUT_03 ((uintptr_t)0x030017B0u)
#define EXE6_PALETTE_BG_OUTPUT_04 ((uintptr_t)0x030017D0u)
#define EXE6_PALETTE_BG_OUTPUT_05 ((uintptr_t)0x030017F0u)
#define EXE6_PALETTE_BG_OUTPUT_06 ((uintptr_t)0x03001810u)
#define EXE6_PALETTE_BG_OUTPUT_07 ((uintptr_t)0x03001830u)
#define EXE6_PALETTE_BG_OUTPUT_08 ((uintptr_t)0x03001850u)
#define EXE6_PALETTE_BG_OUTPUT_09 ((uintptr_t)0x03001870u)
#define EXE6_PALETTE_BG_OUTPUT_0A ((uintptr_t)0x03001890u)
#define EXE6_PALETTE_BG_OUTPUT_0B ((uintptr_t)0x030018B0u)
#define EXE6_PALETTE_BG_OUTPUT_0C ((uintptr_t)0x030018D0u)
#define EXE6_PALETTE_BG_OUTPUT_0D ((uintptr_t)0x030018F0u)
#define EXE6_PALETTE_BG_OUTPUT_0E ((uintptr_t)0x03001910u)
#define EXE6_PALETTE_BG_OUTPUT_0F ((uintptr_t)0x03001930u)

#define EXE6_PALETTE_OBJ_STAGING_00 ((uintptr_t)0x03001960u)
#define EXE6_PALETTE_OBJ_STAGING_01 ((uintptr_t)0x03001980u)
#define EXE6_PALETTE_OBJ_STAGING_02 ((uintptr_t)0x030019A0u)
#define EXE6_PALETTE_OBJ_STAGING_03 ((uintptr_t)0x030019C0u)
#define EXE6_PALETTE_OBJ_STAGING_04 ((uintptr_t)0x030019E0u)
#define EXE6_PALETTE_OBJ_STAGING_05 ((uintptr_t)0x03001A00u)
#define EXE6_PALETTE_OBJ_STAGING_06 ((uintptr_t)0x03001A20u)
#define EXE6_PALETTE_OBJ_STAGING_07 ((uintptr_t)0x03001A40u)
#define EXE6_PALETTE_OBJ_STAGING_08 ((uintptr_t)0x03001A60u)
#define EXE6_PALETTE_OBJ_STAGING_09 ((uintptr_t)0x03001A80u)
#define EXE6_PALETTE_OBJ_STAGING_0A ((uintptr_t)0x03001AA0u)
#define EXE6_PALETTE_OBJ_STAGING_0B ((uintptr_t)0x03001AC0u)
#define EXE6_PALETTE_OBJ_STAGING_0C ((uintptr_t)0x03001AE0u)
#define EXE6_PALETTE_OBJ_STAGING_0D ((uintptr_t)0x03001B00u)
#define EXE6_PALETTE_OBJ_STAGING_0E ((uintptr_t)0x03001B20u)
#define EXE6_PALETTE_OBJ_STAGING_0F ((uintptr_t)0x03001B40u)

#define EXE6_PALETTE_OBJ_OUTPUT_00 ((uintptr_t)0x03001B60u)
#define EXE6_PALETTE_OBJ_OUTPUT_01 ((uintptr_t)0x03001B80u)
#define EXE6_PALETTE_OBJ_OUTPUT_02 ((uintptr_t)0x03001BA0u)
#define EXE6_PALETTE_OBJ_OUTPUT_03 ((uintptr_t)0x03001BC0u)
#define EXE6_PALETTE_OBJ_OUTPUT_04 ((uintptr_t)0x03001BE0u)
#define EXE6_PALETTE_OBJ_OUTPUT_05 ((uintptr_t)0x03001C00u)
#define EXE6_PALETTE_OBJ_OUTPUT_06 ((uintptr_t)0x03001C20u)
#define EXE6_PALETTE_OBJ_OUTPUT_07 ((uintptr_t)0x03001C40u)
#define EXE6_PALETTE_OBJ_OUTPUT_08 ((uintptr_t)0x03001C60u)
#define EXE6_PALETTE_OBJ_OUTPUT_09 ((uintptr_t)0x03001C80u)
#define EXE6_PALETTE_OBJ_OUTPUT_0A ((uintptr_t)0x03001CA0u)
#define EXE6_PALETTE_OBJ_OUTPUT_0B ((uintptr_t)0x03001CC0u)
#define EXE6_PALETTE_OBJ_OUTPUT_0C ((uintptr_t)0x03001CE0u)
#define EXE6_PALETTE_OBJ_OUTPUT_0D ((uintptr_t)0x03001D00u)
#define EXE6_PALETTE_OBJ_OUTPUT_0E ((uintptr_t)0x03001D20u)
#define EXE6_PALETTE_OBJ_OUTPUT_0F ((uintptr_t)0x03001D40u)
#define EXE6_SPRITE_PALETTE_STAGING_00 ((uintptr_t)0x03001550u)

#define EXE6_BATTLE_STATE \
    ((Exe6BattleState *)(uintptr_t)0x0203CA70u)
#define EXE6_CHIP_QUEUE \
    ((Exe6ChipQueue *)(uintptr_t)0x0203CDB0u)
#define EXE6_USED_CHIP_CLASS_COUNTS \
    ((Exe6ChipClassUseCounts *)(uintptr_t)0x020367E0u)

#define EXE6_JOIN_INNER(left, right) left##right
#define EXE6_JOIN(left, right) EXE6_JOIN_INNER(left, right)
#define EXE6_STRINGIFY_INNER(value) #value
#define EXE6_STRINGIFY(value) EXE6_STRINGIFY_INNER(value)

/* ObjHeader.Flags (+0x00). */
#define EXE6_OBJ_FLAG_ACTIVE 0x01
#define EXE6_OBJ_FLAG_VISIBLE 0x02
#define EXE6_OBJ_FLAG_UPDATE_DURING_PAUSE 0x04
#define EXE6_OBJ_FLAG_STOP_SPRITE_UPDATE 0x08
#define EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING 0x10

/* Native battle sprites use OAM priority 2, behind the Custom gauge and HUD. */
#define EXE6_OBJ_PRIORITY_BATTLE 2

/* Flags returned for the current sprite-animation frame. */
#define EXE6_ANIMATION_FRAME_FLAG_END 0x80

/* BlockData.Flags (+0x14). */
#define EXE6_BLOCK_FLAG_SOLID 0x00000010
#define EXE6_BLOCK_FLAG_CRACKED 0x00000040
#define EXE6_BLOCK_FLAG_VALID 0x00010000
/* Collision records contribute these support-object occupancy flags. */
#define EXE6_BLOCK_FLAG_NEUTRAL_SUPPORT_OBJECT 0x00800000
#define EXE6_BLOCK_FLAG_SIDE_1_SUPPORT_OBJECT 0x01000000
#define EXE6_BLOCK_FLAG_SIDE_0_SUPPORT_OBJECT 0x02000000
#define EXE6_BLOCK_FLAG_SUPPORT_OBJECT \
    (EXE6_BLOCK_FLAG_NEUTRAL_SUPPORT_OBJECT | \
     EXE6_BLOCK_FLAG_SIDE_1_SUPPORT_OBJECT | \
     EXE6_BLOCK_FLAG_SIDE_0_SUPPORT_OBJECT)
/* Live Navi hit records contribute these flags to their block. */
#define EXE6_BLOCK_FLAG_SIDE_1_NAVI 0x00200000
#define EXE6_BLOCK_FLAG_SIDE_0_NAVI 0x00400000
/* Generic live hit records contribute these alliance flags. */
#define EXE6_BLOCK_FLAG_SIDE_1_HIT 0x04000000
#define EXE6_BLOCK_FLAG_SIDE_0_HIT 0x08000000

/* HitData.ObjFlags1 (+0x3C). */
#define EXE6_HIT_STATUS_FLAG_AIR_SHOES 0x00000010
#define EXE6_HIT_STATUS_FLAG_FLOAT_SHOES 0x00000020
#define EXE6_HIT_STATUS_FLAG_SUPER_ARMOR 0x00020000
#define EXE6_HIT_STATUS_FLAG_UNDERSHIRT 0x00040000

/* HitData.ObjFlags2 (+0x40). */
#define EXE6_HIT_SECONDARY_FLAG_TIMED_BLINK_REMOVAL 0x00040000
#define EXE6_HIT_SECONDARY_FLAG_DUST_SUCTION_SIDE_0 0x00100000
#define EXE6_HIT_SECONDARY_FLAG_DUST_SUCTION_SIDE_1 0x00200000

/* HitData.HitModifierBase/Final (+0x0E/+0x0F). */
#define EXE6_HIT_MODIFIER_STAGGER 0x01
#define EXE6_HIT_MODIFIER_FIXED_INVULNERABILITY 0x02

/*
 * HitData.Region (+0x01) values used by generic block damage.
 * CENTERED_3X3 is the nine-entry offset list centered on the origin.
 * ALL_VALID_BLOCKS selects the engine's whole-board parameter scan with an
 * empty required/excluded filter, so nonexistent blocks are still rejected.
 */
typedef enum Exe6HitRegion {
    EXE6_HIT_REGION_CURRENT_BLOCK = 0x01,
    EXE6_HIT_REGION_CENTERED_3X3 = 0x0F,
    EXE6_HIT_REGION_ALL_VALID_BLOCKS = 0x80,
} __attribute__((packed)) Exe6HitRegion;

/*
 * HitData.HitEffect (+0x09). These values select only the visual
 * spawned on contact; damage, elements, statuses, and trap deletion are
 * controlled by other hit fields.
 */
typedef enum Exe6HitEffect {
    EXE6_HIT_EFFECT_NORMAL = 0x00,
    EXE6_HIT_EFFECT_FIRE = 0x01,
    EXE6_HIT_EFFECT_AQUA = 0x02,
    EXE6_HIT_EFFECT_ELEC = 0x03,
    EXE6_HIT_EFFECT_WOOD = 0x04,
    EXE6_HIT_EFFECT_CHARGE_SHOT = 0x05,
    EXE6_HIT_EFFECT_SMALL_IMPACT = 0x06,
    EXE6_HIT_EFFECT_EXPLOSION = 0x07,
    EXE6_HIT_EFFECT_PING = 0x08,
    EXE6_HIT_EFFECT_CHIP_DELETE = 0x09,
    EXE6_HIT_EFFECT_BREAK = 0x0A,
    EXE6_HIT_EFFECT_LARGE_EXPLOSION = 0x0B,
    EXE6_HIT_EFFECT_CHARGE_SHOT_PRIORITY_2 = 0x0C,
    EXE6_HIT_EFFECT_BAT = 0x0D,
    EXE6_HIT_EFFECT_UNINSTALL = 0x0E,
    EXE6_HIT_EFFECT_UNINSTALL_ALT = 0x0F,
    EXE6_HIT_EFFECT_NONE = 0xFF,
} __attribute__((packed)) Exe6HitEffect;

/*
 * Bits used by the 32-bit entries in BN6's hit-type table. Several
 * upper bits are matching categories rather than independently consumed
 * effects; their engine semantics are not yet identified.
 */
typedef enum Exe6HitTypeFlag {
    EXE6_HIT_TYPE_FLAG_GUARD_PIERCING = 0x00000002u,
    EXE6_HIT_TYPE_FLAG_INVIS_PIERCING = 0x00000004u,
    EXE6_HIT_TYPE_FLAG_HITS_OBJ_FLAG_04 = 0x00000008u,
    EXE6_HIT_TYPE_FLAG_DELETE_ACTIVE_CHIP = 0x00000010u,
    EXE6_HIT_TYPE_FLAG_00000020 = 0x00000020u,
    EXE6_HIT_TYPE_FLAG_HITS_FLOATING = 0x00000080u,
    EXE6_HIT_TYPE_FLAG_00000100 = 0x00000100u,
    EXE6_HIT_TYPE_FLAG_00000200 = 0x00000200u,
    EXE6_HIT_TYPE_FLAG_00000400 = 0x00000400u,
    EXE6_HIT_TYPE_FLAG_00000800 = 0x00000800u,
    EXE6_HIT_TYPE_FLAG_00001000 = 0x00001000u,
    EXE6_HIT_TYPE_FLAG_00002000 = 0x00002000u,
    EXE6_HIT_TYPE_FLAG_00004000 = 0x00004000u,
    EXE6_HIT_TYPE_FLAG_00008000 = 0x00008000u,
    EXE6_HIT_TYPE_FLAG_00010000 = 0x00010000u,
    /* Requests the native obstacle guard ping and its SFX. */
    EXE6_HIT_TYPE_FLAG_GUARD_BLOCKED = 0x00020000u,
    EXE6_HIT_TYPE_FLAG_00040000 = 0x00040000u,
    EXE6_HIT_TYPE_FLAG_00080000 = 0x00080000u,
    EXE6_HIT_TYPE_FLAG_00100000 = 0x00100000u,
    EXE6_HIT_TYPE_FLAG_00200000 = 0x00200000u,
    EXE6_HIT_TYPE_FLAG_00400000 = 0x00400000u,
    EXE6_HIT_TYPE_FLAG_00800000 = 0x00800000u,
    EXE6_HIT_TYPE_FLAG_01000000 = 0x01000000u,
    EXE6_HIT_TYPE_FLAG_02000000 = 0x02000000u,
    EXE6_HIT_TYPE_FLAG_04000000 = 0x04000000u,
    EXE6_HIT_TYPE_FLAG_08000000 = 0x08000000u,
    EXE6_HIT_TYPE_FLAG_10000000 = 0x10000000u,
    EXE6_HIT_TYPE_FLAG_20000000 = 0x20000000u,
    EXE6_HIT_TYPE_FLAG_40000000 = 0x40000000u,
    EXE6_HIT_TYPE_FLAG_80000000 = 0x80000000u,
} Exe6HitTypeFlag;

/*
 * Byte selectors into BN6's shared hit-type table. The native table has
 * 0x59 entries; 0x58 is its final selector.
 */
typedef enum Exe6HitType {
    EXE6_HIT_TYPE_NONE = 0x00,
    EXE6_HIT_TYPE_NAVI_BODY = 0x01,
    EXE6_HIT_TYPE_CHARACTER_TARGET = 0x02,
    EXE6_HIT_TYPE_03 = 0x03,
    EXE6_HIT_TYPE_STANDARD_ATTACK = 0x04,
    EXE6_HIT_TYPE_STANDARD_TARGET = 0x05,
    EXE6_HIT_TYPE_BREAK_ATTACK = 0x06,
    EXE6_HIT_TYPE_SWORD_ATTACK = 0x07,
    EXE6_HIT_TYPE_DELETE_ACTIVE_CHIP_ATTACK = 0x08,
    EXE6_HIT_TYPE_09 = 0x09,
    EXE6_HIT_TYPE_OBJECT_HITTING_ATTACK = 0x0A,
    EXE6_HIT_TYPE_INVIS_PIERCING_ATTACK = 0x0B,
    EXE6_HIT_TYPE_0C = 0x0C,
    EXE6_HIT_TYPE_0D = 0x0D,
    EXE6_HIT_TYPE_BATTLE_OBJECT_SELF = 0x0E,
    EXE6_HIT_TYPE_BATTLE_OBJECT_TARGET = 0x0F,
    EXE6_HIT_TYPE_ENEMY_BODY = 0x10,
    EXE6_HIT_TYPE_11 = 0x11,
    EXE6_HIT_TYPE_12 = 0x12,
    EXE6_HIT_TYPE_13 = 0x13,
    EXE6_HIT_TYPE_14 = 0x14,
    EXE6_HIT_TYPE_15 = 0x15,
    EXE6_HIT_TYPE_16 = 0x16,
    EXE6_HIT_TYPE_17 = 0x17,
    EXE6_HIT_TYPE_18 = 0x18,
    EXE6_HIT_TYPE_INVIS_PIERCING_OBJECT_HITTING_ATTACK = 0x19,
    EXE6_HIT_TYPE_1A = 0x1A,
    EXE6_HIT_TYPE_1B = 0x1B,
    EXE6_HIT_TYPE_1C = 0x1C,
    EXE6_HIT_TYPE_INVIS_PIERCING_OBJECT_HITTING_DELETE_ACTIVE_CHIP_ATTACK =
        0x1D,
    EXE6_HIT_TYPE_1E = 0x1E,
    EXE6_HIT_TYPE_1F = 0x1F,
    EXE6_HIT_TYPE_20 = 0x20,
    EXE6_HIT_TYPE_21 = 0x21,
    EXE6_HIT_TYPE_22 = 0x22,
    EXE6_HIT_TYPE_23 = 0x23,
    EXE6_HIT_TYPE_24 = 0x24,
    EXE6_HIT_TYPE_25 = 0x25,
    EXE6_HIT_TYPE_26 = 0x26,
    EXE6_HIT_TYPE_27 = 0x27,
    EXE6_HIT_TYPE_28 = 0x28,
    EXE6_HIT_TYPE_29 = 0x29,
    EXE6_HIT_TYPE_2A = 0x2A,
    EXE6_HIT_TYPE_2B = 0x2B,
    EXE6_HIT_TYPE_2C = 0x2C,
    EXE6_HIT_TYPE_2D = 0x2D,
    EXE6_HIT_TYPE_2E = 0x2E,
    EXE6_HIT_TYPE_2F = 0x2F,
    EXE6_HIT_TYPE_30 = 0x30,
    EXE6_HIT_TYPE_31 = 0x31,
    EXE6_HIT_TYPE_32 = 0x32,
    EXE6_HIT_TYPE_33 = 0x33,
    EXE6_HIT_TYPE_34 = 0x34,
    EXE6_HIT_TYPE_35 = 0x35,
    EXE6_HIT_TYPE_36 = 0x36,
    EXE6_HIT_TYPE_37 = 0x37,
    EXE6_HIT_TYPE_38 = 0x38,
    EXE6_HIT_TYPE_39 = 0x39,
    EXE6_HIT_TYPE_3A = 0x3A,
    EXE6_HIT_TYPE_3B = 0x3B,
    EXE6_HIT_TYPE_3C = 0x3C,
    EXE6_HIT_TYPE_3D = 0x3D,
    EXE6_HIT_TYPE_3E = 0x3E,
    EXE6_HIT_TYPE_3F = 0x3F,
    EXE6_HIT_TYPE_40 = 0x40,
    EXE6_HIT_TYPE_41 = 0x41,
    EXE6_HIT_TYPE_42 = 0x42,
    EXE6_HIT_TYPE_43 = 0x43,
    EXE6_HIT_TYPE_44 = 0x44,
    EXE6_HIT_TYPE_45 = 0x45,
    EXE6_HIT_TYPE_46 = 0x46,
    EXE6_HIT_TYPE_47 = 0x47,
    EXE6_HIT_TYPE_48 = 0x48,
    EXE6_HIT_TYPE_49 = 0x49,
    EXE6_HIT_TYPE_4A = 0x4A,
    EXE6_HIT_TYPE_4B = 0x4B,
    EXE6_HIT_TYPE_4C = 0x4C,
    EXE6_HIT_TYPE_4D = 0x4D,
    EXE6_HIT_TYPE_4E = 0x4E,
    EXE6_HIT_TYPE_4F = 0x4F,
    EXE6_HIT_TYPE_50 = 0x50,
    EXE6_HIT_TYPE_51 = 0x51,
    EXE6_HIT_TYPE_52 = 0x52,
    EXE6_HIT_TYPE_53 = 0x53,
    EXE6_HIT_TYPE_54 = 0x54,
    EXE6_HIT_TYPE_55 = 0x55,
    EXE6_HIT_TYPE_56 = 0x56,
    EXE6_HIT_TYPE_57 = 0x57,
    EXE6_HIT_TYPE_58 = 0x58,
} __attribute__((packed)) Exe6HitType;

/* Byte layout of the native block-damage r4 parameter. */
typedef struct __attribute__((aligned(4))) Exe6BlockDamageProperties {
    Exe6HitRegion region;
    Exe6HitEffect hit_effect;
    Exe6HitType target_hit_type;
    Exe6HitType self_hit_type;
} Exe6BlockDamageProperties;

/* BattleState control flags at +0x5C. */
#define EXE6_BATTLE_CONTROL_FLAG_SIDE_0_CHIPS_ENABLED 0x04
#define EXE6_BATTLE_CONTROL_FLAG_SIDE_1_CHIPS_ENABLED 0x08

/* Battle configuration flags. */
#define EXE6_BATTLE_CONFIG_FLAG_LINK 0x08

/* Joypad state bits. */
#define EXE6_KEY_RIGHT 0x10
#define EXE6_KEY_LEFT 0x20
#define EXE6_KEY_UP 0x40
#define EXE6_KEY_DOWN 0x80

/*
 * Convert BN6's native register conventions to the C ABI.  These veneers
 * must use BL so the C helper receives an odd Thumb return address.
 */
#define EXE6_EXPORT_OBJ(name, target) \
    NAKED void name(void) \
    { \
        __asm__( \
            ".syntax unified\n" \
            "push {lr}\n" \
            "adds r0,r5,#0\n" \
            "adds r1,r4,#0\n" \
            "bl " EXE6_STRINGIFY(target) "\n" \
            "pop {pc}\n" \
        ); \
    }

#define EXE6_EXPORT_ATTACK(name, target) \
    NAKED void name(void) \
    { \
        __asm__( \
            ".syntax unified\n" \
            "push {r4,lr}\n" \
            "push {r6,r7}\n" \
            "adds r3,r5,#0\n" \
            "bl " EXE6_STRINGIFY(target) "\n" \
            "add sp,#8\n" \
            "pop {r4,pc}\n" \
        ); \
    }

/*
 * Family 0x1C enters with z in r3, obj spawn parameters in r4, owner in r5,
 * and resolved attack in r6.  Preserve z before putting owner in the fourth C
 * argument, then arrange attack/z/spawn_parameters as stacked arguments 5-7.
 * Saving r7 alongside r4/lr keeps the C call's stack 8-byte aligned.
 */
#define EXE6_EXPORT_EPHEMERAL_ATTACK(name, target) \
    NAKED void name(void) \
    { \
        __asm__( \
            ".syntax unified\n" \
            "push {r4,r7,lr}\n" \
            "push {r4}\n" \
            "push {r3}\n" \
            "push {r6}\n" \
            "adds r3,r5,#0\n" \
            "bl " EXE6_STRINGIFY(target) "\n" \
            "add sp,#12\n" \
            "pop {r4,r7,pc}\n" \
        ); \
    }

/*
 * The native obj spawners copy this value, not a pointer, into obj bytes
 * +0x04 through +0x07.  Attack behavior.object_spawn uses the same layout.
 */
typedef struct __attribute__((aligned(4))) Exe6ObjSpawnParameters {
    uint8_t variant;
    uint8_t subvariant;
    uint8_t animation_state;
    uint8_t removal_state;
} Exe6ObjSpawnParameters;

static inline Exe6ObjSpawnParameters exe6_obj_spawn_empty(void)
{
    return (Exe6ObjSpawnParameters){0};
}

static inline Exe6ObjSpawnParameters exe6_obj_spawn_with_variant(
    uint8_t variant
)
{
    return (Exe6ObjSpawnParameters){ .variant = variant };
}

enum Exe6ChipCode {
    EXE6_CHIP_CODE_A = 0x00,
    EXE6_CHIP_CODE_B = 0x01,
    EXE6_CHIP_CODE_C = 0x02,
    EXE6_CHIP_CODE_D = 0x03,
    EXE6_CHIP_CODE_E = 0x04,
    EXE6_CHIP_CODE_F = 0x05,
    EXE6_CHIP_CODE_G = 0x06,
    EXE6_CHIP_CODE_H = 0x07,
    EXE6_CHIP_CODE_I = 0x08,
    EXE6_CHIP_CODE_J = 0x09,
    EXE6_CHIP_CODE_K = 0x0A,
    EXE6_CHIP_CODE_L = 0x0B,
    EXE6_CHIP_CODE_M = 0x0C,
    EXE6_CHIP_CODE_N = 0x0D,
    EXE6_CHIP_CODE_O = 0x0E,
    EXE6_CHIP_CODE_P = 0x0F,
    EXE6_CHIP_CODE_Q = 0x10,
    EXE6_CHIP_CODE_R = 0x11,
    EXE6_CHIP_CODE_S = 0x12,
    EXE6_CHIP_CODE_T = 0x13,
    EXE6_CHIP_CODE_U = 0x14,
    EXE6_CHIP_CODE_V = 0x15,
    EXE6_CHIP_CODE_W = 0x16,
    EXE6_CHIP_CODE_X = 0x17,
    EXE6_CHIP_CODE_Y = 0x18,
    EXE6_CHIP_CODE_Z = 0x19,
    EXE6_CHIP_CODE_ASTERISK = 0x1A,
    EXE6_CHIP_CODE_NONE = 0xFF,
};

enum Exe6ChipElement {
    EXE6_CHIP_ELEMENT_FIRE = 0x00,
    EXE6_CHIP_ELEMENT_AQUA = 0x01,
    EXE6_CHIP_ELEMENT_ELEC = 0x02,
    EXE6_CHIP_ELEMENT_WOOD = 0x03,
    EXE6_CHIP_ELEMENT_BONUS = 0x04,
    EXE6_CHIP_ELEMENT_SWORD = 0x05,
    EXE6_CHIP_ELEMENT_CURSOR = 0x06,
    EXE6_CHIP_ELEMENT_OBSTACLE = 0x07,
    EXE6_CHIP_ELEMENT_WIND = 0x08,
    EXE6_CHIP_ELEMENT_BREAK = 0x09,
    EXE6_CHIP_ELEMENT_NULL = 0x0A,
};

enum Exe6ChipClass {
    EXE6_CHIP_CLASS_STANDARD = 0x00,
    EXE6_CHIP_CLASS_MEGA = 0x01,
    EXE6_CHIP_CLASS_GIGA = 0x02,
    EXE6_CHIP_CLASS_PROGRAM_ADVANCE = 0x04,
};

enum Exe6ChipEffectFlag {
    // Makes the chip dim/time-freeze and allows it to be used as a cut-in.
    EXE6_CHIP_EFFECT_FLAG_DIMMING = 0x01,
    // Marks an offensive chip whose power receives the standard attack bonus.
    EXE6_CHIP_EFFECT_FLAG_ATTACK = 0x02,
    // Marks a Navi summon and makes its power receive the Navi-chip bonus.
    EXE6_CHIP_EFFECT_FLAG_NAVI = 0x04,
    EXE6_CHIP_EFFECT_FLAG_CHIP_TRADER = 0x08,
    EXE6_CHIP_EFFECT_FLAG_VARIABLE_POWER_DISPLAY = 0x10,
    // Legacy DarkChip category; no stock BN6 chip record sets this bit.
    EXE6_CHIP_EFFECT_FLAG_DARK_CHIP = 0x20,
    // Makes the chip available in this version and visible in the library.
    EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE = 0x40,
    // Calculates and caches chip-specific power when preparing the chip.
    EXE6_CHIP_EFFECT_FLAG_DYNAMIC_POWER = 0x80,
};

typedef struct __attribute__((packed)) Exe6ChipSpawnParametersFields {
    uint8_t variant;
    uint8_t subvariant;
    uint8_t animation_state;
    uint8_t removal_state;
} Exe6ChipSpawnParameters;

typedef struct __attribute__((packed)) Exe6ChipBehaviorFields {
    uint8_t effect_flags;                // +0x09, EXE6_CHIP_EFFECT_FLAG_*
    uint8_t counter_settings;            // +0x0A
    uint8_t family;                      // +0x0B
    uint8_t subfamily;                   // +0x0C
    uint8_t dark_soul_usage;             // +0x0D
    uint8_t unknown_0e;                  // +0x0E
    uint8_t lock_on;                     // +0x0F
    Exe6ChipSpawnParameters object_spawn; // +0x10
    uint8_t delay;                       // +0x14
} Exe6ChipBehavior;

typedef struct __attribute__((packed, aligned(4))) Exe6ChipRecordFields {
    uint8_t codes[4];                    // +0x00
    uint8_t attack_element;              // +0x04
    uint8_t rarity;                      // +0x05
    uint8_t element;                     // +0x06
    uint8_t chip_class;                  // +0x07
    uint8_t mb;                          // +0x08
    Exe6ChipBehavior behavior;           // +0x09
    uint8_t library_number;              // +0x15
    uint8_t library_flags;               // +0x16
    uint8_t library_lock_on_type;        // +0x17
    uint16_t alphabetical_sort;          // +0x18, regenerated after ROM linking
    uint16_t power;                      // +0x1A
    uint16_t library_sort_order;         // +0x1C
    uint8_t library_gate_usage;          // +0x1E
    uint8_t dark_chip_id;                // +0x1F
    const uint8_t *icon;                 // +0x20
    const uint8_t *image;                // +0x24
    const uint8_t *palette;              // +0x28
} Exe6ChipRecord;

struct Exe6ObjFields {
    uint8_t header_flags;                // +0x00, EXE6_OBJ_FLAG_*
    uint8_t object_id;                   // +0x01
    uint8_t object_class;                // +0x02, low nibble
    uint8_t unknown_03;
    union {
        Exe6ObjSpawnParameters spawn_parameters; // +0x04
        struct {
            union {
                uint16_t variant_word;    // +0x04
                struct {
                    uint8_t variant;      // +0x04
                    uint8_t subvariant;   // +0x05
                };
            };
            union {
                uint16_t animation_state_word; // +0x06
                struct {
                    uint8_t animation_state;   // +0x06
                    uint8_t removal_state;     // +0x07
                };
            };
        };
    };
    union {
        uint32_t state_word;             // +0x08
        struct {
            uint8_t state;               // +0x08
            uint8_t phase;               // +0x09
            union {
                uint16_t phase_timer;     // +0x0A
                struct {
                    uint8_t phase_timer_low;
                    uint8_t substate;     // +0x0B
                };
            };
        };
    };
    uint8_t unknown_0c[2];
    uint8_t parameter;                   // +0x0E
    uint8_t unknown_0f;
    union {
        uint16_t animation_word;         // +0x10
        struct {
            uint8_t animation;           // +0x10
            uint8_t palette;             // +0x11
        };
    };
    uint8_t block_x;                     // +0x12
    uint8_t block_y;                     // +0x13
    uint8_t target_block_x;              // +0x14
    uint8_t target_block_y;              // +0x15
    union {
        uint16_t owner_word;             // +0x16
        struct {
            uint8_t owner;               // +0x16
            uint8_t owner_aux;            // +0x17
        };
    };
    uint8_t unknown_18[2];
    uint8_t loaded_chip_count;           // +0x1A
    uint8_t unknown_1b[5];
    uint16_t timer;                      // +0x20
    uint16_t aux_timer;                  // +0x22
    uint16_t hp;                         // +0x24
    uint16_t max_hp;                     // +0x26
    uint8_t unknown_28[2];
    uint16_t active_chip_id;             // +0x2A
    uint32_t attack;                     // +0x2C
    union {
        uint32_t chip_data;              // +0x30
        uint8_t *completion;             // +0x30
        struct {
            uint16_t chip_id;
            uint16_t attack_bonus;       // +0x32
        };
    };
    int32_t x;                          // +0x34
    int32_t y;                          // +0x38
    int32_t z;                          // +0x3C
    int32_t velocity_x;                 // +0x40
    int32_t velocity_y;                 // +0x44
    int32_t velocity_z;                 // +0x48
    Exe6Obj *parent;                         // +0x4C
    uint8_t unknown_50[4];
    Exe6Hit *hit;                           // +0x54
    Exe6PlayerRuntime *runtime_data;     // +0x58
    uint32_t engine_reserved;            // +0x5C
    uint8_t work[0x20];                  // +0x60 through +0x7C
};

struct Exe6ObjectSlotFields {
    Exe6Obj object;
    uint8_t reserved[0xC8 - sizeof(Exe6Obj)];
};

struct Exe6EnemyObjectSlotFields {
    Exe6Obj object;
    uint8_t reserved[0xD8 - sizeof(Exe6Obj)];
};

struct Exe6ShellObjectSlotFields {
    Exe6Obj object;
    uint8_t reserved[0xD8 - sizeof(Exe6Obj)];
};

struct Exe6BlockFields {
    uint8_t active;                      // +0x00
    uint8_t terrain_animation_state;     // +0x01
    uint8_t terrain_state;               // +0x02
    uint8_t owner;                       // +0x03
    uint8_t original_owner;              // +0x04
    uint8_t unknown_05;
    uint8_t rendered_terrain_state;      // +0x06
    uint8_t rendered_owner;              // +0x07
    uint8_t unknown_08[2];
    uint8_t block_x;                     // +0x0A
    uint8_t block_y;                     // +0x0B
    uint8_t edge_flags;                  // +0x0C
    uint8_t transition_pending;          // +0x0D
    uint16_t unknown_0e;
    uint16_t animation_timer;            // +0x10
    uint16_t unknown_12;
    uint32_t status_flags;               // +0x14
    uint8_t unknown_18[8];
};

struct Exe6PlayerRuntimeFields {
    uint8_t type;                        // +0x00
    uint8_t unknown_01[6];
    uint8_t active_power_attack;         // +0x07
    uint8_t b_left;                      // +0x08
    uint8_t unknown_09[0x23];
    uint16_t input;                      // +0x2C
};

struct Exe6NaviSelectChipWorkFields {
    uint8_t active_chip_index;           // +0x00
    uint8_t loaded_chip_count;           // +0x01
    uint16_t chip_ids[6];                // +0x02
    uint8_t unknown_0e[0x42];
};

struct Exe6NaviStatusWorkFields {
    uint8_t unknown_00[0x12];
    uint8_t block_bug_action;             // +0x12
    uint8_t block_bug_severity;           // +0x13
    uint8_t buster_bug_severity;          // +0x14
    uint8_t buster_bug_level;             // +0x15
    uint8_t damage_hp_bug;                // +0x16
    uint8_t unknown_17;
    uint8_t battle_hp_bug;                // +0x18
    uint8_t custom_hp_bug;                // +0x19
    uint8_t color_bug;                    // +0x1A
    uint8_t unknown_1b[0x09];
    uint8_t emotion_bug;                  // +0x24
    uint8_t unknown_25[0x0C];
    uint8_t movement_bug;                 // +0x31
    uint8_t unknown_32[0x22];
    uint16_t custom_damage_bug;           // +0x54
    uint8_t unknown_56[0x0D];
    uint8_t custom_bug;                   // +0x63
};

struct Exe6HitFields {
    uint8_t enabled;                     // +0x00
    uint8_t region;                      // +0x01
    uint8_t unknown_02[0x07];
    uint8_t hit_effect;                  // +0x09, Exe6HitEffect
    uint8_t unknown_0a[0x04];
    uint8_t base_hit_modifier;           // +0x0E, EXE6_HIT_MODIFIER_*
    uint8_t final_hit_modifier;          // +0x0F, EXE6_HIT_MODIFIER_*
    uint8_t unknown_10[0x14];
    uint16_t fixed_invulnerability_timer; // +0x24
    uint8_t unknown_26[0x4A];
    Exe6HitTypeFlag received_hit_flags;         // +0x70, EXE6_RECEIVED_HIT_FLAG_*
    uint8_t unknown_74[0x0C];
    uint16_t final_damage;               // +0x80
    uint16_t damage_buckets[5];          // +0x82..+0x8B
};

struct Exe6BattleContextFields {
    uint32_t state;                      // +0x00
    uint8_t unknown_04;
    uint8_t custom_screen_side;          // +0x05
    uint8_t unknown_06[0x07];
    uint8_t local_side;                  // +0x0D
    uint8_t unknown_0e[0x09];
    uint8_t regular_chip_available;           // +0x17
    uint8_t unknown_18[0x2C];
    uint8_t tag_chips_available;                     // +0x44
    uint8_t unknown_45[0x3B];
    Exe6Obj *battle_units[2][4];             // +0x80
    Exe6Obj *obstacles[EXE6_OBSTACLE_SLOT_COUNT]; // +0xA0
    uint8_t unknown_b8[0x18];
    Exe6Obj *active_units[2][4];             // +0xD0
};

struct Exe6ChipQueueFields {
    uint16_t chips[30];                  // +0x00 through +0x3A
};

struct Exe6ChipClassUseCountsFields {
    uint8_t standard;                    // +0x00
    uint8_t mega;                        // +0x01
    uint8_t giga;                        // +0x02
    uint8_t reserved;                    // +0x03
};

struct Exe6RuntimeFields {
    uint8_t unknown_00[0x18];
    Exe6BattleContext *battle_context;       // +0x18
};

_Static_assert(
    sizeof(Exe6ObjSpawnParameters) == sizeof(uint32_t),
    "obj spawn parameters must occupy one native register"
);
_Static_assert(
    offsetof(Exe6ObjSpawnParameters, variant) == 0
        && offsetof(Exe6ObjSpawnParameters, subvariant) == 1
        && offsetof(Exe6ObjSpawnParameters, animation_state) == 2
        && offsetof(Exe6ObjSpawnParameters, removal_state) == 3,
    "obj spawn parameter byte layout"
);
_Static_assert(sizeof(Exe6ChipBehavior) == 0x0C, "chip behavior size");
_Static_assert(sizeof(Exe6ChipRecord) == 0x2C, "chip record size");
_Static_assert(
    sizeof(Exe6ChipClassUseCounts) == sizeof(uint32_t),
    "chip class use-count size"
);
_Static_assert(offsetof(Exe6ChipRecord, behavior) == 0x09, "chip behavior offset");
_Static_assert(
    offsetof(Exe6ChipRecord, library_number) == 0x15,
    "chip library offset"
);
_Static_assert(offsetof(Exe6ChipRecord, power) == 0x1A, "chip power offset");
_Static_assert(offsetof(Exe6ChipRecord, icon) == 0x20, "chip artwork offset");
_Static_assert(
    offsetof(struct Exe6ObjFields, spawn_parameters) == 0x04,
    "obj spawn parameters offset"
);
_Static_assert(offsetof(struct Exe6ObjFields, object_id) == 0x01, "obj ID offset");
_Static_assert(
    offsetof(struct Exe6ObjFields, object_class) == 0x02,
    "obj class offset"
);
_Static_assert(sizeof(Exe6ObjectSlot) == 0xC8, "native object slot layout");
_Static_assert(sizeof(Exe6EnemyObjectSlot) == 0xD8, "native enemy slot layout");
_Static_assert(sizeof(Exe6ShellObjectSlot) == 0xD8, "native shell slot layout");
_Static_assert(
    sizeof(Exe6Block) == 0x20,
    "native block layout"
);
_Static_assert(
    offsetof(Exe6Block, active) == 0,
    "block active offset"
);
_Static_assert(
    offsetof(Exe6Block, rendered_terrain_state) == 0x06,
    "block rendered terrain offset"
);
_Static_assert(
    offsetof(Exe6Block, rendered_owner) == 0x07,
    "block rendered owner offset"
);
_Static_assert(
    offsetof(Exe6Block, edge_flags) == 0x0C,
    "block edge flags offset"
);
_Static_assert(offsetof(struct Exe6ObjFields, state_word) == 0x08, "obj state offset");
_Static_assert(offsetof(struct Exe6ObjFields, variant) == 0x04, "obj variant offset");
_Static_assert(offsetof(struct Exe6ObjFields, parameter) == 0x0E, "obj parameter offset");
_Static_assert(offsetof(struct Exe6ObjFields, block_x) == 0x12, "obj block offset");
_Static_assert(offsetof(struct Exe6ObjFields, owner_word) == 0x16, "obj owner offset");
_Static_assert(offsetof(struct Exe6ObjFields, timer) == 0x20, "obj timer offset");
_Static_assert(
    offsetof(struct Exe6ObjFields, active_chip_id) == 0x2A,
    "obj active chip id offset"
);
_Static_assert(offsetof(struct Exe6ObjFields, attack) == 0x2C, "obj attack offset");
_Static_assert(offsetof(struct Exe6ObjFields, chip_data) == 0x30, "obj chip data offset");
_Static_assert(offsetof(struct Exe6ObjFields, attack_bonus) == 0x32, "obj attack bonus offset");
_Static_assert(offsetof(struct Exe6ObjFields, velocity_x) == 0x40, "obj velocity offset");
_Static_assert(offsetof(struct Exe6ObjFields, velocity_z) == 0x48, "obj velocity z offset");
_Static_assert(offsetof(struct Exe6ObjFields, parent) == 0x4C, "obj parent offset");
_Static_assert(offsetof(struct Exe6ObjFields, hit) == 0x54, "obj hit offset");
_Static_assert(offsetof(struct Exe6ObjFields, runtime_data) == 0x58, "obj runtime data offset");
_Static_assert(
    offsetof(struct Exe6ObjFields, engine_reserved) == 0x5C,
    "obj engine-reserved offset"
);
_Static_assert(offsetof(struct Exe6ObjFields, work) == 0x60, "obj work offset");
_Static_assert(offsetof(struct Exe6ObjFields, completion) == 0x30, "obj completion offset");
_Static_assert(
    offsetof(struct Exe6PlayerRuntimeFields, type) == 0,
    "player runtime type offset"
);
_Static_assert(
    offsetof(struct Exe6PlayerRuntimeFields, active_power_attack) == 0x07,
    "player runtime active power attack offset"
);
_Static_assert(
    offsetof(struct Exe6PlayerRuntimeFields, b_left) == 0x08,
    "player runtime B-Left offset"
);
_Static_assert(
    offsetof(struct Exe6PlayerRuntimeFields, input) == 0x2C,
    "player runtime input offset"
);
_Static_assert(
    offsetof(struct Exe6NaviSelectChipWorkFields, active_chip_index) == 0,
    "selected chip active index offset"
);
_Static_assert(
    offsetof(struct Exe6NaviSelectChipWorkFields, chip_ids) == 2,
    "selected chip id offset"
);
_Static_assert(
    sizeof(struct Exe6NaviSelectChipWorkFields) == 0x50,
    "selected chip work size"
);
_Static_assert(
    offsetof(struct Exe6NaviStatusWorkFields, block_bug_action) == 0x12,
    "Navi status block bug action offset"
);
_Static_assert(
    offsetof(struct Exe6NaviStatusWorkFields, block_bug_severity) == 0x13,
    "Navi status block bug severity offset"
);
_Static_assert(
    offsetof(struct Exe6NaviStatusWorkFields, buster_bug_severity) == 0x14,
    "Navi status Buster bug severity offset"
);
_Static_assert(
    offsetof(struct Exe6NaviStatusWorkFields, buster_bug_level) == 0x15,
    "Navi status Buster bug level offset"
);
_Static_assert(
    offsetof(struct Exe6NaviStatusWorkFields, damage_hp_bug) == 0x16,
    "Navi status damage HP bug offset"
);
_Static_assert(
    offsetof(struct Exe6NaviStatusWorkFields, battle_hp_bug) == 0x18,
    "Navi status battle HP bug offset"
);
_Static_assert(
    offsetof(struct Exe6NaviStatusWorkFields, custom_hp_bug) == 0x19,
    "Navi status Custom HP bug offset"
);
_Static_assert(
    offsetof(struct Exe6NaviStatusWorkFields, color_bug) == 0x1A,
    "Navi status color bug offset"
);
_Static_assert(
    offsetof(struct Exe6NaviStatusWorkFields, emotion_bug) == 0x24,
    "Navi status emotion bug offset"
);
_Static_assert(
    offsetof(struct Exe6NaviStatusWorkFields, movement_bug) == 0x31,
    "Navi status movement bug offset"
);
_Static_assert(
    offsetof(struct Exe6NaviStatusWorkFields, custom_damage_bug) == 0x54,
    "Navi status Custom Damage bug offset"
);
_Static_assert(
    offsetof(struct Exe6NaviStatusWorkFields, custom_bug) == 0x63,
    "Navi status Custom bug offset"
);
_Static_assert(
    offsetof(struct Exe6HitFields, region) == 0x01,
    "hit region offset"
);
_Static_assert(
    sizeof(Exe6BlockDamageProperties) == sizeof(uint32_t),
    "block damage properties size"
);
_Static_assert(
    offsetof(Exe6BlockDamageProperties, region) == 0,
    "block damage region byte"
);
_Static_assert(
    offsetof(Exe6BlockDamageProperties, hit_effect) == 1,
    "block damage hit effect byte"
);
_Static_assert(
    offsetof(Exe6BlockDamageProperties, target_hit_type) == 2,
    "block damage target hit byte"
);
_Static_assert(
    offsetof(Exe6BlockDamageProperties, self_hit_type) == 3,
    "block damage self hit byte"
);
_Static_assert(
    offsetof(struct Exe6HitFields, hit_effect) == 0x09,
    "hit effect offset"
);
_Static_assert(
    offsetof(struct Exe6HitFields, base_hit_modifier) == 0x0E,
    "base hit modifier offset"
);
_Static_assert(
    offsetof(struct Exe6HitFields, final_hit_modifier) == 0x0F,
    "final hit modifier offset"
);
_Static_assert(
    offsetof(struct Exe6HitFields, fixed_invulnerability_timer) == 0x24,
    "fixed invulnerability timer offset"
);
_Static_assert(
    offsetof(struct Exe6HitFields, received_hit_flags) == 0x70,
    "received hit flags offset"
);
_Static_assert(
    offsetof(struct Exe6HitFields, final_damage) == 0x80,
    "final hit damage offset"
);
_Static_assert(
    offsetof(struct Exe6HitFields, damage_buckets) == 0x82,
    "hit damage buckets offset"
);
_Static_assert(offsetof(Exe6BattleState, state) == 0, "battle state offset");
_Static_assert(
    offsetof(Exe6BattleState, custom_screen_side) == 0x05,
    "battle Custom-screen side offset"
);
_Static_assert(
    offsetof(struct Exe6BattleContextFields, local_side) == 0x0D,
    "battle context local side offset"
);
_Static_assert(
    offsetof(struct Exe6BattleContextFields, regular_chip_available) == 0x17,
    "battle context regular availability offset"
);
_Static_assert(
    offsetof(struct Exe6BattleContextFields, tag_chips_available) == 0x44,
    "battle context work offset"
);
_Static_assert(
    offsetof(struct Exe6BattleContextFields, battle_units) == 0x80,
    "battle context unit offset"
);
_Static_assert(
    offsetof(struct Exe6BattleContextFields, obstacles) == 0xA0,
    "battle context obstacle offset"
);
_Static_assert(
    offsetof(struct Exe6BattleContextFields, active_units) == 0xD0,
    "battle context active unit offset"
);
_Static_assert(
    sizeof(struct Exe6BattleContextFields) == 0xF0,
    "battle context size"
);
_Static_assert(sizeof(Exe6ChipQueue) == 0x3C, "chip queue size");
_Static_assert(
    offsetof(struct Exe6RuntimeFields, battle_context) == 0x18,
    "runtime battle context offset"
);
Exe6Runtime *exe6_runtime(void);
void exe6_sound_req(uint32_t sound);
void exe6_obj_char_init(uint32_t mode, uint32_t group, uint32_t index);
void exe6_obj_shadow_set(void);
void exe6_obj_char_set(void);
void exe6_obj_char_move(void);
void exe6_obj_no_shadow(void);
void exe6_obj_dma_seq_set(uint32_t animation);
void exe6_obj_flip_set(uint32_t flip);
void exe6_obj_clt_set(uint32_t palette);
void exe6_obj_flash_set(void);
void exe6_obj_flash_reset(void);
void exe6_obj_col_efc_set(uint32_t scale);
void exe6_obj_prio_set(uint32_t priority);
uint32_t exe6_obj_seq_info_get(void);
uint32_t exe6_obj_clt_link_get(Exe6Obj *obj);
uint32_t exe6_obj_col_efc_link_get(Exe6Obj *obj);
void exe6_obj_mosaic_set(uint32_t low, uint32_t high);
void exe6_obj_no_trans_flag_num_set(uint32_t piece_index);
void exe6_obj_bld_link_copy(Exe6Obj *source);
void exe6_obj_flash_link_copy(Exe6Obj *source);
void exe6_obj_mosaic_link_copy(Exe6Obj *source);
void exe6_obj_bld_set(uint32_t blend);
void exe6_obj_bld_reset(void);
void exe6_obj_shadow_all_set(void);
void exe6_block_to_pos(void);
void exe6_battle_obj_char_init(uint32_t selector);
void exe6_battle_obj_char_move(void);
void exe6_battle_obj_char_move2(void);
void exe6_pos_to_block(void);
void exe6_obj_move_delete(void);
void exe6_obj_invoke(Exe6Obj *obj, uintptr_t entry);
void exe6_mem_task_trans_set256(const void *source, void *destination, uint32_t size);
void exe6_mem_trans256(const void *source, void *destination, uint32_t size);
void exe6_col_fade_set(
    uint32_t unused,
    uint32_t color,
    uint32_t count,
    uint32_t cache,
    uintptr_t destination
);
void exe6_col_fade_kill(uint32_t cache);

Exe6Obj *exe6_em_open(
    uint32_t type,
    Exe6ObjSpawnParameters spawn_parameters
);
Exe6Obj *exe6_shl_open(
    uint32_t type,
    int32_t x,
    int32_t y,
    int32_t z,
    Exe6ObjSpawnParameters spawn_parameters
);
Exe6Obj *exe6_efc_open(
    uint32_t type,
    Exe6ObjSpawnParameters spawn_parameters
);
Exe6Obj *exe6_efc_open_at(
    uint32_t type,
    int32_t x,
    int32_t y,
    int32_t z,
    Exe6ObjSpawnParameters spawn_parameters
);
Exe6Obj *exe6_set_efc00(
    uint32_t unused,
    int32_t x,
    int32_t y,
    int32_t z,
    uint32_t effect
);

uint32_t exe6_calc_pl_em_dir_spd_for(Exe6Obj *obj);
int32_t exe6_calc_pl_em_spd(void);
uint32_t exe6_enemy_flip_check(void);
uint32_t exe6_block_in_screen_check_sub(uint32_t x, uint32_t y);
uint32_t exe6_block_move_check(
    uint32_t x,
    uint32_t y,
    uint32_t required_flags,
    uint32_t excluded_flags
);
uint32_t exe6_another_block_exist_check(uint32_t x, uint32_t owner);
Exe6Block *exe6_block_at(uint32_t x, uint32_t y);
uint32_t exe6_block_status_get(uint32_t x, uint32_t y);
void exe6_block_flash(uint32_t x, uint32_t y);
void exe6_block_crack_set(uint32_t x, uint32_t y);
void exe6_block_out_set3(uint32_t x, uint32_t y);
uint64_t exe6_get_block_pos(uint32_t x, uint32_t y);
uint32_t exe6_block_in_screen_check(void);
uint32_t exe6_calc_degree(int32_t y, int32_t x);

void exe6_cube_entry(Exe6Obj *obj, uint32_t owner, uint32_t slot);
void exe6_cube_delete(void);
uint32_t exe6_cube_erase2(void);
void exe6_cube_set_dust_suikomi_efc(uint32_t kind);
void exe6_cube_life_span_check(void);

Exe6Hit *exe6_battle_hit_open(void);
void exe6_battle_hit_data_set(
    Exe6Hit *hit,
    Exe6HitType self_hit_type,
    Exe6HitType target_hit_type,
    uint32_t hit_modifier
);
void exe6_battle_hit_set(uint32_t unused, uint32_t region);
uint32_t exe6_battle_hit_req_flag_get(void);
void exe6_battle_hit_check(Exe6Hit *hit);
void exe6_battle_hit_off(Exe6Hit *hit);
void exe6_battle_hit_block_pos_set(void);
void exe6_battle_hit_hit_mark_check(void);
void exe6_battle_hit_hit_mark_set(Exe6HitEffect effect);
void exe6_battle_hit_status_change_set(uint32_t low, uint32_t high);
void exe6_battle_hit_close(Exe6Hit *hit);
void exe6_cube_hit_check(void);
void exe6_cube_guard_mark_check(void);
void exe6_enemy_life_sub(uint32_t damage);
uint32_t exe6_rand(void);
void exe6_cockpit_kokoro_navicus_bug_clear(void);

uint32_t exe6_battle_end_check(void);
uint32_t exe6_battle_event_busy_check(void);
uint32_t exe6_em_set_flag_get(void);
uint32_t exe6_real_operation_battle_check(void);
uint32_t exe6_battle_one_self_check(uint32_t side);
void exe6_battle_pause_on(void);
void exe6_cockpit_pause_set(void);
void exe6_battle_report_flag_on(uint32_t control_flags);
void exe6_battle_report_flag_off(uint32_t control_flags);

Exe6Obj *exe6_get_navi_adrs(uint32_t side);
uint32_t exe6_get_cur_pet_navi(void);
Exe6NaviStatusWork *exe6_navi_status_work_adrs_get(uint32_t side);
uint32_t exe6_navi_status_get(uint32_t side, uint32_t property);
void exe6_navi_status_set(
    uint32_t side,
    uint32_t property,
    uint32_t value
);
uint8_t *exe6_cur_pet_navi_stats_adrs_get(uint32_t navi);
uint8_t *exe6_special_navi_stats_adrs_get(uint32_t index);
const uint8_t *exe6_battle_key_work_adrs_get(uint32_t side);
void exe6_battle_hit_status_flag_off(
    Exe6Obj *player,
    uint32_t status_flags
);
Exe6NaviSelectChipWork *exe6_navi_select_chip_work_adrs_get(uint32_t side);
void exe6_battle_chip_set(void);
void exe6_battle_select_chip_work_init(void);
void exe6_deck_shuffle_sub(
    Exe6ChipQueue *queue,
    uint32_t preserve_regular,
    uint32_t preserve_tag
);
void exe6_cockpit_set_custom_gauge_value(uint32_t value);
uint32_t exe6_cockpit_get_custom_gauge_value(void);
void exe6_operate_slot_in_gauge_sub(uint32_t side, uint32_t amount);
void exe6_yazirushi_trans(uint32_t duration, uint32_t animation);
void exe6_camera_quake_set(uint32_t intensity, uint32_t duration);
Exe6Obj *exe6_set_shl03_ev(
    uint32_t block_x,
    uint32_t block_y,
    uint32_t parameter,
    uint32_t unused,
    Exe6BlockDamageProperties properties,
    uint32_t attack,
    uint32_t mode
);
uint32_t exe6_rand2(void);
void exe6_mem_clear8(void *destination, uint32_t size);
void exe6_mem_trans8(const void *source, void *destination, uint32_t size);
void exe6_shuffle_sub(void *bytes, uint32_t count, uint32_t range);
void exe6_set_efc0c(Exe6Obj *owner, uint32_t mode);
void exe6_saved_navi_dispatch(
    uintptr_t entry,
    Exe6Obj *owner,
    uint32_t block_x,
    uint32_t block_y,
    uint32_t parameter,
    uintptr_t data,
    uint32_t properties,
    uint8_t *completion
);

void exe6_event_chip_state_reset(uint32_t side);
void exe6_event_chip_common_init(void);
void exe6_event_chip_common_fade(void);
void exe6_event_chip_common_telop(void);
void exe6_event_chip_common_end(void);
void exe6_event_chip_common_exit(void);

#endif
