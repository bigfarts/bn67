// Replace the native one-chip response for hit property 0x10. The shared
// handler preserves the normal enemy cleanup and discards a player's full
// loaded hand.
.org 0x0801A2CC
    ldr r0,=chip_delete_entire_hand_main + 1
    bx r0
    .pool
