// Replace the native one-chip response for hit property 0x10. The shared
// handler preserves the normal enemy cleanup and discards a player's full
// loaded hand.
.org 0x0801A2CC
    ldr r0,=chip_delete_entire_hand_main + 1
    bx r0
    .pool

// BN6 mirrors conveyor terrain IDs for player 2 before checking the 0xFF
// hidden-panel sentinel.  Exempt the sentinel and swap the only two conveyor
// states in place; 0x0B ^ 7 == 0x0C and 0x0C ^ 7 == 0x0B.
.org 0x0800C026
    cmp r4,0
    beq 0x0800C038
    cmp r2,0x0B
    blt 0x0800C038
    cmp r2,0x0C
    bgt 0x0800C038
    mov r6,7
    eor r2,r6
    nop

// AquaNeedle normally requests stagger and fixed invulnerability (0x03).
// Keep stagger (0x01), but do not make its target flash after being hit.
.if falzar
    .org 0x080CE5BE
.else
    .org 0x080CFE2E
.endif
    mov r3,0x01

// Status Bug dispatches one of four native handlers for each severity. Replace
// only the normal- and high-severity green-invulnerability entries with their
// corresponding flashing handlers. Keep the native selector at 0x08013E7E
// intact: rewriting its RNG path can hang while applying battle-start bugs.
.org 0x08013EA8
    .dw 0x08013EE7

.org 0x08013EB8
    .dw 0x08013F15
