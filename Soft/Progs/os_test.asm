; vim:ft=fasm

include "../Assembler/maluch.inc"

stack_ptr equ r2

saved_0 equ r4
saved_1 equ r5
saved_2 equ r6
saved_3 equ r7

repeat 8, temp_num:0, reg_num:8
    temp_#temp_num equ r#reg_num
end repeat

BS equ 08
NL equ 10

jmp_insn equ 01001000b shl 8

macro print_str addr, size
        mov r8, addr
        mov r9, size
        call kernel_screen_print_str
end macro

macro print_str_newline addr, size
        mov r8, addr
        mov r9, size
        call kernel_screen_print_str_nl
end macro

macro compare_strings addr_0, addr_1, length
        mov r8, addr_0
        mov r9, addr_1
        mov r10, length
        call string_compare
end macro

macro interrupt_enable id
        if id > 7
            err "ERROR: interrupt ID cannot be greater than 7"
        end if
        or r1, 0001h shl (15 - id)
end macro

start:
        ; SET UP STACK POINTER
        mov stack_ptr, $F000

        ; SET UP THE INTERRUPT TABLE
        mov r3, jmp_insn
        mov r4, timer_handler
        stw $FFF0 + timer*2, r3
        stw $FFF1 + timer*2, r4

        mov r4, kb_handler
        stw $FFF0 + kb*2, r3
        stw $FFF1 + kb*2, r4

        mov r3, 0
repeat 6, i:2
        stw $FFF0 + i*2, r3
end repeat
        ; INITIALIZE VARIABLES
        stw time_elapsed, r3
        stw buffer, r3
        stw cmd_ready, r3
        stw screen_cursor, r3
        mov r3, $FF00
        stw buffer_pos, r3
shell_start:
        print_str_newline str_shell, str_shell_length
        interrupt_enable timer
        interrupt_enable kb
shell_loop:
        ; PRINT PROMPT
        mov temp_0, '>'
        call kernel_screen_print_char
         mov temp_0, ' '
 repeat 100h - 16, idx:0
         stw buffer+idx, temp_0
 end repeat

shell_cmd_wait:
        ldw saved_0, cmd_ready
        test saved_0, saved_0 
        bee shell_cmd_wait ; IF cmd_ready == 0: LOOP
        
        mov saved_0, 0 
        stw cmd_ready, saved_0 ; cmd_ready = 0
        
irp cmd, cmd_help, cmd_echo, cmd_time, cmd_clear
        compare_strings buffer, str_#cmd, str_#cmd#_length
        test temp_0, temp_0
        bne shell_#cmd
end irp
        print_str_newline str_cmd_invalid, str_cmd_invalid_length
        jmp shell_loop

shell_cmd_help:
        print_str_newline str_cmd_help_reply, str_cmd_help_reply_length
        jmp shell_loop

shell_cmd_echo:
        mov temp_0, buffer+5
        mov temp_1, ($FF-5)
        call kernel_screen_print_str_nl
        jmp shell_loop

shell_cmd_time:
        ldw temp_0, time_elapsed
        call kernel_screen_print_reg
        mov temp_0, NL
        call kernel_screen_print_char
        jmp shell_loop

shell_cmd_clear:
        call kernel_screen_clear
        jmp shell_loop
        
; this could be refactored into fasmg namespaces
kernel:
kernel_screen:
kernel_screen_print_char:
        ldw temp_1, screen_cursor

        cmp temp_0, NL
        bee kernel_screen_print_char_newline

        cmp temp_0, BS
        bee kernel_screen_print_char_backspace

        stw temp_1, temp_0

        add temp_1, 1

        and temp_1, 7FFFh
        stw screen_cursor, temp_1

        ret

kernel_screen_print_char_newline:
        add temp_1, 100h
        and temp_1, $7F00
        stw screen_cursor, temp_1

        ret

kernel_screen_print_char_backspace:
        sub temp_1, 1
        and temp_1, $7FFF
        stw screen_cursor, temp_1

        mov temp_0, ' '
        stw temp_1, temp_0

        ret

kernel_screen_print_str:
        test temp_1, temp_1
        bee kernel_screen_print_str_ret

        push temp_0
        push temp_1

        ldw temp_0, temp_0
        call kernel_screen_print_char

        pull temp_1
        pull temp_0

        add temp_0, 1
        sub temp_1, 1

        jmp kernel_screen_print_str

