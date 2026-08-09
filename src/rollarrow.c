#include "abi.h"
#include "runtime.h"

EXE6_USE_SONG(common_navi_summon_song);
EXE6_SPRITE(rollarrow_actor_sprite, "build/rollarrow-actor-sprite.bin");
EXE6_SPRITE(rollarrow_projectile_sprite, "build/rollarrow-projectile-sprite.bin");

EXE6_INCBIN(rollarrow1_icon, "build/rollarrow1-icon.bin");
EXE6_INCBIN(rollarrow2_icon, "build/rollarrow2-icon.bin");
EXE6_INCBIN(rollarrow3_icon, "build/rollarrow3-icon.bin");
EXE6_INCBIN(rollarrow_image, "build/rollarrow-image.bin");
EXE6_INCBIN(rollarrow1_palette, "build/rollarrow1-pal.bin");
EXE6_INCBIN(rollarrow2_palette, "build/rollarrow2-pal.bin");
EXE6_INCBIN(rollarrow3_palette, "build/rollarrow3-pal.bin");
EXE6_SONG(
    rollarrow_fire_song,
    EXE6_PCM(
        rollarrow_fire,
        0x40,
        0x08,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
        ".byte 0xBE,0x7F,0xE1,0x3C,0x7F,0x92,0xB1\n",
        "build/rollarrow-fire-sample.bin"
    )
);

static const uint16_t WAIT_FRAMES = 20;
static const uint16_t FIRE_HOLD_FRAMES = 6;
static const uint16_t POST_SHOT_FRAMES = 30;
static const uint16_t EXIT_FRAMES = 5;
static const int32_t PROJECTILE_SPEED = 0x00070000;
static const uint8_t PROJECTILE_BLOCKS = 8;
static const Exe6HitType PROJECTILE_HIT_TYPE =
    EXE6_HIT_TYPE_08;

static bool timer_expired(Exe6Obj *self)
{
    int32_t timer = (int32_t)self->timer - 1;
    self->timer = (uint16_t)timer;
    return timer < 0;
}

static void set_animation(Exe6Obj *self, uint32_t animation)
{
    self->animation = (uint8_t)animation;
    self->palette = UINT8_MAX;
    exe6_obj_dma_seq_set(animation);
    exe6_obj_char_set();
}

static void spawn_projectile(Exe6Obj *actor)
{
    Exe6Obj *projectile = exe6_shl_open(
        EXE6_OBJ_ID(rollarrow_arrow_main),
        actor->block_y,
        actor->parameter,
        0,
        exe6_obj_spawn_with_variant(PROJECTILE_HIT_TYPE)
    );
    if (projectile == NULL) {
        return;
    }

    projectile->block_x = actor->block_x;
    projectile->block_y = actor->block_y;
    projectile->parameter = actor->parameter;
    projectile->owner_word = actor->owner_word;
    projectile->attack = actor->attack;
    projectile->parent = actor;

    int32_t direction = (int32_t)exe6_calc_pl_em_dir_spd_for(actor);
    projectile->x = actor->x + direction * (8 << 16);
    projectile->y = ((actor->y >> 16) - 1) << 16;
    projectile->z = 0x24 << 16;
    projectile->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
}

static void actor_wait(Exe6Obj *self)
{
    if (self->substate == 0) {
        set_animation(self, 0);
        self->timer = WAIT_FRAMES;
        self->substate = 4;
    }
    if (timer_expired(self)) {
        self->phase = 4;
        self->substate = 0;
    }
}

static void actor_fire(Exe6Obj *self)
{
    if (self->substate == 0) {
        set_animation(self, 7);
        exe6_sound_req(EXE6_SONG_ID(rollarrow_fire_song));
        spawn_projectile(self);
        self->timer = FIRE_HOLD_FRAMES;
        self->substate = 4;
    }
    if (timer_expired(self)) {
        self->phase = 8;
        self->substate = 0;
    }
}

static void actor_exit(Exe6Obj *self)
{
    if (self->substate == 0) {
        set_animation(self, 0);
        self->timer = POST_SHOT_FRAMES;
        self->substate = 4;
        return;
    }
    if (self->substate == 4) {
        if (timer_expired(self)) {
            set_animation(self, 4);
            self->timer = EXIT_FRAMES;
            self->substate = 8;
        }
        return;
    }
    if (timer_expired(self)) {
        self->header_flags &= (uint8_t)~EXE6_OBJ_FLAG_VISIBLE;
        self->state_word = 8;
    }
}

static void actor_update(Exe6Obj *self)
{
    switch (self->phase) {
    case 0:
        actor_wait(self);
        break;
    case 4:
        actor_fire(self);
        break;
    default:
        actor_exit(self);
        break;
    }
}

