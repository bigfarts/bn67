// The native routine unconditionally resets Beast Out to three turns after
// NaviCust effects have run. Keep that three-turn base, then add the bonus
// counted by the registry-owned BeastT+1 effect for each installed piece.
.org 0x080141AC
    ldr r0,=beast_time_counter_init + 1
    bx r0
    .pool

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
