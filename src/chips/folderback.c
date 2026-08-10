#include "runtime.h"

EXE6_INCBIN(folderback_icon, "build/folderback-icon.bin");
EXE6_INCBIN(folderback_image, "build/folderback-image.bin");
EXE6_INCBIN(folderback_palette, "build/folderback-palette.bin");
EXE6_SONG(folderback_rumble_song,
          EXE6_PCM(folderback_rumble, 0x40, 0x08,
                   ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBE,0x7F\n"
                   ".byte 0xBF,0x40,0xF9,0x3C,0x7F,0xAA,0x81,0xB1\n",
                   "build/folderback-rumble-sample.bin"));

EXE6_CHIP_RECORD(0x139){
    .codes =
        {
            EXE6_CHIP_CODE_ASTERISK,
            EXE6_CHIP_CODE_NONE,
            EXE6_CHIP_CODE_NONE,
            EXE6_CHIP_CODE_NONE,
        },
    .attack_element = 0,
    .rarity = 4,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_GIGA,
    .mb = 99,
    .behavior =
        {
            .effect_flags = 0x41,
            .counter_settings = 0x00,
            .family = EXE6_ATTACK_FAMILY(folderback_attack_main),
            .subfamily = EXE6_ATTACK_SUBFAMILY(folderback_attack_main),
            .dark_soul_usage = 0x0A,
            .unknown_0e = 0x00,
            .lock_on = 0x00,
            .object_spawn = {0},
            .delay = 0,
        },
    .library_number = 0x0D,
    .library_flags = 0x10,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 0,
    .library_sort_order = 0x0139,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = folderback_icon,
    .image = folderback_image,
    .palette = folderback_palette,
};

static const uint32_t FULL_GAUGE = 0x4000;
static const uint32_t PALE_FLASH = 0x00004210;
static const uint32_t WHITE_FLASH = 0x00006318;
static const uint16_t IMPACT_FRAMES = 0x46;
static const uint16_t RESTORED_FRAMES = 0x14;

struct Exe6FolderbackWork {
  Exe6Obj *locked_opponents[4]; // +0x60
  uint32_t reserved[3];         // +0x70
};

_Static_assert(sizeof(struct Exe6FolderbackWork) == 0x1C,
               "FolderBack work layout");

static USED __attribute__((noinline)) bool
folderback_opponent_is_locked(const Exe6Obj *opponent) {
  if ((opponent->object_class & 0x0Fu) != EXE6_OBJECT_CLASS_ENEMY) {
    return false;
  }

  const Exe6ObjectSlot *slots = EXE6_EFFECT_POOL_HEAD;
  const uint8_t controller_id =
      (uint8_t)EXE6_OBJ_ID(folderback_controller_main);
  for (size_t slot_index = 0; slot_index < EXE6_POOL_SLOT_COUNT;
       ++slot_index) {
    const Exe6Obj *object = &slots[slot_index].object;
    if ((object->header_flags & EXE6_OBJ_FLAG_ACTIVE) == 0 ||
        object->object_id != controller_id) {
      continue;
    }

    const struct Exe6FolderbackWork *work =
        (const struct Exe6FolderbackWork *)(const void *)object->work;
    for (size_t opponent_index = 0;
         opponent_index < sizeof(work->locked_opponents) / sizeof(work->locked_opponents[0]);
         ++opponent_index) {
      if (work->locked_opponents[opponent_index] == opponent) {
        return true;
      }
    }
  }
  return false;
}

EXE6_PATCH_SECTION(0x080031FA, folderback_type_1_main);

NAKED void folderback_type_1_main(void) {
  // Native object mains consume the dispatcher's live registers and flags.
  // Preserve them around the C predicate, then reproduce the final flag-setting
  // shift from the native table lookup. The native advance routine also returns
  // its object-list count in r0, so preserve it across the long jump back.
  __asm__(".syntax unified\n"
          "pop {r1}\n"
          "push {r7}\n"
          "push {r0,r1,r2,r3,r4,r6,r7}\n"
          "adds r0,r5,#0\n"
          "bl folderback_opponent_is_locked\n"
          "cmp r0,#0\n"
          "bne 1f\n"
          "pop {r0,r1,r2,r3,r4,r6,r7}\n"
          "lsrs r1,r1,#1\n"
          "lsls r1,r1,#1\n"
          "mov lr,pc\n"
          "bx r0\n"
          "b 2f\n"
          "1:\n"
          "pop {r0,r1,r2,r3,r4,r6,r7}\n"
          "lsrs r1,r1,#1\n"
          "lsls r1,r1,#1\n"
          "bl exe6_battle_obj_char_move2\n"
          "2:\n"
          "pop {r7}\n"
          "ldr r0,=0x0800372A + 1\n"
          "mov lr,pc\n"
          "bx r0\n"
          "push {r0}\n"
          "ldr r0,=0x08003206 + 1\n"
          "mov lr,r0\n"
          "pop {r0}\n"
          "mov pc,lr\n");
}

