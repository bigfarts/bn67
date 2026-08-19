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

// Millions sets r5 when its NaviCust stat byte is active, then uses that flag
// to grant its field reward. BeastT+1 owns the byte now, so force the native
// false path by removing the branch that skips `mov r5,0`. The Falzar routine
// is shifted in Gregar; using Falzar's address in both editions corrupts
// Gregar's ACDC Town field-object initializer instead.
.if falzar
    .org 0x0809FCAC
.else
    .org 0x080A118C
.endif
    nop
