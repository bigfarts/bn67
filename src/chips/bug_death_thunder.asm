// Native power-attack ID 34 is BugDeathThunder's DarkThunder ball. This table
// stores one charge-time halfword for each NaviCust Charge level (1 through 5).
.org 0x08020558
    .dh 60,60,60,60,60

// After the projectile firing phase, the action handler seeds a 30-frame
// recovery timer at work offset +0x10. Starting it at zero makes the existing
// decrement-and-cleanup path finish the action immediately on that same update.
.if falzar
    .org 0x080EC826
.else
    .org 0x080EDB66
.endif
    mov r0,10
