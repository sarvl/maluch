; vim:ft=fasm
include "../Assembler/maluch.inc"

start:
    ; 1. Initialize Stack Pointer
    mov r2, $F000

    ; 2. Set default background to Black (0x00) and foreground to White (0xFF)
    mov r7, $02FF ; Foreground White (0xFF) command (0x02)
    out gpu, r7
    mov r7, $0300 ; Background Black (0x00) command (0x03)
    out gpu, r7

    ; 3. Clear Screen by filling VRAM with space character (0x0020)
    mov r3, $1DFF ; Last address of VRAM
    mov r4, ' '   ; Space char
clear_loop:
    stw r3, r4
    sub r3, 1
    bne clear_loop
    stw r3, r4    ; clear address 0

    ; 4. Startup delay (approx 5 seconds) to allow VGA monitor to sync
    mov r6, 250
startup_outer:
    mov r5, 50000
startup_inner:
    sub r5, 1
    bne startup_inner
    sub r6, 1
    bne startup_outer

    ; 5. Bouncing "m" in place at Col 39 (Double Jump)
    mov r9, 0
jump_loop:
    cmp r9, 0
    bee skip_erase_jump

    ; Erase previous "m" at Col 39
    mov r6, r9
    sub r6, 1
    mov r8, jump_table
    add r8, r6
    ldw r10, r8 ; r10 = prev_row
    lsl r10, 8
    or r10, 39  ; Col 39
    mov r11, ' '
    stw r10, r11

skip_erase_jump:
    ; Draw current "m" at Col 39
    mov r8, jump_table
    add r8, r9
    ldw r4, r8 ; r4 = row
    lsl r4, 8
    or r4, 39  ; Col 39
    mov r11, $1C6D ; Green 'm'
    stw r4, r11

    ; Snappy frame rate delay (approx 52ms per frame)
    call delay_frame

    add r9, 1
    cmp r9, 15
    bll jump_loop

    ; 6. Symmetrical Reveal Transition (approx 56ms per frame)
    ; m slides left to 37, h slides right to 42, revealing letters in both ways.

    ; Frame 1: m slides to 38, h appears at 41, L and U appear in the middle.
    ; (Writing L at 39 automatically overwrites the old m at 39)
    mov r10, (14 shl 8) or 38
    mov r11, $1C6D ; Green 'm'
    stw r10, r11

    mov r10, (14 shl 8) or 39
    mov r11, $E04C ; Red 'L'
    stw r10, r11

    mov r10, (14 shl 8) or 40
    mov r11, $E055 ; Red 'U'
    stw r10, r11

    mov r10, (14 shl 8) or 41
    mov r11, $1C68 ; Green 'h'
    stw r10, r11
    call delay_slide

    ; Frame 2: m slides to 37, h slides to 42. A and c appear behind them.
    ; (Writing A at 38 overwrites old m at 38, writing c at 41 overwrites old h at 41)
    mov r10, (14 shl 8) or 37
    mov r11, $1C6D ; Green 'm'
    stw r10, r11

    mov r10, (14 shl 8) or 38
    mov r11, $E041 ; Red 'A'
    stw r10, r11

    mov r10, (14 shl 8) or 41
    mov r11, $1C63 ; Green 'c'
    stw r10, r11

    mov r10, (14 shl 8) or 42
    mov r11, $1C68 ; Green 'h'
    stw r10, r11

    ; Loop infinitely
infinite_loop:
    jmp infinite_loop

; --- SUBROUTINES ---
delay_frame:
    mov r6, 4
delay_frame_outer:
    mov r5, 43750
delay_frame_inner:
    sub r5, 1
    bne delay_frame_inner
    sub r6, 1
    bne delay_frame_outer
    ret

delay_slide:
    mov r6, 6
delay_slide_outer:
    mov r5, 46875
delay_slide_inner:
    sub r5, 1
    bne delay_slide_inner
    sub r6, 1
    bne delay_slide_outer
    ret

; --- DATA TABLES ---
jump_table:
    dw 14, 12, 9, 7, 6, 7, 9, 12, 14  ; Jump 1 (reaches row 6)
    dw 12, 10, 9, 10, 12, 14          ; Jump 2 (reaches row 9)
