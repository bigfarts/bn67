// Crossover spawns Django's GunDelSol as effect variants 0x0B and 0x0C.
// The English ROM redirects both variants from group 0x0C/id 0x0F to id 0,
// even though the effect animations remain in Django's imported archive.
.if falzar
    .org 0x080B8C0C
    .db 0x0F
    .org 0x080B8C11
    .db 0x0F
.else
    .org 0x080BA47C
    .db 0x0F
    .org 0x080BA481
    .db 0x0F
.endif
