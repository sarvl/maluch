# PISS16 Instruction Set Architecture

## Overview

PISS16 is a 16bit architecture designed for educational purposes. It focuses on easily decodable CPU instructions and simple hardware implementations while avoiding significant software limitations. 

## TODO
1. interrupts
2. toc
3. proofreading

### Key Features
- *16-bit Data and Address Width*
- *Variable-length instructions* (16-bit or 32-bit)
- *64KiB Addressable Memory*
- *Simple instruction format* for decoding simplicity
- *Educational focus* with clear, logical instruction organization

## Table of contents


<a name="instructions"></a>
## Instruction Overview

|Instruction|Opcode|Funct|Behavior|
|:---:|:---:|:---:|:---|
|invalid | 0000 | XXX | will always be invalid |
|    |     |     |     |
|mov Rd src | 0001 | XXX | Rd <-- src|
|    |     |     |     |
|add Rd src | 0010 | 000 | Rd <-- Rd + src (sets flags)|
|sub Rd src | 0010 | 001 | Rd <-- Rd - src (sets flags)|
|and Rd src | 0010 | 010 | Rd <-- Rd & src (sets flags)|
|or Rd src | 0010 | 011 | Rd <-- Rd \| src (sets flags)|
|xor Rd src | 0010 | 100 | Rd <-- Rd ^ src (sets flags)|
|not Rd src | 0010 | 101 | Rd <-- ~src (sets flags)|
|lsl Rd src | 0010 | 110 | Rd <-- Rd << src (logical shift left)  (sets flags)|
|lsr Rd src | 0010 | 111 | Rd <-- Rd >> src (logical shift right)  (sets flags)|
|    |     |     |     |
|           | 0011 | 001 | reserved |
|cmp Rd src | 0011 | 001 | Rd <-- Rd - src (sets flags)|
|test Rd src | 0011 | 010 | Rd & src (sets flags)|
|          | 0011 | 011 | reserved |
|           | 0011 | 100 | reserved |
|           | 0011 | 101 | reserved |
|           | 0011 | 110 | reserved |
|           | 0011 | 111 | reserved |
|    |     |     |     |
|jmp src | 0100 | 000 | IP <-- src (unconditional jump)|
|bee src | 0100 | 001 | if(FZ = 1) IP <-- src (branch if equal) |
|bne src | 0100 | 010 | if(FZ = 0) IP <-- src (branch if not equal) |
|bge src | 0100 | 011 | if(FS = FO) IP <-- src (branch if greater or equal (signed)) |
|ble src | 0100 | 100 | if(FZ = 1 or FS /= FO) IP <-- src (branch if less or equal (signed)) |
|bgg src | 0100 | 101 | if(FZ = 0 and FS = FO) IP <-- src (branch if greater (signed)) |
|bll src | 0100 | 110 | if(FS /= FO) IP <-- src (branch if less (signed)) |
|boo src | 0100 | 111 | if(FO = 1) IP <-- src (branch if overflow)
|bbs src | 0101 | 000 | if(busy flags /= x0000) IP <-- src (branch if busy)|
|bss src | 0101 | 001 | if(FS = 1) IP <-- src (branch if sign) |
|bns src | 0101 | 010 | if(FS = 0) IP <-- src (branch if not sign) |
|bae src | 0101 | 011 | if(FC = 0) IP <-- src (branch if above or equal (unsigned)) |
|bbe src | 0101 | 100 | if(FC = 1 or ZF = 1) IP <-- src (branch if below or equal (unsigned)) |
|baa src | 0101 | 101 | if(FC = 0 and FZ = 0) IP <-- src (branch if above (unsigned)) |
|bbb src | 0101 | 110 | if(FC = 1) IP <-- src (branch if below (unsigned)) |
|boo src | 0101 | 111 | if(FO = 0) IP <-- src (branch if no overflow)
|    | |     |     |
| in Rd | 0110 | fff  | Rd <-- IO[fff] |
| out Rd | 0111 | fff  | IO[fff] <--- Rd |
| ldw Rd src | 1000 | XXX | Rd <-- MEM[src] |
| stw Rd src | 1001 | XXX | MEM[src] <-- Rd |
| call src | 1010 | XXX | IP <-- src ; MEM[SP - 2] <-- IP ; SP <-- SP - 2|
| ret | 1011 | XXX | IP <-- MEM[SP] ; SP <-- SP + 2|
| push src | 1100 | XXX | MEM[SP - 2] <-- src ; SP <-- SP - 2  |   
| pull Rd | 1101 | XXX | Rd <-- MEM[SP] ; SP <-- SP + 2  |   
|    | 1110 |     | reserved |
|  | 1111 |     | |


