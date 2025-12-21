; vim:ft=fasm

repeat 16, i:0
  r#i? equ [:i:]
end repeat

timer? equ 000b
kb?    equ 001b
gpu?   equ 010b
pstor? equ 011b

; DYADIC
irp <name,op>, mov,10h, add,20h, sub,21h, \
  and,22h, or,23h, xor,24h, not,25h, lsl,26h, \
  lsr,27h, cmp,31h, test,32h, ldw,80h, stw,90h
    macro name? dst,src
        match [:dst_idx:], dst
            if dst_idx = 0
                err "Cannot modify R0."
            end if
    
            match [:src_idx:], src
                db op
                db (dst_idx shl 4) or src_idx
            else
                db op or 1000b
                db dst_idx shl 4
                dw src bswap 2
            end match
        else
            err "Invalid destination."
        end match
    end macro
end irp

; JUMPS
irp <name,op>, jmp,40h, bee,41h, bne,42h, \
  bge,43h, ble,44h, bgg,45h, bll,46h, boo,47h, \
  bbs,50h, bss,51h, bns,52h, bae,53h, bbe,54h, \
  baa,55h, bbb,56h, bno,57h, call,$A0
    calminstruction name? src
        transform src
        match [:src_idx:], src
        jyes short_jump

        emit 1, op or 1000b
        emit 1, 0

        local tmp
        match tmp:number, src, : 
        jyes is_number
        
        ; CASE: LABEL
        compute src, src shr 1
        
is_number:
        ; CASE: NUMBER
        emit 2, src bswap 2
        exit

short_jump:
        ; CASE: SHORT INDEX
        emit 1, op
        emit 1, src_idx
    end calminstruction

end irp

; STACK
macro push? src
    match [:src_idx:], src
        db $C0
        db src_idx
    else
        db $C0 or 1000b
        db 0
        dw src bswap 2
    end match
end macro

macro pull? dst
    match [:dst_idx:], dst
        if dst_idx = 0
            err "Cannot modify R0."
        end if
        db $D0
        db dst_idx shl 4
    else
        err "Invalid destination."
    end match
end macro

; IO
macro in? dev, dst
    if dev > 7
        err "Invalid IO device ID."
    end if

    match [:dst_idx:], dst
        if dst_idx = 0
            err "Cannot modify R0."
        end if
        db 60h or dev
        db dst_idx shl 4
    else
        err "Invalid destination."
    end match
end macro

macro out? dev, src
    if dev > 7
        err "Invalid IO device ID."
    end if

    match [:src_idx:], src
        db 70h or dev
        db src_idx
    else
        db 70h or 1000b or dev
        db 0
        dw src bswap 2
    end match
end macro