static void actor_init(Exe6Obj *self)
{
    exe6_obj_char_init(
        0x80,
        EXE6_SPRITE_GROUP(rollarrow_actor_sprite),
        EXE6_SPRITE_ID(rollarrow_actor_sprite)
    );
    exe6_obj_char_set();
    exe6_obj_shadow_set();
    self->animation_word = 0;
    exe6_obj_dma_seq_set(0);
    exe6_obj_char_set();
    exe6_block_to_pos();
    self->z = 0;
    exe6_obj_flip_set(exe6_enemy_flip_check());
    exe6_obj_clt_set(0);
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    self->state_word = 4;
    self->substate = 0;
    exe6_sound_req(EXE6_SONG_ID(common_navi_summon_song));
}

static void actor_destroy(Exe6Obj *self)
{
    uint8_t *completion = self->completion;
    if (completion != NULL) {
        *completion = 0;
    }
    exe6_obj_move_delete();
}

EXE6_EM(rollarrow_actor_main)
{
    switch (self->state) {
    case 0:
        actor_init(self);
        break;
    case 4:
        actor_update(self);
        break;
    default:
        actor_destroy(self);
        return;
    }
    exe6_battle_obj_char_move2();
}

static void projectile_free(Exe6Obj *self)
{
    Exe6Hit *hit = self->hit;
    exe6_battle_hit_off(hit);
    exe6_battle_hit_close(hit);
    exe6_obj_move_delete();
}

static bool projectile_init(Exe6Obj *self)
{
    if (exe6_battle_end_check() != 0) {
        exe6_obj_move_delete();
        return false;
    }

    exe6_obj_char_init(
        0x80,
        EXE6_SPRITE_GROUP(rollarrow_projectile_sprite),
        EXE6_SPRITE_ID(rollarrow_projectile_sprite)
    );
    exe6_obj_char_set();
    exe6_obj_no_shadow();
    self->animation_word = 0;
    exe6_obj_dma_seq_set(0);
    exe6_obj_char_set();
    exe6_obj_char_move();
    exe6_obj_flip_set(exe6_enemy_flip_check());
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;

    Exe6Hit *hit = exe6_battle_hit_open();
    if (hit == NULL) {
        exe6_obj_move_delete();
        return false;
    }
    exe6_battle_hit_data_set(
        hit,
        PROJECTILE_HIT_TYPE,
        EXE6_HIT_TYPE_STANDARD_TARGET,
        3
    );
    exe6_battle_hit_hit_mark_set(EXE6_HIT_EFFECT_CHIP_DELETE);
    exe6_battle_hit_set(0, PROJECTILE_HIT_TYPE);
    self->animation_state = PROJECTILE_BLOCKS;
    self->target_block_x = (uint8_t)(
        self->block_x + (int32_t)exe6_calc_pl_em_dir_spd_for(self)
    );
    self->state_word = 4;
    return true;
}

static bool projectile_update(Exe6Obj *self)
{
    if (exe6_battle_end_check() != 0) {
        projectile_free(self);
        return false;
    }

    Exe6Hit *hit = self->hit;
    exe6_battle_hit_check(hit);
    exe6_battle_hit_hit_mark_check();
    if (hit->received_hit_flags != 0) {
        projectile_free(self);
        return false;
    }

    int32_t direction = (int32_t)exe6_calc_pl_em_dir_spd_for(self);
    uint32_t block_x = self->target_block_x;
    uint32_t block_y = self->block_y;
    if (exe6_block_in_screen_check_sub(block_x, block_y) == 0) {
        projectile_free(self);
        return false;
    }

    int32_t target_x = (int32_t)(uint32_t)exe6_get_block_pos(block_x, block_y);
    int32_t next_x = self->x + direction * PROJECTILE_SPEED;
    self->x = next_x;
    bool entered = direction < 0 ? next_x <= target_x : next_x >= target_x;
    if (entered) {
        self->x = target_x;
        self->block_x = self->target_block_x;
        exe6_pos_to_block();
        exe6_battle_hit_block_pos_set();
        if (self->animation_state <= 1) {
            self->animation_state = 0;
            projectile_free(self);
            return false;
        }
        --self->animation_state;
        self->target_block_x = (uint8_t)(self->target_block_x + direction);
    }
    exe6_battle_hit_set(0, PROJECTILE_HIT_TYPE);
    return true;
}

EXE6_SHL(rollarrow_arrow_main)
{
    switch (self->state) {
    case 0:
        if (!projectile_init(self)) {
            return;
        }
        break;
    case 4:
        if (!projectile_update(self)) {
            return;
        }
        break;
    default:
        exe6_obj_move_delete();
        return;
    }
    exe6_battle_obj_char_move2();
}

EXE6_SUMMON_ATTACK(0x018, rollarrow_attack_main)
{
    Exe6Obj *actor = exe6_em_open(
        EXE6_OBJ_ID(rollarrow_actor_main), spawn_parameters
    );
    if (actor == NULL) {
        return;
    }
    actor->block_x = (uint8_t)block_x;
    actor->block_y = (uint8_t)block_y;
    actor->parameter = (uint8_t)parameter;
    actor->owner_word = owner->owner_word;
    actor->parent = owner;
    actor->attack = attack;
    actor->completion = completion;
    *completion = 1;
}
