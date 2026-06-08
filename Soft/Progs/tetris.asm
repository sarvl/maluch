; vim:ft=fasm

include "../Assembler/maluch.inc"

; --- KONFIGURACJA KOMPILACJI ---
; 1 - Tryb Symulatora (podstawowe znaki ASCII, brak kolorów)
; 0 - Tryb Sprzętowy (znaki CP437, pełne wsparcie dla kolorów GPU)
SIM_MODE equ 0

if SIM_MODE = 1
    CHAR_EMPTY    equ '.'
    CHAR_BOARD    equ '#'
    CHAR_PIECE    equ '@'
    CHAR_BORDER_V equ '|'
    CHAR_BORDER_H equ '-'
    
    macro set_color c
    end macro
    macro clear_gpu 
    end macro
else
    CHAR_EMPTY    equ ' '
    CHAR_BOARD    equ 219 ; Pełny blok w CP437
    CHAR_PIECE    equ 219 ; Pełny blok w CP437
    CHAR_BORDER_V equ 186 ; Podwójna ramka pionowa w CP437
    CHAR_BORDER_H equ 205 ; Podwójna ramka pozioma w CP437
    
    macro set_color c
        mov r14, $0200 or c
        out gpu, r14
    end macro
    macro clear_gpu
        mov r14, $0400
        out gpu, r14
    end macro
end if

; --- PAMIĘĆ DANYCH ---
virtual at $10000
board: rw 200           ; Plansza 10x20
piece_x: rw 1           ; Pozycja X obecnego klocka
piece_y: rw 1           ; Pozycja Y obecnego klocka
piece_id: rw 1          ; Typ klocka (0-6)
piece_rot: rw 1         ; Rotacja (0-3)
next_piece_id: rw 1     ; Zmienna na następny klocek (Next Box)
tick_flag: rw 1         ; Flaga oznaczająca cykl opadania (1 sekunda)
key_pressed: rw 1       ; Kod ASCII ostatnio wciśniętego klawisza
rand_seed: rw 1         ; Ziarno generatora PRNG
game_over: rw 1         ; Flaga końca gry
timer_counter: rw 1     ; Licznik milisekund dla opadania klocka
score: rw 1             ; Aktualny wynik punktowy
vram_cache: rw 360      ; Cache dla differential VRAM rendering (prostokąt Y w [2..21], X w [10..27])
end virtual

; --- INICJALIZACJA ---
start:
    ; Ustawienie wskaźnika stosu
    mov r2, $F000 

    ; Wyzerowanie zmiennych stanu
    mov r3, 0
    stw key_pressed, r3
    stw tick_flag, r3
    stw game_over, r3
    stw timer_counter, r3
    stw score, r3
    mov r3, 1
    stw rand_seed, r3

    ; Ręczne czyszczenie VRAM (80x30)
    mov r4, 0           ; y = 0
clear_vram_y:
    cmp r4, 30
    bge clear_vram_done
    mov r5, 0           ; x = 0
clear_vram_x:
    cmp r5, 80
    bge clear_vram_next_y
    
    mov r6, r4
    lsl r6, 8
    or r6, r5           ; adres = (y << 8) | x
    
    mov r7, CHAR_EMPTY  ; spacja (kolor w starszym bajcie to 0)
    stw r6, r7
    
    add r5, 1
    jmp clear_vram_x
clear_vram_next_y:
    add r4, 1
    jmp clear_vram_y
clear_vram_done:

    ; Inicjalizacja vram_cache wartościami $FFFF (360 słów)
    mov r4, 0
    mov r5, $FFFF
init_cache_loop:
    cmp r4, 360
    bge init_cache_done
    mov r6, vram_cache
    add r6, r4
    stw r6, r5
    add r4, 1
    jmp init_cache_loop
init_cache_done:

    ; Wyczyść ekran GPU (jeśli sprzętowe)
    clear_gpu

    ; Inicjalizacja układu i wektora przerwań
    call init_board
    call init_graphics

    ; Przerwanie timera (ID = 0 -> skok pod $FFF0)
    mov r3, 01001000b shl 8 ; Instrukcja JMP imm
    mov r4, timer_handler
    stw $FFF0, r3
    stw $FFF1, r4

    ; Przerwanie klawiatury (ID = 1 -> skok pod $FFF2)
    mov r4, kb_handler
    stw $FFF2, r3
    stw $FFF3, r4

    ; Inicjalizacja pierwszego klocka dla "Next Box"
    call rand_mod7
    stw next_piece_id, r4

    ; Startowy klocek i pierwsze rysowanie
    call spawn_piece
    call draw_board

    ; Odblokowanie przerwań (Timer bit 15, KB bit 14)
    mov r1, $C000