note: 
- `src` may mean register or immediate, depending on the encoding, see [Encoding](encoding)
- `XXX` should be set to 0 for future compatibility

<a name="encoding"></a>
## Encoding

all instructions follow the same format   
`OOOOJFFF'DDDDSSSS IIIIIIII'IIIIIIII`  
    
    O - main opcode  
    J - is immediate  
    F - funct field  
    D - destination register  
    S - source register  
    I - immediate field  

if J = 1 then second operand is immediate and S is ignored 
 `OOOO1FFF'DDDD____ IIIIIIII'IIIIIIII`  

if J = 0 then second operand is register and I field **is not present**  
 `OOOO0FFF'DDDDSSSS`  


<a name="registers"></a>
## Registers

PISS16 provides 16 registers, each 16 bits wide:

| Register | Name | Usage | Access |
|:--------:|:----:|:------:|:------:|
| x0 | CR0 | Control Register 0 | [Special](#control-register-0) |
| x1 | CR1 | Control Register 1 | [Special](#control-register-1) |
| x2 | SP | Stack Pointer | [General](#stack-pointer) |
| x3-x15 | - | General purpose registers | General |

**General** means that a register can be used in any instruction requiring register, few are implicitly used by other instructions
- *General Purpose Registers (GPRs)*: x1-x15 can be used in any instruction requiring a register.


<a name="control-register-0"></a>
### Control Register 0 (x0) Details

This register is divided into two 8-bit sections:

```
 15            8 7               0
┌───────────────┬────────────────┐
│   int flags   │   busy flags   │
└───────────────┴────────────────┘
```

- *Bits 15-8*: int flags, read only, set by hardware, info which interrupt occured (from left to right)
- *Bits 7-0*: busy flags, read only, set by hardware, info whether IO device is processing request

Each flag corresponds to a specific interrupt source. The mask bits (CR1) control whether the corresponding interrupt is enabled (1) or disabled (0).
Write to flags part is ignored

see [IO](IO)

<a name="control-register1"></a>
### Control Register 1 (x1) Details

This register is divided into two 8-bit sections:

```
 15             8 7               0
┌────────────────┬────────────────┐
│    int mask    │    reserved    │
└────────────────┴────────────────┘
```

- *Bits 15-8*: int mask, if a bit is `1` then corresponding bit in CR0 CAN cause interrupt, to disable an interrupt write `0`
- *Bits 7-0*: reserved, leave at 0

Each flag corresponds to a specific interrupt source. The mask bits (CR1) control whether the corresponding interrupt is enabled (1) or disabled (0).
Write to flags part is ignored

see [IO](IO)

<!-- TOC --><a name="stack-pointer"></a>
### Stack Pointer (x2) Details
*Stack Pointer (x1)* is automatically managed by stack operations (CALL/RET/PUSH/PULL). Can also be used by any instruction requiring GPR.


<!-- TOC --><a name="io"></a>
### IO

IO is based on 2 mechanisms:
- polling
- interrupts 


<!-- TOC --><a name="polling"></a>
#### polling

Communication is initiated by the CPU via `in` and `out` instructions, these instructions can only be executed when IO device has its busy flag set to 0, otherwise the behavior is undefined

<!-- TOC --><a name="interrupts"></a>
#### Interrupts

TODO


#### Interrupt Handler

<!-- TOC --><a name="processor-flags"></a>
## Processor Flags

The processor maintains four condition flags that are automatically updated by arithmetic and logical operations:

| Flag | Name | Set When | Defined For |
|:----:|:----:|:---------|:-----------|
| FS | Sign | Result bit 15 = 1 | All flag-modifying instructions |
| FO | Overflow | Carry out of MSb ≠ carry into MSb | ADD, SUB, CMP |
| FC | Carry | Carry out of MSb = 1 | ADD, SUB, CMP |
| FZ | Zero | All result bits = 0 | All flag-modifying instructions |

<!-- TOC --><a name="Detailed Instruction Description"></a>
## Detailed Instruction Description

<!-- TOC --><a name="instruction mov"></a>
### MOV
- instruction: mov Rd src 
- opcode: 0001 
- funct: XXX 
- flags: unmodified
- description: copies src into Rd

<!-- TOC --><a name="instruction add"></a>
### ADD
- instruction: add Rd src 
- opcode: 0010 
- funct: 000 
- flags: modified
- description: adds src to Rd, stores result into Rd

<!-- TOC --><a name="instruction sub"></a>
### SUB
- instruction: sub Rd src 
- opcode: 0010 
- funct: 001 
- flags: modified
- description: subs src from Rd, stores result into Rd

<!-- TOC --><a name="instruction and"></a>
### AND
- instruction: and Rd src 
- opcode: 0010 
- funct: 010 
- flags: modified
- description: ands src with Rd, stores result into Rd

<!-- TOC --><a name="instruction or"></a>
### OR
- instruction: or Rd src 
- opcode: 0010 
- funct: 011 
- flags: modified
- description: ors src with Rd, stores result into Rd

<!-- TOC --><a name="instruction xor"></a>
### XOR
- instruction: xor Rd src 
- opcode: 0010 
- funct: 100 
- flags: modified
- description: xors src with Rd, stores result into Rd

<!-- TOC --><a name="instruction not"></a>
### NOT
- instruction: not Rd src 
- opcode: 0010 
- funct: 101 
- flags: modified
- description: nots src, stores result into Rd

<!-- TOC --><a name="instruction lsl"></a>
### XOR
- instruction: lsl Rd src 
- opcode: 0010 
- funct: 110 
- flags: modified
- description: logically shifts Rd by src (lower 4 bits) to the left, stores result into Rd

<!-- TOC --><a name="instruction lsr"></a>
### XOR
- instruction: lsr Rd src 
- opcode: 0010 
- funct: 111 
- flags: modified
- description: logically shifts Rd by src (lower 4 bits) to the right, stores result into Rd

<!-- TOC --><a name="instruction cmp"></a>
### CMP
- instruction: CMP Rd src 
- opcode: 0010 
- funct: 001 
- flags: modified
- description: subs src from Rd

<!-- TOC --><a name="instruction test"></a>
### TEST
- instruction: test Rd src 
- opcode: 0010 
- funct: 010 
- flags: modified
- description: ands src with Rd

<!-- TOC --><a name="instruction jmp"></a>
### JMP 
- instruction: jmp src 
- opcode: 0100 
- funct: 000 
- flags: unmodified
- description: IP <-- src (unconditional jump)

<!-- TOC --><a name="instruction bee"></a>
### BEE 
- instruction: bee src 
- opcode 0100 
- funct: 001 
- flags: unmodified
- description: if(FZ = 1) IP <-- src (branch if equal) 

<!-- TOC --><a name="instruction bne"></a>
### BNE 
- instruction: bne src 
- opcode 0100 
- funct: 010 
- flags: unmodified
- description: if(FZ = 0) IP <-- src (branch if not equal) 

<!-- TOC --><a name="instruction bge"></a>
### BGE 
- instruction: bge src 
- opcode 0100 
- funct: 011 
- flags: unmodified
- description: if(FS = FO) IP <-- src (branch if greater or equal (signed)) 

<!-- TOC --><a name="instruction ble"></a>
### BLE 
- instruction: ble src 
- opcode 0100 
- funct: 100 
- flags: unmodified
- description: if(FZ = 1 or FS /= FO) IP <-- src (branch if less or equal (signed)) 

<!-- TOC --><a name="instruction bgg"></a>
### BGG 
- instruction: bgg src 
- opcode 0100 
- funct: 101 
- flags: unmodified
- description: if(FZ = 0 and FS = FO) IP <-- src (branch if greater (signed)) 

<!-- TOC --><a name="instruction bll"></a>
### BLL 
- instruction: bll src 
- opcode 0100 
- funct: 110 
- flags: unmodified
- description: if(FS /= FO) IP <-- src (branch if less (signed)) 

<!-- TOC --><a name="instruction boo"></a>
### BOO 
- instruction: boo src 
- opcode 0100 
- funct: 111 
- flags: unmodified
- description: if(FO = 1) IP <-- src (branch if overflow)

<!-- TOC --><a name="instruction bbs"></a>
### BBS 
- instruction: bbs src 
- opcode 0101 
- funct: 000 
- flags: unmodified
- description: if(busy flags /= x0000) IP <-- src (branch if busy)

<!-- TOC --><a name="instruction bss"></a>
### BSS 
- instruction: bss src 
- opcode 0101 
- funct: 001 
- flags: unmodified
- description: if(FS = 1) IP <-- src (branch if sign) 

<!-- TOC --><a name="instruction bns"></a>
### BNS 
- instruction: bns src 
- opcode 0101 
- funct: 010 
- flags: unmodified
- description: if(FS = 0) IP <-- src (branch if not sign) 

<!-- TOC --><a name="instruction bae"></a>
### BAE 
- instruction: bae src 
- opcode 0101 
- funct: 011 
- flags: unmodified
- description: if(FC = 0) IP <-- src (branch if above or equal (unsigned)) 

<!-- TOC --><a name="instruction bbe"></a>
### BBE 
- instruction: bbe src 
- opcode 0101 
- funct: 100 
- flags: unmodified
- description: if(FC = 1 or ZF = 1) IP <-- src (branch if below or equal (unsigned)) 

<!-- TOC --><a name="instruction baa"></a>
### BAA 
- instruction: baa src 
- opcode 0101 
- funct: 101 
- flags: unmodified
- description: if(FC = 0 and FZ = 0) IP <-- src (branch if above (unsigned)) 

<!-- TOC --><a name="instruction bbb"></a>
### BBB 
- instruction: bbb src 
- opcode 0101 
- funct: 110 
- flags: unmodified
- description: if(FC = 1) IP <-- src (branch if below (unsigned)) 

<!-- TOC --><a name="instruction boo"></a>
### BOO 
- instruction: boo src 
- opcode 0101 
- funct: 111 
- flags: unmodified
- description: if(FO = 0) IP <-- src (branch if no overflow)

<!-- TOC --><a name="instruction in"></a>
### IN 
- instruction: in Rd 
- opcode 0110 
- funct: fff  
- flags: unmodified
- description: Rd <-- IO[fff] 

<!-- TOC --><a name="instruction out"></a>
### OUT 
- instruction: out Rd 
- opcode 0111 
- funct: fff  
- flags: unmodified
- description: IO[fff] <--- Rd 

<!-- TOC --><a name="instruction ldw"></a>
### LDW 
- instruction: ldw Rd src 
- opcode 1000 
- funct: XXX 
- flags: unmodified
- description: Rd <-- MEM[src] 

<!-- TOC --><a name="instruction stw"></a>
### STW 
- instruction: stw Rd src 
- opcode 1001 
- funct: XXX 
- flags: unmodified
- description: MEM[src] <-- Rd 

<!-- TOC --><a name="instruction call"></a>
### CALL 
- instruction: call src 
- opcode 1010 
- funct: XXX 
- flags: unmodified
- description: IP <-- src ; MEM[SP - 2] <-- IP ; SP <-- SP - 2

<!-- TOC --><a name="instruction ret"></a>
### RET 
- instruction: ret 
- opcode 1011 
- funct: XXX 
- flags: unmodified
- description: IP <-- MEM[SP] ; SP <-- SP + 2

<!-- TOC --><a name="instruction push"></a>
### PUSH 
- instruction: push src 
- opcode 1100 
- funct: XXX 
- flags: unmodified
- description: MEM[SP - 2] <-- src ; SP <-- SP - 2  

<!-- TOC --><a name="instruction pull"></a>
### PULL 
- instruction: pull Rd 
- opcode 1101 
- funct: XXX 
- flags: unmodified
- description: Rd <-- MEM[SP] ; SP <-- SP + 2  