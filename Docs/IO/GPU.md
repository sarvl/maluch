# Overview

The CPU-GPU communication happens in two ways
1. Memory Mapped VRAM  
2. IO port 1   

Each is described in corresponding section


# VRAM

Writing to memory with adress in the range 0x0000 to 0x7FFF (inclusive) sends character to the GPU.

## Address

The GPU may completely ignore the address and write data to the screen sequentially. 
If the GPU does NOT ignore the address, it has to divide it into 2 parts high [14:8] and low [7:0] \(note that 15th bit - the most significant one - is always 0).
The high part is responsible for choosing the row, while the low part is responsible for choosing the column. 
This provides easy way for processor to implement newline - simply increment the address by 0x0100 and AND it with 0x7F00.

## Data

Only lowest 8 bits of data are interpreted by the GPU, they use the IBM code page 437 charset.  

# IO port

IO port is used to read GPU interface type and to write commands.

## Reading
`in 1, Reg` must always return GPU type, currently there are only 2 types - depending on whether color is supported.

## GPU Type
0 - invalid  
1 - no color (black and white)
2 - color 

## Commands

High byte of the command is responsible for command type, while low one is responsible for data supplied to the command.

```
x00YY - invalid
x01YY - invalid
x02YY - set foreground color to YY (supported only by GPU type 2, ignored by GPU type 1)
x03YY - set background color to YY (supported only by GPU type 2, ignored by GPU type 1)
x04YY - clear the screen (replace all characters with whitespace)
x05YY - clear the YYth line (replace all characters with whitespace)
```

# Display

## Color
Regardless of GPU type, the color must by default be set to white foreground and black background