; --- GŁÓWNA PĘTLA GRY ---
game_loop:
    ldw r4, game_over
    test r4, r4
    bne game_over_loop

    jmp continue_game_loop

game_over_loop:
    ldw r4, key_pressed
    test r4, r4
    bee game_over_loop
    
    ; Reset key_pressed
    mov r5, 0
    stw key_pressed, r5
    
    ; Sprawdzenie klawisza 'r' lub Enter ($0A)
    or r4, 32
    cmp r4, 'r'
    bee restart_game
    cmp r4, $0A
    bee restart_game
    jmp game_over_loop

restart_game:
    jmp start

continue_game_loop:
    ; "Kręcenie" generatorem liczb losowych dla losowości na podstawie czasu reakcji gracza
    call spin_rng

    ; Sprawdzenie, czy jest wciśnięty klawisz
    ldw r4, key_pressed
    test r4, r4
    bee check_tick

    call handle_key
    call draw_board

check_tick:
    ; Sprawdzenie, czy timer zgłosił tyknięcie zegara
    ldw r4, tick_flag
    test r4, r4
    bee game_loop

    ; Reset flagi
    mov r4, 0
    stw tick_flag, r4

    call do_tick
    call draw_board

    jmp game_loop


; --- LOGIKA CZASU (TICK) ---
do_tick:
    ldw r4, piece_x
    ldw r5, piece_y
    add r5, 1
    ldw r6, piece_id
    ldw r7, piece_rot
    call check_collision
    test r4, r4
    bne tick_collide

    ; Można opadać dalej
    ldw r5, piece_y
    add r5, 1
    stw piece_y, r5
    ret

tick_collide:
    call merge_piece
    call clear_lines
    call spawn_piece
    ret


; --- OBSŁUGA KLAWIATURY ---
handle_key:
    ldw r4, key_pressed
    mov r5, 0
    stw key_pressed, r5

    ; Konwersja na małe litery (wielkie 'A'-'Z' stają się 'a'-'z', spacja pozostaje)
    or r4, 32

    ; 'a' - w lewo
    cmp r4, 'a'
    bne hk_try_d
    ldw r4, piece_x
    sub r4, 1
    ldw r5, piece_y
    ldw r6, piece_id
    ldw r7, piece_rot
    call check_collision
    test r4, r4
    bne hk_end
    ldw r4, piece_x
    sub r4, 1
    stw piece_x, r4
    ret

hk_try_d:
    ; 'd' - w prawo
    cmp r4, 'd'
    bne hk_try_s
    ldw r4, piece_x
    add r4, 1
    ldw r5, piece_y
    ldw r6, piece_id
    ldw r7, piece_rot
    call check_collision
    test r4, r4
    bne hk_end
    ldw r4, piece_x
    add r4, 1
    stw piece_x, r4
    ret

hk_try_s:
    ; 's' - miękki opad (Soft Drop)
    cmp r4, 's'
    bne hk_try_w
    ldw r4, piece_x
    ldw r5, piece_y
    add r5, 1
    ldw r6, piece_id
    ldw r7, piece_rot
    call check_collision
    test r4, r4
    bne hk_end
    ldw r5, piece_y
    add r5, 1
    stw piece_y, r5
    ret

hk_try_w:
    ; 'w' - rotacja
    cmp r4, 'w'
    bne hk_try_space
    ldw r4, piece_x
    ldw r5, piece_y
    ldw r6, piece_id
    ldw r7, piece_rot
    add r7, 1
    and r7, 3
    call check_collision
    test r4, r4
    bne hk_end
    ldw r7, piece_rot
    add r7, 1
    and r7, 3
    stw piece_rot, r7
    ret

hk_try_space:
    ; ' ' - twardy opad (Hard Drop)
    cmp r4, ' '
    bne hk_end
