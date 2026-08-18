#include "abi.h"
#include "runtime.h"

BN67_INCBIN(folder_back_icon, "build/folder_back_icon.bin");
BN67_INCBIN(folder_back_image, "build/folder_back_image.bin");
BN67_INCBIN(folder_back_palette, "build/folder_back_palette.bin");
BN67_SONG(folder_back_rumble_song,
          BN67_PCM(folder_back_rumble, 0x40, 0x08,
                   ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBE,0x7F\n"
                   ".byte 0xBF,0x40,0xF9,0x3C,0x7F,0xAA,0x81,0xB1\n",
                   "build/folder_back_rumble_sample.bin"));

BN67_CHIP_RECORD(0x0c2){
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
    .chip_class = EXE6_CHIP_CLASS_STANDARD,
    .mb = 99,
    .behavior =
        {
            .effect_flags = EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
            .counter_settings = 0x00,
            .family = BN67_ATTACK_FAMILY(folder_back_attack_main),
            .subfamily = BN67_ATTACK_SUBFAMILY(folder_back_attack_main),
            .dark_soul_usage = 0x0A,
            .unknown_0e = 0x00,
            .lock_on = 0x00,
            .object_spawn = {0},
            .delay = 0,
        },
    /* Preserve ColorPoint's Standard-library position and ordering. */
    .library_number = 0xC7,
    .library_flags = 0x00,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 0,
    .library_sort_order = 0x00C7,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = folder_back_icon,
    .image = folder_back_image,
    .palette = folder_back_palette,
};

static const uint32_t FULL_GAUGE = 0x4000;
static const uint32_t PALE_FLASH = 0x00004210;
static const uint32_t WHITE_FLASH = 0x00006318;
static const uint16_t IMPACT_FRAMES = 0x46;
static const uint16_t RESTORED_FRAMES = 0x14;

enum EffectStep {
  EFFECT_STEP_IMPACT,
  EFFECT_STEP_FLASH = 4,
  EFFECT_STEP_RESTORE = 8,
  EFFECT_STEP_WAIT = 12,
};

struct ControllerWork {
  uint16_t held_custom_gauge;
  bool folder_restored;
};

_Static_assert(sizeof(struct ControllerWork) <=
                   sizeof(((Exe6Obj *)0)->work),
               "FolderBack controller work exceeds object work storage");

static USED __attribute__((noinline)) bool
folder_back_object_should_pause(const Exe6Obj *object) {
  const enum Exe6ObjectClass object_class =
      (enum Exe6ObjectClass)(object->object_class & 0x0Fu);
  const bool is_enemy = object_class == EXE6_OBJECT_CLASS_ENEMY;
  const bool is_shell = object_class == EXE6_OBJECT_CLASS_SHELL;
  if (!is_enemy && !is_shell) {
    return false;
  }

  const Exe6ObjectSlot *slots = EXE6_EFFECT_POOL_HEAD;
  const uint8_t controller_id =
      (uint8_t)BN67_OBJ_ID(folder_back_controller_main);
  for (size_t slot_index = 0; slot_index < EXE6_POOL_SLOT_COUNT;
       ++slot_index) {
    const Exe6Obj *controller = &slots[slot_index].object;
    if ((controller->header_flags & EXE6_OBJ_FLAG_ACTIVE) == 0 ||
        controller->object_id != controller_id) {
      continue;
    }

    return true;
  }
  return false;
}

// The class-1 dispatch table is relocated by the registry compiler, leaving
// its original first two entries available for the section-patch relay.
BN67_PATCH_SECTION(0x080031FA, 0x08003C9C, folder_back_dispatch_main);

/* The original class-4 table is dead after registry relocation, so its first
 * two entries can relay the native Custom-opening path to the linked code. */
BN67_PATCH_SECTION(0x0800819A, 0x080E42C8,
                   folder_back_custom_transition_dispatch);

NAKED void folder_back_dispatch_main(void) {
  // Native object mains consume the dispatcher's live registers and flags.
  // Preserve them around the C predicate, then reproduce the final flag-setting
  // shift from the native table lookup. Rejoin at the native pop/call sequence
  // so paths which branch directly to it remain untouched.
  __asm__(".syntax unified\n"
          "pop {r1}\n"
          "push {r7}\n"
          "push {r0,r1,r2,r3,r4,r6,r7}\n"
          "adds r0,r5,#0\n"
          "bl folder_back_object_should_pause\n"
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
          "push {r0}\n"
          "ldr r0,=0x08003200 + 1\n"
          "mov lr,r0\n"
          "pop {r0}\n"
          "mov pc,lr\n");
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
  exe6_sound_req(BN67_SONG_ID(folder_back_rumble_song));
}

static void hold_local_custom_gauge(Exe6Obj *self) {
  struct ControllerWork *work = (struct ControllerWork *)self->work;
  exe6_cockpit_set_custom_gauge_value(work->held_custom_gauge);
}

static void fill_local_custom_gauge(Exe6Obj *self) {
  struct ControllerWork *work = (struct ControllerWork *)self->work;
  work->held_custom_gauge = FULL_GAUGE;
  exe6_cockpit_set_custom_gauge_value(FULL_GAUGE);
  exe6_sound_req(0x8F);
}

