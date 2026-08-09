#include "runtime.h"

EXE6_SPRITE(bugcharge_gospel_sprite, "build/bugcharge-gospel-sprite.bin");

EXE6_INCBIN(bugcharge_icon, "build/bugcharge-icon.bin");
EXE6_INCBIN(bugcharge_image, "build/bugcharge-image.bin");
EXE6_INCBIN(bugcharge_palette, "build/bugcharge-palette.bin");
EXE6_SONG(
    bugcharge_charge_song,
    EXE6_PCM(
        bugcharge_charge,
        0x80,
        0x08,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
        ".byte 0xBE,0x70,0xD9,0x3C,0x7F,0x8A,0xB1\n",
        "build/bugcharge-charge-sample.bin"
    )
);
EXE6_SONG(
    bugcharge_fire_song,
    ".byte 1,0,0x80,0\n"
    ".long bugcharge_fire_voicegroup\n"
    ".long bugcharge_fire_track\n"
    ".global bugcharge_fire_voicegroup\n"
    "bugcharge_fire_voicegroup:\n"
    ".byte 0x0C,0x3C,0,0\n"
    ".long 0\n"
    ".byte 0,3,0,0\n"
    ".global bugcharge_fire_track\n"
    "bugcharge_fire_track:\n"
    ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
    ".byte 0xBE,0x60,0xD2,0x3C,0x7F,0x83,0xEA,0x48\n"
    ".byte 0x9B,0x81,0xB1\n"
);

static const uint8_t ACTIVE_STATE = 4;
static const uint8_t EFFECT_PHASE = 8;
static const uint8_t OUTRO_PHASE = 0x0C;
static const uint8_t FIRE_SUBSTATE = 4;
static const uint8_t COOLDOWN_SUBSTATE = 8;
static const uint8_t GOSPEL_VISUAL = 25;
static const uint16_t CHARGE_FRAMES = 39;
static const uint16_t SHOT_INTERVAL = 14;
static const uint16_t FINAL_COOLDOWN = 29;
static const Exe6HitType HIT_SELECTOR =
    EXE6_HIT_TYPE_STANDARD_TARGET;
static const uint32_t PRESENT_HIT_VALUE = HIT_SELECTOR << 3;
static const uint32_t EXTENDED_HIT_VALUE =
    PRESENT_HIT_VALUE << 8;

static const uint8_t BYTE_PROPERTIES[] = {
    0x13, 0x14, 0x16, 0x19, 0x18, 0x1A, 0x63,
};

static uint16_t count_and_clear_bugs(Exe6Obj *controller)
{
    uint8_t *properties = exe6_navi_status_work_adrs_get(controller->owner);
    uint16_t count = 1;

    if (properties[0x31] != 0) {
        ++count;
        properties[0x31] = 0;
    }
    for (size_t index = 0;
         index < sizeof(BYTE_PROPERTIES) / sizeof(BYTE_PROPERTIES[0]);
         ++index) {
        uint8_t offset = BYTE_PROPERTIES[index];
        if (properties[offset] != 0) {
            ++count;
            properties[offset] = 0;
        }
    }

    uint16_t *halfword = (uint16_t *)(properties + 0x54);
    if (*halfword != 0) {
        ++count;
        *halfword = 0;
    }
    if (properties[0x24] != 0) {
        ++count;
        properties[0x24] = 0;
    }
    exe6_cockpit_kokoro_navicus_bug_clear();
    return count;
}

static void head_set_position(Exe6Obj *self)
{
    Exe6Obj *player = self->parent;
    self->x = player->x;
    self->y = player->y;
    int32_t direction = (int32_t)exe6_calc_pl_em_dir_spd_for(self);
    self->x += direction * (24 << 16);
    self->z = 0x17 << 16;
}

static void head_init(Exe6Obj *self)
{
    exe6_obj_char_init(
        0x80,
        EXE6_SPRITE_GROUP(bugcharge_gospel_sprite),
        EXE6_SPRITE_ID(bugcharge_gospel_sprite)
    );
    exe6_obj_char_set();
    exe6_obj_no_shadow();
    self->animation = 1;
    self->palette = 1;
    exe6_obj_dma_seq_set(1);
    exe6_obj_char_set();
    exe6_obj_char_move();
    exe6_obj_flip_set(exe6_enemy_flip_check());
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    self->state_word = ACTIVE_STATE;
    head_set_position(self);
}

static bool head_update(Exe6Obj *self)
{
    if (exe6_battle_end_check() != 0 || self->parent == NULL) {
        exe6_obj_move_delete();
        return false;
    }
    int32_t timer = (int32_t)self->aux_timer - 1;
    self->aux_timer = (uint16_t)timer;
    if (timer < 0) {
        exe6_obj_move_delete();
        return false;
    }
    head_set_position(self);
    return true;
}