hd_loop:
    ldw r4, piece_x
    ldw r5, piece_y
    add r5, 1
    ldw r6, piece_id
    ldw r7, piece_rot
    call check_collision
    test r4, r4
    bne hk_end
    ldw r5, piece_y
    add r5, 1
    stw piece_y, r5
    jmp hd_loop

hk_end:
    ret


; --- SYSTEM KRYZYSOWY GRY (KOLIZJE, ŁĄCZENIE ITP.) ---
spawn_piece:
    ; Wylosuj klocka z bufora "Następny"
    ldw r4, next_piece_id
    stw piece_id, r4

    ; Przelosuj i zapisz nową figurę na miejsce starej w buforze
    call rand_mod7
    stw next_piece_id, r4

    mov r4, 0
    stw piece_rot, r4
    mov r4, 3
    stw piece_x, r4
    mov r4, 0
    stw piece_y, r4

    ; Sprawdzanie czy natychmiastowa kolizja kończy grę
    ldw r4, piece_x
    ldw r5, piece_y
    ldw r6, piece_id
    ldw r7, piece_rot
    call check_collision
    test r4, r4
    bee sp_ok

    ; Koniec gry (GameOver)
    mov r4, 1
    stw game_over, r4
    call draw_game_over
sp_ok:
    ret


; Rejestry powrotne kolizji: 1 w r4 jeśli kolizja, 0 w r4 jeśli wolne
check_collision:
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13

    ; Wskaźnik r8 = pieces_table + ((id * 4) + rot) * 8
    mov r8, r6
    lsl r8, 2
    add r8, r7
    lsl r8, 3
    add r8, pieces_table

    mov r9, 0 ; i = 0
cc_loop:
    cmp r9, 4
    bge cc_ok

    mov r10, r9
    lsl r10, 1
    add r10, r8
    ldw r11, r10 ; bx
    add r10, 1
    ldw r12, r10 ; by

    add r11, r4 ; cx = x + bx
    add r12, r5 ; cy = y + by

    ; Detekcja ścian bocznych i dolnej
    cmp r11, 0
    bll cc_fail
    cmp r11, 10
    bge cc_fail

    cmp r12, 0
    bll cc_next ; Akceptuj bloki nad planszą (Y < 0)
    cmp r12, 20
    bge cc_fail

    ; Detekcja zajętych bloków na planszy
    mov r10, r12
    lsl r10, 3
    mov r13, r12
    lsl r13, 1
    add r10, r13  ; cy * 10
    add r10, r11  ; + cx
    add r10, board
    ldw r10, r10
    test r10, r10
    bne cc_fail

cc_next:
    add r9, 1
    jmp cc_loop

cc_fail:
    mov r4, 1
    jmp cc_end

cc_ok:
    mov r4, 0
cc_end:
    pull r13
    pull r12
    pull r11
    pull r10
    pull r9
    pull r8
    ret


merge_piece:
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13

    ldw r4, piece_x
    ldw r5, piece_y
    ldw r6, piece_id
    ldw r7, piece_rot

    mov r8, r6
    lsl r8, 2
    add r8, r7
    lsl r8, 3
    add r8, pieces_table

    mov r9, 0
mp_loop:
    cmp r9, 4
    bge mp_end

    mov r10, r9
    lsl r10, 1
    add r10, r8
    ldw r11, r10
    add r10, 1
    ldw r12, r10

    add r11, r4
    add r12, r5

    cmp r12, 0
    bll mp_next

    mov r10, r12
    lsl r10, 3
    mov r13, r12
    lsl r13, 1
    add r10, r13
    add r10, r11
    add r10, board
    mov r13, 1
    stw r10, r13

mp_next:
    add r9, 1
    jmp mp_loop

mp_end:
    pull r13
    pull r12
    pull r11
    pull r10
    pull r9
    pull r8
    ret


clear_lines:
    push r4
    push r5
    push r6
    push r7
    push r8
    push r9
    push r10
    push r11

    mov r11, 0 ; Licznik wyczyszczonych linii w tym cyklu
    mov r4, 19
cl_loop_y:
    cmp r4, 0
    bll cl_end

    mov r5, 0 ; licznik zapelnienia
    mov r6, 0 ; x
cl_loop_x:
    cmp r6, 10
    bge cl_check_full

    mov r7, r4
    lsl r7, 3
    mov r8, r4
    lsl r8, 1
    add r7, r8
    add r7, r6
    add r7, board
    ldw r7, r7
    test r7, r7
    bee cl_next_x
    add r5, 1
