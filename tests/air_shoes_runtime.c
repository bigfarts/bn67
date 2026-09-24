/* Run against an assembled ROM and its Armips symbols; see RUNTIME_QA.md. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mgba/core/core.h>
#include <mgba/internal/arm/arm.h>
#include <mgba/internal/arm/isa-inlines.h>

#define PLAYER 0x02010000u
#define PLAYER_RUNTIME 0x02011000u
#define HIT 0x02012000u
#define CONTROLLER 0x02013000u
#define VISUAL 0x02014000u
#define GLOBAL 0x02015000u
#define BATTLE 0x02016000u
#define STATUS 0x0203CE00u
#define STACK 0x03007000u
#define RETURN 0x03007800u

static uint32_t controller_main, effect_open, sound_req;
static uint32_t allocation;

static uint32_t symbol(const char *path, const char *name) {
    FILE *file = fopen(path, "r");
    assert(file);
    uint32_t address;
    char line[512], label[256];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%x %255s", &address, label) == 2 &&
            strcmp(label, name) == 0) {
            fclose(file);
            return address & ~1u;
        }
    }
    fprintf(stderr, "Missing symbol: %s\n", name);
    exit(1);
}

static void enter(struct ARMCore *cpu, uint32_t address) {
    _ARMSetMode(cpu, MODE_THUMB);
    cpu->gprs[15] = address & ~1u;
    ThumbWritePC(cpu);
}

static void run(struct mCore *core, uint32_t entry, uint32_t self) {
    struct ARMCore *cpu = core->cpu;
    cpu->gprs[5] = self;
    cpu->gprs[10] = GLOBAL;
    cpu->gprs[13] = STACK;
    cpu->gprs[14] = RETURN | 1u;
    enter(cpu, entry);
    for (int i = 0; i < 10000; ++i) {
        uint32_t pc = cpu->gprs[15] - 2;
        if (pc == RETURN) {
            assert(cpu->gprs[13] == STACK);
            assert((uint32_t)cpu->gprs[5] == self);
            return;
        }
        /* Isolate presentation/allocation. Status writes, hit flags, and
         * Uninstall execute the actual native engine routines in the ROM. */
        if (pc == effect_open || pc == sound_req) {
            if (pc == effect_open) cpu->gprs[0] = allocation;
            enter(cpu, cpu->gprs[14]);
        } else {
            core->step(core);
        }
    }
    fprintf(stderr, "Routine %08x did not return: pc=%08x\n",
            entry, cpu->gprs[15]);
    exit(1);
}

static void activate(struct mCore *core, int side, int had_shoes,
                     int visual_available) {
    core->rawWrite32(core, CONTROLLER + 8, -1, 0x00000804);
    allocation = visual_available ? VISUAL : 0;
    run(core, controller_main, CONTROLLER);
    if (visual_available) {
        assert(core->rawRead32(core, VISUAL + 0x4C, -1) == PLAYER);
        assert(core->rawRead8(core, VISUAL, -1) & 0x10);
    }
    for (int frame = 1; frame <= 40; ++frame) {
        run(core, controller_main, CONTROLLER);
        uint32_t active = had_shoes || frame >= 10;
        assert(core->rawRead8(core, STATUS + side * 0x64 + 0x1C, -1) == active);
        assert(core->rawRead8(core, STATUS + side * 0x64 + 0x1B, -1) == 0);
        assert(core->rawRead32(core, HIT + 0x3C, -1) ==
               (0x40020u | (active ? 0x10u : 0)));
        assert(core->rawRead8(core, STATUS + (side ^ 1) * 0x64 + 0x1C, -1) == 0);
        assert(core->rawRead8(core, CONTROLLER + 9, -1) == (frame < 40 ? 8 : 12));
    }
}

int main(int argc, char **argv) {
    assert(argc == 3);
    controller_main = symbol(argv[2], "air_shoes_controller_main");
    effect_open = symbol(argv[2], "exe6_efc_open");
    sound_req = symbol(argv[2], "exe6_sound_req");
    uint32_t uninstall = symbol(argv[2], "exe6_navi_uninstall");
    struct mCore *core = mCoreFind(argv[1]);
    assert(core && core->init(core));
    mCoreConfigInit(&core->config, "bn67-air-shoes-probe");
    assert(mCoreLoadFile(core, argv[1]));
    const uint32_t record = 0x08021DA8 + 4 * 0x2C;
    assert(core->rawRead8(core, record, -1) == 0x1A); /* Wildcard. */
    assert(core->rawRead8(core, record + 6, -1) == 0x0A); /* Null. */
    assert(core->rawRead8(core, record + 7, -1) == 0); /* Standard. */
    assert(core->rawRead8(core, record + 8, -1) == 26);
    assert(core->rawRead8(core, record + 9, -1) & 1); /* Dimming/cut-in. */
    assert(core->rawRead8(core, record + 0x0B, -1) == 0x15);
    assert(core->rawRead16(core, record + 0x1A, -1) == 0);

    for (int side = 0; side < 2; ++side) {
        for (int had_shoes = 0; had_shoes < 2; ++had_shoes) {
            for (int visual_available = 0; visual_available < 2; ++visual_available) {
                core->reset(core);
                for (uint32_t addr = PLAYER; addr < BATTLE + 0x100; addr += 4)
                    core->rawWrite32(core, addr, -1, 0);
                for (uint32_t addr = STATUS; addr < STATUS + 0xC8; addr += 4)
                    core->rawWrite32(core, addr, -1, 0);
                core->rawWrite32(core, GLOBAL + 0x18, -1, BATTLE);
                core->rawWrite32(core, BATTLE + 0x3C, -1, BATTLE + 0x80);
                core->rawWrite32(core, PLAYER + 0x54, -1, HIT);
                core->rawWrite32(core, PLAYER + 0x58, -1, PLAYER_RUNTIME);
                core->rawWrite8(core, PLAYER + 0x16, -1, side);
                core->rawWrite32(core, CONTROLLER + 0x4C, -1, PLAYER);
                core->rawWrite32(core, HIT + 0x3C, -1,
                                 0x40020u | (had_shoes ? 0x10u : 0));
                core->rawWrite8(core, STATUS + side * 0x64 + 0x1C, -1, had_shoes);
                activate(core, side, had_shoes, visual_available);
                /* Reusing the chip must retain the ability and other flags. */
                activate(core, side, 1, visual_available);
                ((struct ARMCore *)core->cpu)->gprs[0] = PLAYER;
                run(core, uninstall, PLAYER);
                assert(core->rawRead8(core, STATUS + side * 0x64 + 0x1C, -1) == 0);
                assert((core->rawRead32(core, HIT + 0x3C, -1) & 0x10) == 0);
            }
        }
    }
    mCoreConfigDeinit(&core->config);
    core->deinit(core);
    puts("AirShoes: timing, both sides, repeat use, allocation failure, and Uninstall passed");
    return 0;
}