kernel_screen_print_str_ret:
        ret

kernel_screen_print_str_nl:
        call kernel_screen_print_str
        mov temp_0, NL
        jmp kernel_screen_print_char

kernel_screen_print_reg:
        push saved_0
        
        mov saved_0, temp_0

        mov temp_0, 'x'
        call kernel_screen_print_char

i = 12
while i >= 0
        mov temp_0, saved_0
        lsr temp_0, i
        and temp_0, 1111b
        add temp_0, print_convert_hex
        ldw temp_0, temp_0
        call kernel_screen_print_char
i = i - 4
end while
        pull saved_0
        ret

kernel_screen_print_bool:
        test temp_0, temp_0
        bee kernel_screen_print_bool_false

        print_str str_true, str_true_length
        ret

kernel_screen_print_bool_false:
        print_str str_false, str_false_length
        ret

kernel_screen_clear:
        mov temp_0, $7FFF
        mov temp_1, ' '

kernel_screen_clear_loop:
        stw temp_0, temp_1
        sub temp_0, 1
        bne kernel_screen_clear_loop

        stw temp_0, temp_1

        ; because temp_0 == 0
        stw screen_cursor, temp_0

        ret

string:
string_compare:
        test temp_2, temp_2
        bee string_compare_true

        ldw temp_3, temp_0
        ldw temp_4, temp_1

        cmp temp_3, temp_4
        bne string_compare_false

        add temp_0, 1
        add temp_1, 1
        sub temp_2, 1
        jmp string_compare

string_compare_true:
        mov temp_0, 1
        ret

string_compare_false:
        mov temp_0, 0
        ret

IHR:
timer_handler:
        push temp_0

        in temp_0, timer
        ldw temp_0, time_elapsed
        add temp_0, 1
        stw time_elapsed, temp_0
        
        pull temp_0
        iret

kb_handler:
        push saved_0
        push saved_1
        push temp_0
        in saved_0, kb 
        ldw saved_1, buffer_pos
        mov temp_0, saved_0
        call kernel_screen_print_char

        cmp saved_0, NL
        bee IHR_1_new_cmd 

        cmp saved_0, BS
        bee IHR_1_buffer_reduce
        stw saved_1, saved_0

        add saved_1, 1
        cmp saved_1, $FFF0
        bll IHR_1_skip_0

        mov saved_1, $FF00
IHR_1_skip_0:
        stw buffer_pos, saved_1
        
        pull temp_0
        pull saved_1
        pull saved_0

        iret

IHR_1_new_cmd:
        mov saved_1, buffer
        stw buffer_pos, saved_1

        mov temp_0, 1
check:
        stw cmd_ready, temp_0

        pull temp_0
        pull saved_1
        pull saved_0
        iret

IHR_1_buffer_reduce:
        cmp saved_1, $FF00
        bee IHR_1_ret

        sub saved_1, 1
        mov saved_0, ' '
        stw saved_1, saved_0

        stw buffer_pos, saved_1

IHR_1_ret:
        pull temp_0
        pull saved_1
        pull saved_0
        iret

str_shell:                db "mALUsh v0.1"
str_shell_length          = ($/2) - str_shell
str_true:                 db "true"
str_true_length           = ($/2) - str_true
str_false:                db "false"
str_false_length          = ($/2) - str_false
str_cmd_help:             db "help"
str_cmd_help_length       = ($/2) - str_cmd_help
str_cmd_time:             db "time"
str_cmd_time_length       = ($/2) - str_cmd_time
str_cmd_help_reply:       db "help - prints this help",NL,"echo string - prints string",NL,"time - prints elapsed time",NL,"clear - clears the screen"
str_cmd_help_reply_length = ($/2) - str_cmd_help_reply
str_cmd_echo:             db "echo"
str_cmd_echo_length       = ($/2) - str_cmd_echo
str_cmd_clear:            db "clear"
str_cmd_clear_length      = ($/2) - str_cmd_clear
str_cmd_invalid:          db "invalid command"
str_cmd_invalid_length    = ($/2) - str_cmd_invalid

print_convert_hex: db "0123456789ABCDEF",0

; RAM
virtual at 10000h
time_elapsed: dw ?
screen_cursor: dw ?
cmd_ready: dw ?
buffer_pos: dw ?
end virtual
virtual at $FF00 * 2
assert $ = $FF00 * 2
buffer: rw 100h - 16
end virtual