static void clear_loaded_hand(uint32_t owner) {
  Exe6NaviSelectChipWork *selection =
      exe6_navi_select_chip_work_adrs_get(owner);
  selection->active_chip_index = 0;
  selection->loaded_chip_count = 0;
  for (size_t index = 0;
       index < sizeof(selection->chip_ids) / sizeof(selection->chip_ids[0]);
       ++index) {
    selection->chip_ids[index] = UINT16_MAX;
  }
}

static void clear_chip_use_counts(void) {
  *EXE6_USED_CHIP_CLASS_COUNTS = (Exe6ChipClassUseCounts){0};
}

static void restore_local_folder(Exe6Obj *self) {
  if (exe6_battle_one_self_check(self->owner) != 0) {
    return;
  }

  Exe6BattleContext *context = exe6_runtime()->battle_context;

  uint8_t regular_chip_available = context->regular_chip_available;
  uint8_t tag_chips_available = context->tag_chips_available;

  exe6_battle_chip_set();

  /* BN6 rejects a Mega or Giga chip after the number selected this battle
   * exceeds the Navi's class limit.  FolderBack restores the complete Folder,
   * so all of its owner's accumulated class totals start over with it. */
  clear_chip_use_counts();

  context->regular_chip_available = regular_chip_available;
  context->tag_chips_available = tag_chips_available;

  /* The native initializer clears both players, but only this core restores
   * the local user's Folder.  Clearing the opponent here loses their retained
   * chips on one peer and desynchronizes the next time they fire one. */
  clear_loaded_hand(self->owner);
}

static void restore_folder_once(Exe6Obj *self) {
  struct ControllerWork *work = (struct ControllerWork *)self->work;
  if (work->folder_restored) {
    return;
  }
  work->folder_restored = true;
  fill_local_custom_gauge(self);
  restore_local_folder(self);
}

static void delete_controller(Exe6Obj *self) {
  // FolderBack bypasses the common event-chip exit while opening Custom.
  // Clear the persistent attack's handshake before removing its controller.
  exe6_event_chip_state_reset(self->owner);
  exe6_obj_invoke(self, (uintptr_t)exe6_obj_move_delete);
}

static USED __attribute__((noinline)) void
folder_back_coalesce_native_custom(void) {
  Exe6ObjectSlot *slots = EXE6_EFFECT_POOL_HEAD;
  const uint8_t controller_id =
      (uint8_t)BN67_OBJ_ID(folder_back_controller_main);
  for (size_t slot_index = 0; slot_index < EXE6_POOL_SLOT_COUNT;
       ++slot_index) {
    Exe6Obj *controller = &slots[slot_index].object;
    if ((controller->header_flags & EXE6_OBJ_FLAG_ACTIVE) == 0 ||
        controller->object_id != controller_id) {
      continue;
    }

    /* Beast Over can invoke the native Custom transition while FolderBack's
     * effect is still running. Finish FolderBack and consume that same Custom
     * transition so its suspended controller cannot request Custom again. */
    restore_palette();
    restore_folder_once(controller);
    delete_controller(controller);
  }
  exe6_battle_pause_on();
}

NAKED void folder_back_custom_transition_dispatch(void) {
  __asm__(".syntax unified\n"
          "pop {r1}\n"
          "push {r4-r7,lr}\n"
          "bl folder_back_coalesce_native_custom\n"
          "movs r0,#0x20\n"
          "pop {r4-r7,pc}\n");
}

static bool effect_update(Exe6Obj *self) {
  switch (self->phase_timer_low) {
  case EFFECT_STEP_IMPACT:
    self->timer = IMPACT_FRAMES;
    self->phase_timer_low = EFFECT_STEP_FLASH;
    impact();
    apply_white_flash(self);
    break;
  case EFFECT_STEP_FLASH:
    if (--self->timer == 0) {
      restore_palette();
      self->phase_timer_low = EFFECT_STEP_RESTORE;
    } else {
      apply_white_flash(self);
    }
    break;
  case EFFECT_STEP_RESTORE:
    self->timer = RESTORED_FRAMES;
    self->phase_timer_low = EFFECT_STEP_WAIT;
    restore_folder_once(self);
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

BN67_EFFECT(folder_back_controller_main) {
  struct ControllerWork *work = (struct ControllerWork *)self->work;
  if (self->state == EXE6_OBJECT_STATE_INIT) {
    /* Link battles advance the displayed cockpit gauge directly.  Their
     * operation-work gauge can remain zero even while Custom is charged. */
    work->held_custom_gauge =
        (uint16_t)exe6_cockpit_get_custom_gauge_value();
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
  } else if (self->state != EXE6_OBJECT_STATE_ACTIVE) {
    delete_controller(self);
    return;
  }

  // Restore the displayed snapshot every controller frame so the per-frame
  // gauge update cannot accumulate. FullCust replaces it below.
  hold_local_custom_gauge(self);

  if (exe6_battle_end_check() != 0) {
    restore_palette();
    delete_controller(self);
    return;
  }
  if (!effect_update(self)) {
    return;
  }

  uint32_t owner = self->owner;
  delete_controller(self);
  open_custom(owner);
}

BN67_PERSISTENT_ATTACK(0x0c2, folder_back_attack_main) {
  Exe6Obj *controller =
      exe6_efc_open(BN67_OBJ_ID(folder_back_controller_main), spawn_parameters);
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
  ((struct ControllerWork *)controller->work)->folder_restored = false;
  return controller;
}