static void spawn_charge_head(
    Exe6Obj *controller,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    Exe6Obj *player = exe6_get_navi_adrs(controller->owner);
    if (player == NULL) {
        return;
    }
    Exe6Obj *head = exe6_efc_open(
        EXE6_OBJ_ID(bugcharge_head_main), spawn_parameters
    );
    if (head == NULL) {
        return;
    }
    head->aux_timer = (uint16_t)(controller->timer * 15u + 55u);
    head->owner_word = controller->owner_word;
    head->parent = player;
}

static void spawn_gospel(Exe6Obj *controller)
{
    int32_t direction =
        (int32_t)exe6_calc_pl_em_dir_spd_for(controller);
    Exe6Obj *gospel = exe6_shl_open(
        EXE6_OBJ_ID(bugcharge_gospel_main),
        0,
        0,
        0,
        exe6_obj_spawn_with_variant(GOSPEL_VISUAL)
    );
    if (gospel == NULL) {
        return;
    }
    gospel->block_x = (uint8_t)((int32_t)controller->block_x + direction);
    gospel->block_y = controller->block_y;
    gospel->owner_word = controller->owner_word;
    gospel->attack = controller->attack;
    gospel->parent = controller;
    gospel->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
}

static void effect_update(
    Exe6Obj *self,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    if (self->substate == 0) {
        self->timer = count_and_clear_bugs(self);
        spawn_charge_head(self, spawn_parameters);
        exe6_sound_req(EXE6_SONG_ID(bugcharge_charge_song));
        self->aux_timer = CHARGE_FRAMES;
        self->substate = FIRE_SUBSTATE;
        return;
    }

    int32_t delay = (int32_t)self->aux_timer - 1;
    self->aux_timer = (uint16_t)delay;
    if (delay >= 0) {
        return;
    }

    if (self->substate == FIRE_SUBSTATE) {
        exe6_sound_req(EXE6_SONG_ID(bugcharge_fire_song));
        spawn_gospel(self);
        exe6_camera_quake_set(2, 20);
        if (--self->timer != 0) {
            self->aux_timer = SHOT_INTERVAL;
        } else {
            self->aux_timer = FINAL_COOLDOWN;
            self->substate = COOLDOWN_SUBSTATE;
        }
        return;
    }

    self->phase = OUTRO_PHASE;
    self->phase_timer = 0;
}

static void controller_update(
    Exe6Obj *self,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    if (self->phase == 0) {
        exe6_event_chip_common_fade();
    } else if (self->phase == 4) {
        exe6_event_chip_common_telop();
    } else if (self->phase == EFFECT_PHASE) {
        effect_update(self, spawn_parameters);
    } else {
        exe6_event_chip_common_end();
    }
}

static void free_hit(Exe6Obj *self)
{
    Exe6Hit *hit = self->hit;
    if (hit != NULL) {
        exe6_battle_hit_off(hit);
        exe6_battle_hit_close(self->hit);
    }
    exe6_obj_move_delete();
}

static bool hit_init(Exe6Obj *self)
{
    if (exe6_battle_end_check() != 0) {
        exe6_obj_move_delete();
        return false;
    }
    exe6_block_to_pos();
    self->z = 0x10 << 16;
    Exe6Hit *hit = exe6_battle_hit_open();
    if (hit == NULL) {
        exe6_obj_move_delete();
        return false;
    }
    exe6_battle_hit_data_set(
        hit,
        EXE6_HIT_TYPE_0A,
        HIT_SELECTOR,
        3
    );
    hit->region = 1;
    exe6_battle_hit_hit_mark_set(EXE6_HIT_EFFECT_FIRE);
    exe6_battle_hit_set(0, PRESENT_HIT_VALUE);
    self->state_word = ACTIVE_STATE;
    return true;
}

static void hit_update(Exe6Obj *self)
{
    Exe6Hit *hit = self->hit;
    exe6_battle_hit_check(hit);
    exe6_battle_hit_hit_mark_check();
    exe6_battle_hit_off(hit);
    exe6_battle_hit_close(self->hit);
    exe6_obj_move_delete();
}

static void spawn_hit(Exe6Obj *source, uint32_t block_x, uint32_t block_y)
{
    Exe6Obj *hit = exe6_shl_open(
        EXE6_OBJ_ID(bugcharge_hit_main),
        0,
        0,
        0,
        exe6_obj_spawn_with_variant(GOSPEL_VISUAL)
    );
    if (hit == NULL) {
        return;
    }
    hit->block_x = (uint8_t)block_x;
    hit->block_y = (uint8_t)block_y;
    hit->owner_word = source->owner_word;
    hit->attack = source->attack;
    hit->parent = source;
    hit->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
}

