.gba
.thumb

.definelabel OBJECT_FREE,              0x08003458
.definelabel CUSTOM_OBJECT_KIND,       0x7C

.macro EngineCall,target
    push {r4}
    ldr r4,=target + 1
    mov r12,r4
    pop {r4}
    mov lr,pc
    bx r12
.endmacro

.if FALZAR
    .include "build/c-symbols-falzar.generated.asm"
    .org CCodeStart
    .incbin "build/gameplay-falzar.bin"
.else
    .include "build/c-symbols-gregar.generated.asm"
    .org CCodeStart
    .incbin "build/gameplay-gregar.bin"
.endif

.defineregion CCodeEnd,0x09000000-CCodeEnd,0xFF
.include "src/title_screen.asm"
.include "build/registry.generated.asm"