static void lock_opponent(Exe6Obj *self) {
  struct Exe6FolderbackWork *work = (struct Exe6FolderbackWork *)self->work;
  Exe6Obj *const *units =
      exe6_runtime()->battle_context->active_units[self->owner ^ 1u];
  for (size_t index = 0; index < 4; ++index) {
    work->locked_opponents[index] = units[index];
  }
}

static void unlock_opponent(Exe6Obj *self) {
  struct Exe6FolderbackWork *work = (struct Exe6FolderbackWork *)self->work;
  for (size_t index = 0; index < 4; ++index) {
    work->locked_opponents[index] = NULL;
  }
}

static void apply_white_flash(Exe6Obj *self) {
  uint32_t color = (self->timer & 4u) != 0 ? PALE_FLASH : WHITE_FLASH;
  exe6_col_fade_set(0, color, 0x0F, 0x14, EXE6_PALETTE_OBJ_OUTPUT_00);
  exe6_col_fade_set(0, color, 0x0F, 0x15, EXE6_PALETTE_BG_OUTPUT_00);
}

static void restore_palette(void) {
  exe6_col_fade_kill(0x14);
  exe6_col_fade_kill(0x15);
}

static void impact(void) {
  exe6_camera_quake_set(3, IMPACT_FRAMES);
  exe6_sound_req(EXE6_SONG_ID(folderback_rumble_song));
}

static void fill_local_custom_gauge(void) {
  Exe6BattleContext *context = exe6_runtime()->battle_context;
  uint8_t *player = exe6_op_work_adrs_get(context->local_side);
  *(uint16_t *)(player + 0x28) = FULL_GAUGE;
  exe6_cockpit_set_custom_gauge_value(FULL_GAUGE);
  exe6_sound_req(0x8F);
}

static void restore_local_folder(Exe6Obj *self) {
  if (exe6_battle_one_self_check(self->owner) != 0) {
    return;
  }

  Exe6BattleContext *context = exe6_runtime()->battle_context;

  // Perform native shuffle.
  exe6_battle_chip_set();

  // Perform secondary shuffle to get rid of reg chip/tag chips.
  context->regular_chip_available = 0;
  context->tag_chips_available = 0;
  exe6_deck_shuffle_sub(EXE6_CHIP_QUEUE, 0, 0);

  exe6_battle_select_chip_work_init();
}

static bool effect_update(Exe6Obj *self) {
  switch (self->phase_timer_low) {
  case 0:
    self->timer = IMPACT_FRAMES;
    self->phase_timer_low = 4;
    impact();
    apply_white_flash(self);
    break;
  case 4:
    if (--self->timer == 0) {
      restore_palette();
      self->phase_timer_low = 8;
    } else {
      apply_white_flash(self);
    }
    break;
  case 8:
    self->timer = RESTORED_FRAMES;
    self->phase_timer_low = 0x0C;
    fill_local_custom_gauge();
    restore_local_folder(self);
    break;
  default:
    if (--self->timer == 0) {
      return true;
    }
    break;
  }
  return false;
}

static void open_custom(uint32_t owner) {
  Exe6BattleState *battle = EXE6_BATTLE_STATE;
  uint32_t state = battle->state;
  if (state == 4) {
    battle->custom_screen_side = (uint8_t)owner;
    exe6_battle_pause_on();
    battle->state = 0x18;
    exe6_cockpit_pause_set();
  } else if (state == 8) {
    exe6_battle_pause_on();
    battle->state = 0x20;
  }
}

EXE6_EFFECT(folderback_controller_main) {
  if (self->state == 0) {
    self->state_word = 4;
    lock_opponent(self);
  } else if (self->state != 4) {
    exe6_obj_move_delete();
    return;
  }

  if (exe6_battle_end_check() != 0) {
    restore_palette();
    unlock_opponent(self);
    exe6_obj_move_delete();
    return;
  }
  if (!effect_update(self)) {
    return;
  }

  uint32_t owner = self->owner;
  unlock_opponent(self);
  exe6_obj_move_delete();
  open_custom(owner);
}

EXE6_PERSISTENT_ATTACK(0x139, folderback_attack_main) {
  Exe6Obj *controller =
      exe6_efc_open(EXE6_OBJ_ID(folderback_controller_main), spawn_parameters);
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
  struct Exe6FolderbackWork *work =
      (struct Exe6FolderbackWork *)controller->work;
  for (size_t index = 0; index < 4; ++index) {
    work->locked_opponents[index] = NULL;
  }
  for (size_t index = 0; index < 3; ++index) {
    work->reserved[index] = 0;
  }
  return controller;
}