static USED void attack_row(Exe6Obj *source)
{
    uint32_t opposing_navi_flag = source->owner == 0
        ? EXE6_BLOCK_FLAG_SIDE_1_NAVI
        : EXE6_BLOCK_FLAG_SIDE_0_NAVI;
    for (uint32_t block_x = 6; block_x != 0; --block_x) {
        if (exe6_block_move_check(
                block_x,
                source->block_y,
                opposing_navi_flag,
                0
            ) != 0) {
            spawn_hit(source, block_x, source->block_y);
        }
    }
}

static bool gospel_init(Exe6Obj *self)
{
    if (exe6_battle_end_check() != 0) {
        exe6_obj_move_delete();
        return false;
    }
    exe6_block_to_pos();
    self->z = 0x14 << 16;
    exe6_obj_char_init(
        0x80,
        EXE6_SPRITE_GROUP(bugcharge_gospel_sprite),
        EXE6_SPRITE_ID(bugcharge_gospel_sprite)
    );
    exe6_obj_char_set();
    exe6_obj_no_shadow();
    self->animation = 0;
    self->palette = 0;
    exe6_obj_dma_seq_set(0);
    exe6_obj_char_set();
    exe6_obj_flip_set(exe6_enemy_flip_check());
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;

    Exe6Hit *hit = exe6_battle_hit_open();
    if (hit == NULL) {
        exe6_obj_move_delete();
        return false;
    }
    exe6_battle_hit_data_set(
        hit,
        EXE6_HIT_TYPE_STANDARD_ATTACK,
        HIT_SELECTOR,
        3
    );
    exe6_battle_hit_hit_mark_set(EXE6_HIT_EFFECT_SMALL_IMPACT);
    exe6_battle_hit_status_change_set(0, PRESENT_HIT_VALUE);
    exe6_battle_hit_set(0, EXTENDED_HIT_VALUE);
    self->animation_state = 8;
    self->state_word = ACTIVE_STATE;
    return true;
}

static bool gospel_update(Exe6Obj *self)
{
    if (exe6_battle_end_check() != 0) {
        free_hit(self);
        return false;
    }

    Exe6Hit *hit = self->hit;
    exe6_battle_hit_check(hit);
    exe6_battle_hit_hit_mark_check();
    if (hit->received_hit_flags != 0) {
        free_hit(self);
        return false;
    }

    int32_t direction = (int32_t)exe6_calc_pl_em_dir_spd_for(self);
    self->x += direction * (10 << 16);
    exe6_pos_to_block();
    exe6_battle_hit_block_pos_set();
    if (exe6_block_in_screen_check() == 0) {
        free_hit(self);
        return false;
    }

    uint64_t coordinates = exe6_get_block_pos(self->block_x, self->block_y);
    int32_t block_x = (int32_t)(uint32_t)coordinates;
    uint32_t block_y = (uint32_t)(coordinates >> 32);
    if (self->x == block_x && --self->animation_state == 0) {
        free_hit(self);
        return false;
    }
    exe6_battle_hit_set(0, block_y);
    return true;
}

EXE6_SHELL(bugcharge_hit_main)
{
    if (self->state == 0) {
        if (!hit_init(self)) {
            return;
        }
    } else if (self->state == ACTIVE_STATE) {
        hit_update(self);
        return;
    } else {
        exe6_obj_move_delete();
        return;
    }
    exe6_battle_obj_char_move2();
}

EXE6_SHELL(bugcharge_gospel_main)
{
    if (self->state == 0) {
        if (!gospel_init(self) || !gospel_update(self)) {
            return;
        }
    } else if (self->state == ACTIVE_STATE) {
        if (!gospel_update(self)) {
            return;
        }
    } else {
        exe6_obj_move_delete();
        return;
    }
    exe6_battle_obj_char_move2();
}

EXE6_EFFECT(bugcharge_head_main)
{
    if (self->state == 0) {
        head_init(self);
    } else if (self->state == ACTIVE_STATE) {
        if (!head_update(self)) {
            return;
        }
    } else {
        exe6_obj_move_delete();
        return;
    }
    exe6_battle_obj_char_move2();
}

EXE6_EFFECT(bugcharge_controller_main)
{
    if (self->state == 0) {
        exe6_event_chip_common_init();
    } else if (self->state == ACTIVE_STATE) {
        controller_update(self, spawn_parameters);
    } else {
        exe6_event_chip_common_exit();
    }
}

EXE6_PERSISTENT_ATTACK(0x131, bugcharge_attack_main)
{
    Exe6Obj *controller = exe6_efc_open(
        EXE6_OBJ_ID(bugcharge_controller_main), spawn_parameters
    );
    if (controller == NULL) {
        return NULL;
    }
    controller->block_x = (uint8_t)block_x;
    controller->block_y = (uint8_t)block_y;
    controller->parameter = (uint8_t)parameter;
    controller->parent = owner;
    controller->owner_word = owner->owner_word;
    controller->attack = attack;
    controller->chip_data = chip_data;
    return controller;
}
