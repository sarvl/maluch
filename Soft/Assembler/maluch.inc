; vim:ft=fasm

repeat 16, i:0
  r#i? equ [:i:]
end repeat

macro mov? dst,src
    match [:dst_idx:], dst
        if dst_idx = 0
            err "Cannot MOV into R0."
        end if

        match [:src_idx:], src
            db 00010000b
            db (dst_idx shl 4) or src_idx
        else
            db 00011000b
            db dst_idx shl 4
            dw src bswap 2
        end match
    else
        err "Incorrect MOV Target."
    end match
end macro