cl_next_x:
    add r6, 1
    jmp cl_loop_x

cl_check_full:
    cmp r5, 10
    bne cl_not_full

    ; Przesunięcie pamięci w dół dla pełnej linii
    mov r5, r4
cl_shift_loop:
    cmp r5, 0
    bee cl_shift_zero

    mov r6, 0
cl_copy_x:
    cmp r6, 10
    bge cl_shift_next

    ; src = r5-1
    mov r7, r5
    sub r7, 1
    mov r8, r7
    lsl r8, 3
    mov r9, r7
    lsl r9, 1
    add r8, r9
    add r8, r6
    add r8, board
    ldw r8, r8

    ; dst = r5
    mov r7, r5
    mov r9, r7
    lsl r9, 3
    mov r10, r7
    lsl r10, 1
    add r9, r10
    add r9, r6
    add r9, board
    stw r9, r8

    add r6, 1
    jmp cl_copy_x

cl_shift_next:
    sub r5, 1
    jmp cl_shift_loop

cl_shift_zero:
    mov r6, 0
    mov r8, 0
cl_clear_0:
    cmp r6, 10
    bge cl_shifted
    mov r7, board
    add r7, r6
    stw r7, r8
    add r6, 1
    jmp cl_clear_0

cl_shifted:
    add r11, 1 ; Zwiększ licznik linii
    add r4, 1 ; powtórz sprawdzanie bieżącego wiersza y, ponieważ bloki zjechały w dół

cl_not_full:
    sub r4, 1
    jmp cl_loop_y

cl_end:
    ; Naliczanie punktów za wyczyszczone linie
    test r11, r11
    bee cl_no_points
    
    cmp r11, 1
    bne cl_try_2
    ldw r4, score
    add r4, 100
    stw score, r4
    jmp cl_no_points
cl_try_2:
    cmp r11, 2
    bne cl_try_3
    ldw r4, score
    add r4, 300
    stw score, r4
    jmp cl_no_points
cl_try_2_ext: ; helper label for far jumps if needed
cl_try_3:
    cmp r11, 3
    bne cl_try_4
    ldw r4, score
    add r4, 500
    stw score, r4
    jmp cl_no_points
cl_try_4:
    ; 4 lub więcej linii
    ldw r4, score
    add r4, 800
    stw score, r4

cl_no_points:
    pull r11
    pull r10
    pull r9
    pull r8
    pull r7
    pull r6
    pull r5
    pull r4
    ret


; --- INICJALIZACJE I GRAFIKA (GPU VRAM) ---
init_board:
    mov r4, 0
    mov r5, 0
ib_loop:
    cmp r4, 200
    bge ib_end
    mov r6, board
    add r6, r4
    stw r6, r5
    add r4, 1
    jmp ib_loop
ib_end:
    ret

init_graphics:
    set_color 8 ; Ciemnoszary dla elementów stałych
    mov r4, 2
ig_loop_y:
    cmp r4, 22
    bge ig_bottom
    mov r5, r4
    lsl r5, 8
    add r5, 9
    mov r6, CHAR_BORDER_V
    stw r5, r6
    mov r5, r4
    lsl r5, 8
    add r5, 20
    stw r5, r6
    add r4, 1
    jmp ig_loop_y
ig_bottom:
    mov r5, 9
ig_loop_x:
    cmp r5, 21
    bge ig_next_text
    mov r6, 22
    lsl r6, 8
    add r6, r5
    mov r7, CHAR_BORDER_H
    stw r6, r7
    add r5, 1
    jmp ig_loop_x

ig_next_text:
    set_color 14 ; Żółty dla napisu NEXT
    ; Rysowanie napisu NEXT dla Next Box'a
    mov r4, $0218 ; Wiersz 2, Kolumna 24
    mov r5, 'N'
    stw r4, r5
    add r4, 1
    mov r5, 'E'
    stw r4, r5
    add r4, 1
    mov r5, 'X'
    stw r4, r5
    add r4, 1
    mov r5, 'T'
    stw r4, r5

    ; Rysowanie napisu SCORE
    set_color 14 ; Żółty
    mov r4, $0A18 ; Wiersz 10, Kolumna 24
    mov r5, 'S'
    stw r4, r5
    add r4, 1
    mov r5, 'C'
    stw r4, r5
    add r4, 1
    mov r5, 'O'
    stw r4, r5
    add r4, 1
    mov r5, 'R'
    stw r4, r5
    add r4, 1
    mov r5, 'E'
    stw r4, r5

