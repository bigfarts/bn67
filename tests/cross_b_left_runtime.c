/* Execute the assembled input path in mGBA; see RUNTIME_QA.md. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <mgba/core/core.h>
#include <mgba/internal/arm/arm.h>
#include <mgba/internal/arm/isa-inlines.h>

#define PLAYER 0x02010000
#define RUNTIME 0x02011000
#define STATUS 0x0203CE00
#define STACK 0x03007000

static void enter(struct mCore *core, uint32_t address) {
    struct ARMCore *cpu = core->cpu;
    _ARMSetMode(cpu, MODE_THUMB);
    cpu->gprs[15] = address;
    ThumbWritePC(cpu);
}

static void setup(struct mCore *core, int form, int side, int rapid) {
    core->reset(core);
    for (int i = 0; i < 0x100; ++i) {
        core->rawWrite8(core, PLAYER+i, -1, 0);
        core->rawWrite8(core, RUNTIME+i, -1, 0);
    }
    core->rawWrite32(core, PLAYER+0x58, -1, RUNTIME);
    core->rawWrite8(core, PLAYER+0x16, -1, side);
    core->rawWrite8(core, PLAYER+0x17, -1, side);
    core->rawWrite8(core, STATUS+side*0x64+0x2c, -1, form);
    core->rawWrite8(core, RUNTIME+6, -1, rapid ? 3 : 1);
    core->rawWrite8(core, RUNTIME+7, -1, 1);
    core->rawWrite8(core, RUNTIME+8, -1, 0xff);
}

static uint32_t tick(struct mCore *core, uint16_t held, uint16_t pressed, uint16_t released) {
    core->rawWrite16(core, RUNTIME+0x22, -1, held);
    core->rawWrite16(core, RUNTIME+0x24, -1, pressed);
    core->rawWrite16(core, RUNTIME+0x26, -1, released);
    struct ARMCore *cpu = core->cpu;
    cpu->gprs[4] = RUNTIME;
    cpu->gprs[5] = PLAYER;
    cpu->gprs[6] = core->rawRead32(core, RUNTIME+0x44, -1);
    cpu->gprs[7] = held;
    cpu->gprs[13] = STACK;
    enter(core, 0x08013130);
    for (int i = 0; i < 1000; ++i) {
        if (cpu->gprs[15]-2 == 0x080131d8) {
            assert(cpu->gprs[4] == RUNTIME && cpu->gprs[5] == PLAYER);
            assert(cpu->gprs[13] == STACK);
            return core->rawRead32(core, RUNTIME+0x44, -1);
        }
        core->step(core);
    }
    fprintf(stderr, "input path did not return: pc=%08x\n", cpu->gprs[15]);
    exit(2);
}

int main(int argc, char **argv) {
    assert(argc == 2);
    struct mCore *core = mCoreFind(argv[1]);
    assert(core && core->init(core));
    mCoreConfigInit(&core->config, "bn67-cross-probe");
    assert(mCoreLoadFile(core, argv[1]));
    int failures = 0;
    for (int form = 13; form <= 15; form += 2) {
        for (int side = 0; side < 2; ++side) {
            uint16_t back = side ? 0x10 : 0x20;
            for (int delay = 0; delay < 8; ++delay) {
                setup(core, form, side, 1);
                for (int frame = 0; frame <= delay; ++frame) {
                    uint32_t flags = tick(core, 2 | (frame == delay ? back : 0),
                        (frame == 0 ? 2 : 0) | (frame == delay ? back : 0), 0);
                    uint32_t expected = frame == delay ? 0x10 : 0;
                    if (flags != expected) {
                        fprintf(stderr, "form=%d side=%d delay=%d frame=%d flags=%x expected=%x\n",
                            form, side, delay, frame, flags, expected);
                        ++failures;
                        break;
                    }
                }
            }
            setup(core, form, side, 1);
            uint32_t flags = 0;
            for (int frame = 0; frame < 8; ++frame)
                flags = tick(core, 2, frame == 0 ? 2 : 0, 0);
            assert(flags == 1); // Holding B alone still starts rapid fire.
        }
    }
    if (failures) {
        mCoreConfigDeinit(&core->config);
        core->deinit(core);
        return 1;
    }
    // Quick taps must still fire on release during the command window.
    for (int form = 13; form <= 15; form += 2) {
        for (int duration = 1; duration < 8; ++duration) {
            setup(core, form, 0, 1);
            for (int frame = 0; frame < duration; ++frame)
                assert(tick(core, 2, frame == 0 ? 2 : 0, 0) == 0);
            assert(tick(core, 0, 0, 2) == 1);
            assert(core->rawRead8(core, RUNTIME+0x13, -1) == 0);
        }
        // Once started, holding B continues to queue rapid shots immediately.
        setup(core, form, 0, 1);
        for (int frame = 0; frame < 8; ++frame)
            tick(core, 2, frame == 0 ? 2 : 0, 0);
        for (int frame = 0; frame < 10; ++frame) {
            core->rawWrite32(core, RUNTIME+0x44, -1, 0);
            assert(tick(core, 2, 0, 0) == 1);
        }
        // The existing B-left cooldown still blocks the ability.
        setup(core, form, 0, 1);
        core->rawWrite8(core, RUNTIME+0x15, -1, 30);
        assert(tick(core, 0x22, 0x22, 0) == 1);
    }
    // Regular Crosses still recognize the same eight-frame command window.
    for (int form = 1; form <= 3; form += 2) {
        for (int delay = 0; delay < 8; ++delay) {
            setup(core, form, 0, 0);
            for (int frame = 0; frame <= delay; ++frame) {
                uint32_t flags = tick(core, 2 | (frame == delay ? 0x20 : 0),
                    (frame == 0 ? 2 : 0) | (frame == delay ? 0x20 : 0), 0);
                assert(flags == (frame == delay ? 0x10u : 0u));
            }
        }
    }
    // Other forms retain native rapid fire, including Tengu/Dust's exception.
    for (int form = 0; form <= 24; ++form) {
        if (form == 13 || form == 15) continue;
        setup(core, form, 0, 1);
        assert(tick(core, 2, 2, 0) == 1);
        if (form == 20 || form == 22) {
            setup(core, form, 0, 1);
            core->rawWrite8(core, RUNTIME+8, -1, 1);
            assert(tick(core, 0x22, 0x22, 0) == 0x10);
        }
    }
    mCoreConfigDeinit(&core->config);
    core->deinit(core);
    printf("Cross Beast input: %d failing cases\n", failures);
    return failures ? 1 : 0;
}
