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

// The Custom Screen collapses chip IDs 0xC0 and 0xC1 into a preceding Attack
// or Navi chip because those slots were originally Attack+10 and Navi+20.
// Rook and SignalRed now occupy those IDs, so disable only the two consume
// branches while leaving the other native add-on chips unchanged.
.org 0x08029238
    nop

.org 0x0802923C
    nop

// Status Bug dispatches one of four native handlers for each severity. The
// second normal/high entries inflict green invulnerability; redirect them to
// the first entries, which apply the matching 300/600-frame flashing status.
// Keep the native selector at 0x08013E7E intact: rewriting its RNG path can
// hang at battle startup.
.org 0x08013EA4
    .dw 0x08013EC3

.org 0x08013EB4
    .dw 0x08013EF1
