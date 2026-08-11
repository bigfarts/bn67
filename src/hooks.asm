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
