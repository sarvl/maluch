; vim:ft=fasm

repeat 16, i:0
  r#i? equ [:i:]
end repeat

board? equ 000b
kb?    equ 001b
gpu?   equ 010b
pstor? equ 011b

;; DYADIC
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

; IO
macro in? dev, dst
    if dev > 7
        err "Invalid IO device ID."
    end if

    match [:dst_idx:], dst
        if dst_idx = 0
            err "Cannot modify R0."
        end if
        db $60 or dev
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
        db $70 or dev
        db 0
        db src_idx
    else
        db $70 or 1000b or dev
        dw src bswap 2
    end match
end macro