ig_end:
    ret

draw_board:
    push r4
    ldw r4, game_over
    test r4, r4
    bee db_begin
    pull r4
    ret

db_begin:
    push r5
    push r6
    push r7
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13

    mov r4, 0
db_loop_y:
    cmp r4, 20
    bge db_draw_piece
    mov r5, 0
db_loop_x:
    cmp r5, 10
    bge db_next_y

    mov r6, r4
    lsl r6, 3
    mov r7, r4
    lsl r7, 1
    add r6, r7
    add r6, r5
    add r6, board
    ldw r6, r6

    mov r7, r4
    add r7, 2
    lsl r7, 8
    mov r8, r5
    add r8, 10
    or r7, r8

    test r6, r6
    bee db_draw_empty
    mov r13, (7 shl 8) or CHAR_BOARD
    mov r10, r7
    call write_vram_cached
    jmp db_next_x
db_draw_empty:
    mov r13, (0 shl 8) or CHAR_EMPTY
    mov r10, r7
    call write_vram_cached

db_next_x:
    add r5, 1
    jmp db_loop_x
db_next_y:
    add r4, 1
    jmp db_loop_y

db_draw_piece:
    ldw r4, piece_x
    ldw r5, piece_y
    ldw r6, piece_id
    ldw r7, piece_rot

    mov r8, r6
    lsl r8, 2
    add r8, r7
    lsl r8, 3
    add r8, pieces_table

    mov r9, 0
dp_loop:
    cmp r9, 4
    bge db_draw_next

    mov r10, r9
    lsl r10, 1
    add r10, r8
    ldw r11, r10
    add r10, 1
    ldw r12, r10

    add r11, r4
    add r12, r5

    cmp r12, 0
    bll dp_next

    mov r10, r12
    add r10, 2
    lsl r10, 8
    mov r13, r11
    add r13, 10
    or r10, r13
    mov r13, (10 shl 8) or CHAR_PIECE
    call write_vram_cached

dp_next:
    add r9, 1
    jmp dp_loop


db_draw_next:
    ; Wyczyść sekcję Next Box
    mov r4, 4 ; y = 4
dnb_clear_y:
    cmp r4, 8
    bge dnb_draw_piece
    mov r5, 24 ; x = 24
dnb_clear_x:
    cmp r5, 28
    bge dnb_next_y
    mov r6, r4
    lsl r6, 8
    or r6, r5
    mov r10, r6
    mov r13, (0 shl 8) or CHAR_EMPTY
    call write_vram_cached
    add r5, 1
    jmp dnb_clear_x
dnb_next_y:
    add r4, 1
    jmp dnb_clear_y

dnb_draw_piece:
    ; Narysuj aktualnego klocka wpisanego w zmienną next_piece
    ldw r6, next_piece_id
    mov r7, 0 ; standardowa rotacja
    mov r8, r6
    lsl r8, 2
    add r8, r7
    lsl r8, 3
    add r8, pieces_table

    mov r9, 0
dnp_loop:
    cmp r9, 4
    bge db_draw_score

    mov r10, r9
    lsl r10, 1
    add r10, r8
    ldw r11, r10 ; bx
    add r10, 1
    ldw r12, r10 ; by

    mov r10, r12
    add r10, 4
    lsl r10, 8
    mov r13, r11
    add r13, 24
    or r10, r13
    mov r13, (11 shl 8) or CHAR_PIECE
    call write_vram_cached

    add r9, 1
    jmp dnp_loop

db_draw_score:
    call draw_score

db_end:
    pull r13
    pull r12
    pull r11
    pull r10
    pull r9
    pull r8
    pull r7
    pull r6
    pull r5
    pull r4
    ret


