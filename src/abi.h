#ifndef ABI_H
#define ABI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NAKED __attribute__((naked, noinline, used))
#define USED __attribute__((used))

typedef struct ObjectFields Object;
typedef struct PlayerRuntimeFields PlayerRuntime;
typedef struct CollisionFields Collision;
typedef struct BattleContextFields BattleContext;
typedef struct RuntimeFields Runtime;

#define BN6_PALETTE_BG_STAGING_00 ((uintptr_t)0x03001550u)
#define BN6_PALETTE_BG_STAGING_01 ((uintptr_t)0x03001570u)
#define BN6_PALETTE_BG_STAGING_02 ((uintptr_t)0x03001590u)
#define BN6_PALETTE_BG_STAGING_03 ((uintptr_t)0x030015B0u)
#define BN6_PALETTE_BG_STAGING_04 ((uintptr_t)0x030015D0u)
#define BN6_PALETTE_BG_STAGING_05 ((uintptr_t)0x030015F0u)
#define BN6_PALETTE_BG_STAGING_06 ((uintptr_t)0x03001610u)
#define BN6_PALETTE_BG_STAGING_07 ((uintptr_t)0x03001630u)
#define BN6_PALETTE_BG_STAGING_08 ((uintptr_t)0x03001650u)
#define BN6_PALETTE_BG_STAGING_09 ((uintptr_t)0x03001670u)
#define BN6_PALETTE_BG_STAGING_0A ((uintptr_t)0x03001690u)
#define BN6_PALETTE_BG_STAGING_0B ((uintptr_t)0x030016B0u)
#define BN6_PALETTE_BG_STAGING_0C ((uintptr_t)0x030016D0u)
#define BN6_PALETTE_BG_STAGING_0D ((uintptr_t)0x030016F0u)
#define BN6_PALETTE_BG_STAGING_0E ((uintptr_t)0x03001710u)
#define BN6_PALETTE_BG_STAGING_0F ((uintptr_t)0x03001730u)

#define BN6_PALETTE_BG_OUTPUT_00 ((uintptr_t)0x03001750u)
#define BN6_PALETTE_BG_OUTPUT_01 ((uintptr_t)0x03001770u)
#define BN6_PALETTE_BG_OUTPUT_02 ((uintptr_t)0x03001790u)
#define BN6_PALETTE_BG_OUTPUT_03 ((uintptr_t)0x030017B0u)
#define BN6_PALETTE_BG_OUTPUT_04 ((uintptr_t)0x030017D0u)
#define BN6_PALETTE_BG_OUTPUT_05 ((uintptr_t)0x030017F0u)
#define BN6_PALETTE_BG_OUTPUT_06 ((uintptr_t)0x03001810u)
#define BN6_PALETTE_BG_OUTPUT_07 ((uintptr_t)0x03001830u)
#define BN6_PALETTE_BG_OUTPUT_08 ((uintptr_t)0x03001850u)
#define BN6_PALETTE_BG_OUTPUT_09 ((uintptr_t)0x03001870u)
#define BN6_PALETTE_BG_OUTPUT_0A ((uintptr_t)0x03001890u)
#define BN6_PALETTE_BG_OUTPUT_0B ((uintptr_t)0x030018B0u)
#define BN6_PALETTE_BG_OUTPUT_0C ((uintptr_t)0x030018D0u)
#define BN6_PALETTE_BG_OUTPUT_0D ((uintptr_t)0x030018F0u)
#define BN6_PALETTE_BG_OUTPUT_0E ((uintptr_t)0x03001910u)
#define BN6_PALETTE_BG_OUTPUT_0F ((uintptr_t)0x03001930u)

