// Replace the native one-chip response for hit property 0x10. The shared
// handler preserves the normal enemy cleanup and discards a player's full
// loaded hand.
.org 0x0801A2CC
    ldr r0,=chip_delete_entire_hand_main + 1
    bx r0
    .pool

// The native hidden-block renderer copies an empty tilemap template.  That
// leaves stale panel graphics in player 2's staged BG map.  Use BN6's adjacent
// rectangle-fill primitive for the same body and edge rectangles instead.
.org 0x0800C02A
b hidden_block_state_transform

.org 0x0800C050
beq hidden_block_body_fill

.org 0x0800C08C
hidden_block_body_fill:
    ldr r0,[sp,0x0]
    ldr r1,[sp,0x4]
    mov r2,2
    mov r3,0
    mov r4,5
    mov r5,3
    bl 0x080018D0
    add sp,0x28
    pop {r4-r7,pc}

.org 0x0800C0A0
hidden_block_state_transform:
    cmp r2,0xFF
    beq 0x0800C038
    cmp r2,0x0B
    blt 0x0800C038
    b 0x0800C02E

.org 0x0800C15E
mov r3,0

.org 0x0800C164
bl 0x080018D0
