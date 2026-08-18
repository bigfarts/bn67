.gba
.thumb

.definelabel object_free,              0x08003458

.macro copy_c_data,address,symbol,size
    .org address
    .if falzar
        .incbin "build/gameplay-falzar.bin",symbol-c_code_start,size
    .else
        .incbin "build/gameplay-gregar.bin",symbol-c_code_start,size
    .endif
.endmacro

.include "src/hooks.asm"
.include "src/chips/django.asm"
.include "src/chips/otenko.asm"
.include "src/chips/navi_variants.asm"

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
