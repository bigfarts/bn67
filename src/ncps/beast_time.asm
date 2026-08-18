// The native routine unconditionally resets Beast Out to three turns after
// NaviCust effects have run. Keep that three-turn base, then add the bonus
// counted by the registry-owned BeastT+1 effect for each installed piece.
.org 0x080141AC
    ldr r0,=beast_time_counter_init + 1
    bx r0
    .pool

// This nearby veneer lets the link initializer reach expanded-ROM C code
// from a Thumb BL whose direct range is too short.
beast_time_link_counter_veneer:
    ldr r0,=beast_time_link_counter_init + 1
    bx r0
    .pool

// Link setup has a separate hard-coded three-turn write into the copied
// battle stats. Replace that write with the same full-byte per-piece bonus.
.org 0x0800B1A2
    bl beast_time_link_counter_veneer
    nop

// Millions' old field-reward check reads Navi-stats byte 0x33. BeastT+1 now
// owns that entire byte as its piece count, so permanently disable the stale
// boolean consumer instead of treating a one-piece bonus as Millions.
.org 0x0809FCAA
    mov r5,0
    nop
    nop
