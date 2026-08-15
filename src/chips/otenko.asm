// The English ROM retained Otenko's effect routine but replaced its sprite ID
// with 0. Restore the Japanese group-0x0C/id-0x49 handle.
.if falzar
    .org 0x080DB2F0
.else
    .org 0x080DCB60
.endif
    .db 0x49