; Subroutine write_vram_cached
; Inputs:
;   r10: VRAM address (y << 8) | x
;   r13: VRAM data (color << 8) | character
write_vram_cached:
    push r4
    push r5
    push r6
    push r7

    ; Extract y (r10 >> 8)
    mov r4, r10
    lsr r4, 8

    ; Extract x (r10 & $FF)
    mov r5, r10
    and r5, $FF

    ; Check if y in [2..21] (i.e. y >= 2 and y < 22)
    cmp r4, 2
    bll wvc_direct
    cmp r4, 22
    bge wvc_direct

    ; Check if x in [10..27] (i.e. x >= 10 and x < 28)
    cmp r5, 10
    bll wvc_direct
    cmp r5, 28
    bge wvc_direct

    ; Calculate cache index: (y - 2) * 18 + (x - 10)
    mov r6, r4
    sub r6, 2

    ; (y - 2) * 18 = ((y - 2) << 4) + ((y - 2) << 1)
    mov r7, r6
    lsl r7, 4
    mov r4, r6
    lsl r4, 1
    add r7, r4

    ; + (x - 10)
    sub r5, 10
    add r7, r5

    ; Add base address of vram_cache
    add r7, vram_cache

    ; Load cached value
    ldw r4, r7

    ; Compare with new value r13
    cmp r4, r13
    bee wvc_end

    ; If different, update cache and write to VRAM
    stw r7, r13
    stw r10, r13

wvc_end:
    pull r7
    pull r6
    pull r5
    pull r4
    ret

wvc_direct:
    ; Out of bounds writes go directly to VRAM
    stw r10, r13
    pull r7
    pull r6
    pull r5
    pull r4
    ret


; Subroutine draw_score
; Converts 16-bit score to 4 ASCII digits and draws them at Row 11, Cols 24..27
draw_score:
    push r4
    push r5
    push r6
    push r7
    push r10
    push r13

    ldw r4, score

    ; Thousands
    mov r5, 0
ds_thousands_loop:
    cmp r4, 1000
    bll ds_thousands_done
    sub r4, 1000
    add r5, 1
    jmp ds_thousands_loop
ds_thousands_done:
    add r5, '0'
    mov r10, $0B18
    mov r13, 15 shl 8
    or r13, r5
    call write_vram_cached

    ; Hundreds
    mov r5, 0
ds_hundreds_loop:
    cmp r4, 100
    bll ds_hundreds_done
    sub r4, 100
    add r5, 1
    jmp ds_hundreds_loop
ds_hundreds_done:
    add r5, '0'
    mov r10, $0B19
    mov r13, 15 shl 8
    or r13, r5
    call write_vram_cached

    ; Tens
    mov r5, 0
ds_tens_loop:
    cmp r4, 10
    bll ds_tens_done
    sub r4, 10
    add r5, 1
    jmp ds_tens_loop
ds_tens_done:
    add r5, '0'
    mov r10, $0B1A
    mov r13, 15 shl 8
    or r13, r5
    call write_vram_cached

    ; Ones
    add r4, '0'
    mov r10, $0B1B
    mov r13, 15 shl 8
    or r13, r4
    call write_vram_cached

    pull r13
    pull r10
    pull r7
    pull r6
    pull r5
    pull r4
    ret


draw_game_over:
    push r4
    push r5
    push r10
    push r13

    ; Rysuj "GAME OVER" na czerwono (12)
    mov r4, $090B ; Wiersz 9, Kolumna 11
    mov r5, 'G'
    mov r13, 12 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached
    add r4, 1
    mov r5, 'A'
    mov r13, 12 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached
    add r4, 1
    mov r5, 'M'
    mov r13, 12 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached
    add r4, 1
    mov r5, 'E'
    mov r13, 12 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached

    mov r4, $0910 ; Wiersz 9, Kolumna 16
    mov r5, 'O'
    mov r13, 12 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached
    add r4, 1
    mov r5, 'V'
    mov r13, 12 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached
    add r4, 1
    mov r5, 'E'
    mov r13, 12 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached
    add r4, 1
    mov r5, 'R'
    mov r13, 12 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached

    ; Rysuj "PRESS R" na żółto (14)
    mov r4, $0B0B ; Wiersz 11, Kolumna 11
    mov r5, 'P'
    mov r13, 14 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached
    add r4, 1
    mov r5, 'R'
    mov r13, 14 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached
    add r4, 1
    mov r5, 'E'
    mov r13, 14 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached
    add r4, 1
    mov r5, 'S'
    mov r13, 14 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached
    add r4, 1
    mov r5, 'S'
    mov r13, 14 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached
    add r4, 2 ; Pusta spacja przed R
    mov r5, 'R'
    mov r13, 14 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached

    ; Rysuj "TO PLAY" na żółto (14)
    mov r4, $0C0B ; Wiersz 12, Kolumna 11
    mov r5, 'T'
    mov r13, 14 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached
    add r4, 1
    mov r5, 'O'
    mov r13, 14 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached
    add r4, 2
    mov r5, 'P'
    mov r13, 14 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached
    add r4, 1
    mov r5, 'L'
    mov r13, 14 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached
    add r4, 1
    mov r5, 'A'
    mov r13, 14 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached
    add r4, 1
    mov r5, 'Y'
    mov r13, 14 shl 8
    or r13, r5
    mov r10, r4
    call write_vram_cached

    pull r13
    pull r10
    pull r5
    pull r4
    ret