#define BN6_PALETTE_OBJ_STAGING_00 ((uintptr_t)0x03001960u)
#define BN6_PALETTE_OBJ_STAGING_01 ((uintptr_t)0x03001980u)
#define BN6_PALETTE_OBJ_STAGING_02 ((uintptr_t)0x030019A0u)
#define BN6_PALETTE_OBJ_STAGING_03 ((uintptr_t)0x030019C0u)
#define BN6_PALETTE_OBJ_STAGING_04 ((uintptr_t)0x030019E0u)
#define BN6_PALETTE_OBJ_STAGING_05 ((uintptr_t)0x03001A00u)
#define BN6_PALETTE_OBJ_STAGING_06 ((uintptr_t)0x03001A20u)
#define BN6_PALETTE_OBJ_STAGING_07 ((uintptr_t)0x03001A40u)
#define BN6_PALETTE_OBJ_STAGING_08 ((uintptr_t)0x03001A60u)
#define BN6_PALETTE_OBJ_STAGING_09 ((uintptr_t)0x03001A80u)
#define BN6_PALETTE_OBJ_STAGING_0A ((uintptr_t)0x03001AA0u)
#define BN6_PALETTE_OBJ_STAGING_0B ((uintptr_t)0x03001AC0u)
#define BN6_PALETTE_OBJ_STAGING_0C ((uintptr_t)0x03001AE0u)
#define BN6_PALETTE_OBJ_STAGING_0D ((uintptr_t)0x03001B00u)
#define BN6_PALETTE_OBJ_STAGING_0E ((uintptr_t)0x03001B20u)
#define BN6_PALETTE_OBJ_STAGING_0F ((uintptr_t)0x03001B40u)

#define BN6_PALETTE_OBJ_OUTPUT_00 ((uintptr_t)0x03001B60u)
#define BN6_PALETTE_OBJ_OUTPUT_01 ((uintptr_t)0x03001B80u)
#define BN6_PALETTE_OBJ_OUTPUT_02 ((uintptr_t)0x03001BA0u)
#define BN6_PALETTE_OBJ_OUTPUT_03 ((uintptr_t)0x03001BC0u)
#define BN6_PALETTE_OBJ_OUTPUT_04 ((uintptr_t)0x03001BE0u)
#define BN6_PALETTE_OBJ_OUTPUT_05 ((uintptr_t)0x03001C00u)
#define BN6_PALETTE_OBJ_OUTPUT_06 ((uintptr_t)0x03001C20u)
#define BN6_PALETTE_OBJ_OUTPUT_07 ((uintptr_t)0x03001C40u)
#define BN6_PALETTE_OBJ_OUTPUT_08 ((uintptr_t)0x03001C60u)
#define BN6_PALETTE_OBJ_OUTPUT_09 ((uintptr_t)0x03001C80u)
#define BN6_PALETTE_OBJ_OUTPUT_0A ((uintptr_t)0x03001CA0u)
#define BN6_PALETTE_OBJ_OUTPUT_0B ((uintptr_t)0x03001CC0u)
#define BN6_PALETTE_OBJ_OUTPUT_0C ((uintptr_t)0x03001CE0u)
#define BN6_PALETTE_OBJ_OUTPUT_0D ((uintptr_t)0x03001D00u)
#define BN6_PALETTE_OBJ_OUTPUT_0E ((uintptr_t)0x03001D20u)
#define BN6_PALETTE_OBJ_OUTPUT_0F ((uintptr_t)0x03001D40u)

#define BN6_JOIN_INNER(left, right) left##right
#define BN6_JOIN(left, right) BN6_JOIN_INNER(left, right)
#define BN6_STRINGIFY_INNER(value) #value
#define BN6_STRINGIFY(value) BN6_STRINGIFY_INNER(value)

/* ObjectHeader.Flags (+0x00). */
#define BN6_OBJECT_FLAG_ACTIVE 0x01
#define BN6_OBJECT_FLAG_VISIBLE 0x02
#define BN6_OBJECT_FLAG_UPDATE_DURING_PAUSE 0x04
#define BN6_OBJECT_FLAG_STOP_SPRITE_UPDATE 0x08
#define BN6_OBJECT_FLAG_UPDATE_DURING_TIME_STOP 0x10

/* Flags returned for the current sprite-animation frame. */
#define BN6_ANIMATION_FRAME_FLAG_END 0x80

