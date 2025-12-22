; vim:ft=fasm

repeat 16, i:0
  r#i? equ [:i:]
end repeat

timer? equ 000b
kb?    equ 001b
gpu?   equ 010b
pstor? equ 011b


; LABEL INTERCEPTING
struc ? tail&
    match :, tail
        ; CASE: LABEL:
        label . at $ shr 1
    else match : instruction, tail
        ; CASE: LABEL: MNEMONIC
        label . at $ shr 1
        instruction
    else
        ; CASE: NO LABEL
        . tail
    end match
end struc

; DB AND DW REPLACEMENTS
calminstruction db? data&
    local value, char, len, i, tmp
loop:
    match value =, data, data
    jyes process_value
    
    ; Handle the last argument (no comma left)
    arrange value, data
    arrange data, 
    
process_value:
    ; 2. Check for Reserve (?)
    match =?, value
    jyes reserve
    
    ; 3. Check for String Literal ('...')
    ; The :quoted modifier matches only quoted strings
    match tmp:quoted, value, :
    jyes is_string
    
    ; 4. Default: Number or Label
    emit 1, value
    jump next_item
    
is_string:
    ; Iterate over string characters
    compute len, lengthof value
    compute i, 0
str_loop:
    check i < len
    jno next_item
    ; Extract byte: (string >> (i*8)) & 0xFF
    compute char, (value shr (i * 8)) and 0FFh
    emit 1, 0
    emit 1, char
    compute i, i + 1
    jump str_loop
    
reserve:
    ; emit with 1 argument reserves uninitialized space (like rb)
    emit 1
    
next_item:
    ; Continue if there are more arguments
    match , data
    jno loop
    exit
end calminstruction

calminstruction dw? data&
    local value, cmd

loop:
    match value =, data, data
    jyes check_value

    ; handle last (or only) argument
    match =?, data
    jyes reserve_last

    emit 2, data bswap 2
    exit

reserve_last:
    arrange cmd, =rb 2
    assemble cmd
    exit

check_value:
    match =?, value
    jyes reserve_value

    emit 2, value bswap 2
    jump loop

reserve_value:
    arrange cmd, =rb 2
    assemble cmd
    jump loop
end calminstruction

; DYADIC
irp <name,op>, mov,10h, add,20h, sub,21h, \
  and,22h, or,23h, xor,24h, not,25h, lsl,26h, \
  lsr,27h, cmp,31h, ldw,80h, stw,90h
    calminstruction name? dst, src
        local dst_idx, src_idx, cmd
        transform dst

        match [:dst_idx:], dst
        jyes dst_is_reg

        ; DESTINATION IS NOT A REGISTER
        arrange cmd, =err "Destination must be a register."
        assemble cmd
        exit
dst_is_reg:
        check dst_idx = 0
        jno dst_ok

        ; DESTINATION IS R0
        arrange cmd, =err "Cannot modify r0."
        assemble cmd
        exit
dst_ok:
        transform src
        match [:src_idx:], src
        jyes src_is_reg

        ; SOURCE IS IMMEDIATE
        emit 1, op or 1000b
        emit 1, dst_idx shl 4
        emit 2, src bswap 2
        exit

src_is_reg:
        emit 1, op
        emit 1, (dst_idx shl 4) or src_idx
    end calminstruction
end irp

; The test instruction is separate, because it doesn't modify destination
; so it can have r0 as it's dst
calminstruction test? dst, src
    local dst_idx, src_idx, cmd

    transform dst
    match [:dst_idx:], dst
    jyes dst_is_reg

    ; DESTINATION IS NOT A REGISTER
    arrange cmd, =err "Destination must be a register."
    assemble cmd
    exit
dst_is_reg:
    transform src
    match [:src_idx:], src
    jyes src_is_reg

    ; SOURCE IS IMMEDIATE
    emit 1, 32h or 1000b
    emit 1, dst_idx shl 4
    emit 2, src bswap 2
    exit

src_is_reg:
    emit 1, 32h
    emit 1, (dst_idx shl 4) or src_idx
end calminstruction

; JUMPS
irp <name,op>, jmp,40h, bee,41h, bne,42h, \
  bge,43h, ble,44h, bgg,45h, bll,46h, boo,47h, \
  bbs,50h, bss,51h, bns,52h, bae,53h, bbe,54h, \
  baa,55h, bbb,56h, bno,57h, call,$A0
    calminstruction name? src
        transform src
        match [:src_idx:], src
        jyes short_jump

        ; CASE: NUMBER
        emit 1, op or 1000b
        emit 1, 0
        emit 2, src bswap 2
        exit

short_jump:
        ; CASE: SHORT INDEX
        emit 1, op
        emit 1, src_idx
    end calminstruction

end irp

; STACK
calminstruction push? src
    local src_idx

    transform src
    match [:src_idx:], src
    jyes is_reg

    ; PUSH IMMEDIATE
    emit 1, $C0 or 1000b
    emit 1, 0
    emit 2, src bswap 2
    exit

is_reg:
    ; PUSH REGISTER
    emit 1, $C0
    emit 1, src_idx
end calminstruction

calminstruction pull? dst
    local dst_idx, cmd

    transform dst
    match [:dst_idx:], dst
    jyes dst_ok

    arrange cmd, =err "Destination must be a register."
    assemble cmd
    exit

dst_ok:
    check dst_idx = 0
    jno no_error
    arrange cmd, =err "Cannot modify r0."
    assemble cmd
    exit

no_error:
    emit 1, $D0
    emit 1, dst_idx shl 4
end calminstruction

; IO
calminstruction in? dev, dst
    local dst_idx, cmd
    transform dst

    check dev > 7
    jno dev_ok
    arrange cmd, =err "Invalid IO device ID (Must be 0-7)."
    assemble cmd
    exit

dev_ok:
    match [:dst_idx:], dst
    jyes dst_ok
    arrange cmd, =err "Destination must be a register."
    assemble cmd
    exit

dst_ok:
    check dst_idx = 0
    jno final

    arrange cmd, =err "Cannot modify r0."
    assemble cmd
    exit

final:
    emit 1, 60h or dev
    emit 1, dst_idx shl 4
end calminstruction

calminstruction out? dev, src
    local src_idx, cmd
    transform src

    check dev > 7
    jno dev_ok

    arrange cmd, =err "Invalid IO device ID (Must be 0-7)."
    assemble cmd
    exit

dev_ok:
    match [:src_idx:], src
    jyes is_reg

    ; SOURCE IS IMMEDIATE
    emit 1, 70h or 1000b or dev
    emit 1, 0
    emit 2, src bswap 2
    exit

is_reg:
    ; SOURCE IS REGISTER
    emit 1, 70h or dev
    emit 1, src_idx

end calminstruction

; RETURNS
calminstruction ret?
    emit 1, $B0
    emit 1, 0
end calminstruction

calminstruction iret?
    emit 1, $B1
    emit 1, 0
end calminstruction