; --- MATEMATYKA I PRNG ---
spin_rng:
    ldw r4, rand_seed
    test r4, r4
    bne sr_nz
    mov r4, 1
sr_nz:
    mov r5, r4
    lsl r5, 7
    xor r4, r5
    mov r5, r4
    lsr r5, 9
    xor r4, r5
    mov r5, r4
    lsl r5, 8
    xor r4, r5
    stw rand_seed, r4
    ret

rand_mod7:
    ldw r4, rand_seed
    and r4, $7FFF   ; FIX BŁĘDU Z KLONOWANIEM/1x1: usunięcie bita znaku chroniące przed generowaniem ujemnych wyników
rnd_mod700:
    cmp r4, 700
    bll rnd_mod70
    sub r4, 700
    jmp rnd_mod700
rnd_mod70:
    cmp r4, 70
    bll rnd_mod7
    sub r4, 70
    jmp rnd_mod70
rnd_mod7:
    cmp r4, 7
    bll rnd_done
    sub r4, 7
    jmp rnd_mod7
rnd_done:
    ret


; --- PRZERWANIA ---
timer_handler:
    push r4
    push r5
    in r4, timer ; potwiedź przerwanie
    
    ldw r4, timer_counter
    add r4, 1
    cmp r4, 700 ; 1000 ms = 1 sekunda
    bll th_save
    
    mov r4, 0
    mov r5, 1
    stw tick_flag, r5
th_save:
    stw timer_counter, r4
    pull r5
    pull r4
    iret

kb_handler:
    push r4
    in r4, kb ; kod z klawiatury jest sczytywany jako znak ASCII i potwiedza przerwanie
    stw key_pressed, r4
    pull r4
    iret


; --- DEFICINJE BAZY KLOCKÓW ---
pieces_table:
; I piece
dw 0,1, 1,1, 2,1, 3,1
dw 2,0, 2,1, 2,2, 2,3
dw 0,2, 1,2, 2,2, 3,2
dw 1,0, 1,1, 1,2, 1,3
; J piece
dw 0,0, 0,1, 1,1, 2,1
dw 1,0, 2,0, 1,1, 1,2
dw 0,1, 1,1, 2,1, 2,2
dw 1,0, 1,1, 0,2, 1,2
; L piece
dw 2,0, 0,1, 1,1, 2,1
dw 1,0, 1,1, 1,2, 2,2
dw 0,1, 1,1, 2,1, 0,2
dw 0,0, 1,0, 1,1, 1,2
; O piece
dw 1,0, 2,0, 1,1, 2,1
dw 1,0, 2,0, 1,1, 2,1
dw 1,0, 2,0, 1,1, 2,1
dw 1,0, 2,0, 1,1, 2,1
; S piece
dw 1,0, 2,0, 0,1, 1,1
dw 1,0, 1,1, 2,1, 2,2
dw 1,1, 2,1, 0,2, 1,2
dw 0,0, 0,1, 1,1, 1,2
; T piece
dw 1,0, 0,1, 1,1, 2,1
dw 1,0, 1,1, 2,1, 1,2
dw 0,1, 1,1, 2,1, 1,2
dw 1,0, 0,1, 1,1, 1,2
; Z piece
dw 0,0, 1,0, 1,1, 2,1
dw 2,0, 1,1, 2,1, 1,2
dw 0,1, 1,1, 1,2, 2,2
dw 1,0, 0,1, 1,1, 0,2