/* PanelData.Flags (+0x14). */
#define BN6_PANEL_FLAG_SOLID 0x00000010
#define BN6_PANEL_FLAG_CRACKED 0x00000040
#define BN6_PANEL_FLAG_VALID 0x00010000
/* Live Navi collision records contribute these flags to their panel. */
#define BN6_PANEL_FLAG_SIDE_1_NAVI 0x00200000
#define BN6_PANEL_FLAG_SIDE_0_NAVI 0x00400000
/* Generic live collision records contribute these alliance flags. */
#define BN6_PANEL_FLAG_SIDE_1_COLLISION 0x04000000
#define BN6_PANEL_FLAG_SIDE_0_COLLISION 0x08000000

/* CollisionData.ObjectFlags1 (+0x3C). */
#define BN6_COLLISION_STATUS_FLAG_AIR_SHOES 0x00000010
#define BN6_COLLISION_STATUS_FLAG_FLOAT_SHOES 0x00000020
#define BN6_COLLISION_STATUS_FLAG_SUPER_ARMOR 0x00020000
#define BN6_COLLISION_STATUS_FLAG_UNDERSHIRT 0x00040000

/* CollisionData.ObjectFlags2 (+0x40). */
#define BN6_COLLISION_SECONDARY_FLAG_WIND_REMOVAL 0x00040000
#define BN6_COLLISION_SECONDARY_FLAG_DUST_SUCTION_SIDE_0 0x00100000
#define BN6_COLLISION_SECONDARY_FLAG_DUST_SUCTION_SIDE_1 0x00200000

/* BattleState control flags at +0x5C. */
#define BN6_BATTLE_CONTROL_FLAG_SIDE_0_CHIPS_ENABLED 0x04
#define BN6_BATTLE_CONTROL_FLAG_SIDE_1_CHIPS_ENABLED 0x08

/* Battle configuration flags. */
#define BN6_BATTLE_CONFIG_FLAG_LINK 0x08

/* Joypad state bits. */
#define BN6_KEY_RIGHT 0x10
#define BN6_KEY_LEFT 0x20
#define BN6_KEY_UP 0x40
#define BN6_KEY_DOWN 0x80

/*
 * Convert BN6's native register conventions to the C ABI.  These veneers
 * must use BL so the C helper receives an odd Thumb return address.
 */
#define BN6_EXPORT_OBJECT(name, target) \
    NAKED void name(void) \
    { \
        __asm__( \
            ".syntax unified\n" \
            "push {lr}\n" \
            "adds r0,r5,#0\n" \
            "adds r1,r4,#0\n" \
            "bl " BN6_STRINGIFY(target) "\n" \
            "pop {pc}\n" \
        ); \
    }

#define BN6_EXPORT_ATTACK(name, target) \
    NAKED void name(void) \
    { \
        __asm__( \
            ".syntax unified\n" \
            "push {r4,lr}\n" \
            "push {r6,r7}\n" \
            "adds r3,r5,#0\n" \
            "bl " BN6_STRINGIFY(target) "\n" \
            "add sp,#8\n" \
            "pop {r4,pc}\n" \
        ); \
    }

