; vim:ft=fasm

include "../../Assembler/maluch.inc"

loop:
    mov r1, r2
    mov r1, 12
    jmp loop
