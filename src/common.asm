.gba
.thumb

.definelabel object_free,              0x08003458

.macro engine_call,target
    push {r4}
    ldr r4,=target + 1
    mov r12,r4
    pop {r4}
    mov lr,pc
    bx r12
.endmacro

// Replace the native one-chip response for hit property 0x10. The shared
// handler preserves the normal enemy cleanup and discards a player's full
// loaded hand.
.org 0x0801A2CC
    ldr r0,=chip_delete_entire_hand_main + 1
    bx r0
    .pool

.if falzar
    .include "build/c-symbols-falzar.generated.asm"
    .org c_code_start
    .incbin "build/gameplay-falzar.bin"
.else
    .include "build/c-symbols-gregar.generated.asm"
    .org c_code_start
    .incbin "build/gameplay-gregar.bin"
.endif

.defineregion c_code_end,0x09000000-c_code_end,0xFF
.include "src/title_screen.asm"
.if falzar
    .include "build/registry-falzar.generated.asm"
.else
    .include "build/registry-gregar.generated.asm"
.endif