struct ObjectFields {
    uint8_t header_flags;                // +0x00, BN6_OBJECT_FLAG_*
    uint8_t unknown_01[3];
    union {
        uint16_t variant_word;            // +0x04
        struct {
            uint8_t variant;              // +0x04
            uint8_t subvariant;            // +0x05
        };
    };
    union {
        uint16_t animation_state_word;    // +0x06
        struct {
            uint8_t animation_state;      // +0x06
            uint8_t removal_state;        // +0x07
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
    uint8_t panel_x;                     // +0x12
    uint8_t panel_y;                     // +0x13
    uint8_t target_panel_x;              // +0x14
    uint8_t target_panel_y;              // +0x15
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
    uint8_t unknown_28[4];
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
    Object *parent;                      // +0x4C
    uint8_t unknown_50[4];
    Collision *collision;                // +0x54
    void *runtime_data;                  // +0x58
    uint32_t engine_reserved;            // +0x5C
    uint8_t work[0x20];                  // +0x60 through +0x7C
};

struct PlayerRuntimeFields {
    uint8_t unknown_00[7];
    uint8_t active_power_attack;         // +0x07
    uint8_t b_left;                      // +0x08
};

struct CollisionFields {
    uint8_t enabled;                     // +0x00
    uint8_t region;                      // +0x01
    uint8_t unknown_02[0x6E];
    uint32_t received_collision_type_flags; // +0x70
    uint8_t unknown_74[0x0C];
    uint16_t final_damage;               // +0x80
};

struct BattleContextFields {
    uint8_t unknown_00[0x0D];
    uint8_t local_side;                  // +0x0D
    uint8_t unknown_0e[0x09];
    uint8_t regular_available;           // +0x17
    uint8_t unknown_18[0x2C];
    uint8_t work_44;                     // +0x44
    uint8_t unknown_45[0x3B];
    Object *battle_units[2][4];          // +0x80
    Object *live_objects[8];             // +0xA0
    uint8_t unknown_c0[0x10];
    Object *active_units[2][4];          // +0xD0
};

struct RuntimeFields {
    uint8_t unknown_00[0x18];
    BattleContext *battle_context;       // +0x18
};

_Static_assert(offsetof(struct ObjectFields, state_word) == 0x08, "object state offset");
_Static_assert(offsetof(struct ObjectFields, variant) == 0x04, "object variant offset");
_Static_assert(offsetof(struct ObjectFields, parameter) == 0x0E, "object parameter offset");
_Static_assert(offsetof(struct ObjectFields, panel_x) == 0x12, "object panel offset");
_Static_assert(offsetof(struct ObjectFields, owner_word) == 0x16, "object owner offset");
_Static_assert(offsetof(struct ObjectFields, timer) == 0x20, "object timer offset");
_Static_assert(offsetof(struct ObjectFields, attack) == 0x2C, "object attack offset");
_Static_assert(offsetof(struct ObjectFields, chip_data) == 0x30, "object chip data offset");
_Static_assert(offsetof(struct ObjectFields, attack_bonus) == 0x32, "object attack bonus offset");
_Static_assert(offsetof(struct ObjectFields, velocity_x) == 0x40, "object velocity offset");
_Static_assert(offsetof(struct ObjectFields, velocity_z) == 0x48, "object velocity z offset");
_Static_assert(offsetof(struct ObjectFields, parent) == 0x4C, "object parent offset");
_Static_assert(offsetof(struct ObjectFields, collision) == 0x54, "object collision offset");
_Static_assert(offsetof(struct ObjectFields, runtime_data) == 0x58, "object runtime data offset");
_Static_assert(
    offsetof(struct ObjectFields, engine_reserved) == 0x5C,
    "object engine-reserved offset"
);
_Static_assert(offsetof(struct ObjectFields, work) == 0x60, "object work offset");
_Static_assert(offsetof(struct ObjectFields, completion) == 0x30, "object completion offset");
_Static_assert(
    offsetof(struct PlayerRuntimeFields, active_power_attack) == 0x07,
    "player runtime active power attack offset"
);
_Static_assert(
    offsetof(struct PlayerRuntimeFields, b_left) == 0x08,
    "player runtime B-Left offset"
);
_Static_assert(
    offsetof(struct CollisionFields, region) == 0x01,
    "collision region offset"
);
_Static_assert(
    offsetof(struct CollisionFields, received_collision_type_flags) == 0x70,
    "received collision type flags offset"
);
_Static_assert(
    offsetof(struct CollisionFields, final_damage) == 0x80,
    "final collision damage offset"
);
_Static_assert(
    offsetof(struct BattleContextFields, local_side) == 0x0D,
    "battle context local side offset"
);
_Static_assert(
    offsetof(struct BattleContextFields, regular_available) == 0x17,
    "battle context regular availability offset"
);
_Static_assert(
    offsetof(struct BattleContextFields, work_44) == 0x44,
    "battle context work offset"
);
_Static_assert(
    offsetof(struct BattleContextFields, battle_units) == 0x80,
    "battle context unit offset"
);
_Static_assert(
    offsetof(struct BattleContextFields, live_objects) == 0xA0,
    "battle context live object offset"
);
_Static_assert(
    offsetof(struct BattleContextFields, active_units) == 0xD0,
    "battle context active unit offset"
);
_Static_assert(
    sizeof(struct BattleContextFields) == 0xF0,
    "battle context size"
);
_Static_assert(
    offsetof(struct RuntimeFields, battle_context) == 0x18,
    "runtime battle context offset"
);
void bn6_play_sound(uint32_t sound);
void bn6_self_sprite_load(uint32_t mode, uint32_t group, uint32_t index);
void bn6_self_sprite_property_2e3c(void);
void bn6_self_sprite_load_animation_data(void);
void bn6_self_sprite_update(void);
void bn6_self_sprite_no_shadow(void);
void bn6_self_sprite_set_animation(uint32_t animation);
void bn6_self_sprite_set_flip(uint32_t flip);
void bn6_self_sprite_set_palette(uint32_t palette);
void bn6_self_sprite_flash_white(void);
void bn6_self_sprite_set_scale(uint32_t scale);
void bn6_self_sprite_set_priority(uint32_t priority);
uint32_t bn6_self_sprite_get_frame_flags(void);
uint32_t bn6_sprite_get_palette(Object *object);
uint32_t bn6_sprite_get_scale(Object *object);
void bn6_self_sprite_set_blend(uint32_t low, uint32_t high);
void bn6_self_sprite_hide_piece(uint32_t piece_index);
void bn6_self_sprite_copy_visibility(Object *source);
void bn6_self_sprite_copy_palette_bits(Object *source);
void bn6_self_sprite_copy_special_bits(Object *source);
void bn6_self_sprite_set_blend_mode(uint32_t blend);
void bn6_self_sprite_property_2cce(void);
void bn6_self_death_sprite_special(void);
void bn6_self_object_set_coords(void);
void bn6_self_object_load_navi_sprite(uint32_t selector);
void bn6_self_object_update(void);
void bn6_self_object_update_timestop(void);
void bn6_self_object_update_panel(void);
void bn6_self_object_free(void);
void bn6_object_invoke(Object *object, uintptr_t entry);
void bn6_display_setup(const void *source, void *destination, uint32_t size);
void bn6_palette_write(
    uint32_t unused,
    uint32_t color,
    uint32_t count,
    uint32_t cache,
    uintptr_t destination
);
void bn6_palette_restore(uint32_t cache);

Object *bn6_spawn_type1(uint32_t type, uint32_t implicit_r4);
Object *bn6_spawn_type3(
    uint32_t type,
    int32_t x,
    int32_t y,
    int32_t z,
    uint32_t implicit_r4
);
Object *bn6_spawn_type4(uint32_t type, uint32_t implicit_r4);
Object *bn6_spawn_type4_at(
    uint32_t type,
    int32_t x,
    int32_t y,
    int32_t z,
    uint32_t implicit_r4
);
Object *bn6_spawn_battle_effect(
    uint32_t unused,
    int32_t x,
    int32_t y,
    int32_t z,
    uint32_t effect
);

uint32_t bn6_object_front_direction_for(Object *object);
int32_t bn6_self_object_side_direction(void);
uint32_t bn6_self_object_get_flip(void);
uint32_t bn6_panel_is_valid_xy(uint32_t x, uint32_t y);
uint32_t bn6_panel_matches_flags(
    uint32_t x,
    uint32_t y,
    uint32_t required_flags,
    uint32_t excluded_flags
);
uint32_t bn6_panel_get_parameters(uint32_t x, uint32_t owner);
uint32_t bn6_panel_get_flags(uint32_t x, uint32_t y);
void bn6_panel_set_flash(uint32_t x, uint32_t y);
void bn6_panel_crack_from_solid(uint32_t x, uint32_t y);
void bn6_panel_crack(uint32_t x, uint32_t y);
uint64_t bn6_panel_to_coords(uint32_t x, uint32_t y);
uint32_t bn6_self_panel_is_valid_object(void);
uint32_t bn6_angle_from_vector(int32_t y, int32_t x);

void bn6_object_register_deployable(Object *object, uint32_t owner, uint32_t slot);
void bn6_self_object_unregister_deployable(void);
uint32_t bn6_self_object_update_suction(void);
void bn6_self_object_store_dust_ammo(uint32_t kind);
void bn6_self_deployable_lifetime_update(void);

Collision *bn6_self_collision_create(void);
void bn6_collision_setup(Collision *collision, uint32_t passive_region, uint32_t hit_region, uint32_t mode);
void bn6_self_collision_present(uint32_t unused, uint32_t region);
uint32_t bn6_self_collision_get_secondary_flags(void);
void bn6_collision_remove(Collision *collision);
void bn6_collision_clear_region(Collision *collision);
void bn6_self_collision_update_panel(void);
void bn6_self_collision_spawn_effect(void);
void bn6_self_collision_set_hit_effect(uint32_t effect);
void bn6_self_collision_set_extended_effect(uint32_t low, uint32_t high);
void bn6_collision_free(Collision *collision);
void bn6_self_field_collision_update(void);
void bn6_self_object_apply_damage(uint32_t damage);
uint32_t bn6_rng_next(void);
void bn6_bugfix_clear_runtime_state(void);

uint32_t bn6_battle_is_over(void);
uint32_t bn6_battle_is_time_stopped(void);
uint32_t bn6_battle_get_config_flags(void);
uint32_t bn6_link_battle_active(void);
uint32_t bn6_compare_local_side(uint32_t side);
void bn6_lock_battle_state(void);
void bn6_begin_local_custom(void);
void bn6_battle_set_control_flags(uint32_t control_flags);
void bn6_battle_clear_control_flags(uint32_t control_flags);

Object *bn6_player_object_for_side(uint32_t side);
uint8_t *bn6_player_properties_for_side(uint32_t side);
uint32_t bn6_player_property_for_side(uint32_t side, uint32_t property);
void bn6_player_property_set_for_side(
    uint32_t side,
    uint32_t property,
    uint32_t value
);
const uint8_t *bn6_input_state_for_side(uint32_t side);
void bn6_player_clear_collision_status_flags(
    Object *player,
    uint32_t status_flags
);
const uint8_t *bn6_chip_list(uint32_t side);
uint8_t *bn6_player_data(uint32_t side);
void bn6_rebuild_folder(void);
void bn6_clear_hand(void);
void bn6_shuffle_chip_queue(void *queue, uint32_t reserved, uint32_t regular, uint32_t size);
void bn6_set_hud_gauge(uint32_t value);
void bn6_gauge_subtract(uint32_t side, uint32_t amount);
void bn6_draw_delete_overlay(uint32_t duration, uint32_t animation);
void bn6_screen_shake_set(uint32_t intensity, uint32_t duration);
Object *bn6_spawn_panel_damage(
    uint32_t panel_x,
    uint32_t panel_y,
    uint32_t parameter,
    uint32_t unused,
    uint32_t properties,
    uint32_t attack,
    uint32_t mode
);
uint32_t bn6_battle_rng(void);
void bn6_memory_clear(void *destination, uint32_t size);
void bn6_memory_copy(const void *source, void *destination, uint32_t size);
void bn6_shuffle_bytes(void *bytes, uint32_t count, uint32_t range);
void bn6_saved_navi_intro(Object *owner, uint32_t mode);
void bn6_saved_navi_dispatch(
    uintptr_t entry,
    Object *owner,
    uint32_t panel_x,
    uint32_t panel_y,
    uint32_t parameter,
    uintptr_t data,
    uint32_t properties,
    uint8_t *completion
);

void bn6_self_type4_timestop_init(void);
void bn6_self_type4_timestop_intro(void);
void bn6_self_type4_timestop_freeze(void);
void bn6_self_type4_timestop_outro(void);
void bn6_self_type4_timestop_free(void);

#endif
