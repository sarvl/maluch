# PISS16 Instruction Set Architecture

## Overview

PISS16 is a 16bit architecture designed for educational purposes. It focuses on easily decodable CPU instructions and simple hardware implementations while avoiding significant software limitations. 

### Key Features
- *16-bit Data and Address Width*
- *Variable-length instructions* (16-bit or 32-bit)
- *128kiB (64kiW) Word Addressable Memory*
- *Simple instruction format* for decoding simplicity
- *Educational focus* with clear, logical instruction organization
- *LSBian machine*

## Table of contents

1. [Terminology](#terminology)
2. [Instructions Overview](#instructions-overview)
3. [Encoding](#encoding)
4. [Memory](#memory)
5. [Registers](#registers)
6. [IO](#io)
7. [Flags](#flags)
8. [Detailed Instruction Description](#detailed-instruction-description)

## Terminology
word - 16bit, base length on which ISA operates

## Instructions Overview

|Instruction|Opcode|Funct|Behavior|
|:---:|:---:|:---:|:---|
|invalid | 0000 | XXX | will always be invalid |
|mov Rd src | 0001 | XXX | Rd <-- src|
|add Rd src | 0010 | 000 | Rd <-- Rd + src (sets flags)|
|sub Rd src | 0010 | 001 | Rd <-- Rd - src (sets flags)|
|and Rd src | 0010 | 010 | Rd <-- Rd & src (sets flags)|
|or Rd src | 0010 | 011 | Rd <-- Rd \| src (sets flags)|
|xor Rd src | 0010 | 100 | Rd <-- Rd ^ src (sets flags)|
|not Rd src | 0010 | 101 | Rd <-- ~src (sets flags)|
|lsl Rd src | 0010 | 110 | Rd <-- logical\_shift\_left(Rd, src mod 16) (sets flags)|
|lsr Rd src | 0010 | 111 | Rd <-- logical\_shift\_right(Rd, src mod 16) (sets flags)|
|           | 0011 | 001 | reserved |
|cmp Rd src | 0011 | 001 | Rd <-- Rd - src (sets flags)|
|test Rd src | 0011 | 010 | Rd & src (sets flags)|
|          | 0011 | 011 | reserved |
|           | 0011 | 100 | reserved |
|           | 0011 | 101 | reserved |
|           | 0011 | 110 | reserved |
|           | 0011 | 111 | reserved |
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
|bno src | 0101 | 111 | if(FO = 0) IP <-- src (branch if no overflow)
|in fff Rd | 0110 | fff  | Rd <-- IO[fff] |
|out fff src | 0111 | fff  | IO[fff] <-- src |
|ldw Rd src | 1000 | XXX | Rd <-- MEM[src] |
|stw Rd src | 1001 | XXX | MEM[src] <-- Rd |
|call src | 1010 | XXX | MEM[SP - 1] <-- IP ; SP <-- SP - 1 ; IP <-- src |
|ret | 1011 | 000 | IP <-- MEM[SP] ; SP <-- SP + 1|
|iret | 1011 | 001 | IP <-- MEM[SP] ; SP <-- SP + 1 ; turns on interrupts |
|     | 1011 | 010 | reserved |
|     | 1011 | 011 | reserved |
|     | 1011 | 100 | reserved |
|     | 1011 | 101 | reserved |
|     | 1011 | 110 | reserved |
|     | 1011 | 111 | reserved |
|push src | 1100 | XXX | MEM[SP - 1] <-- src ; SP <-- SP - 1  |   
|pull Rd | 1101 | XXX | Rd <-- MEM[SP] ; SP <-- SP + 1  |   
|   | 1110 |     | reserved |
|   | 1111 |     | reserved |


note: 
- `src` may mean register or immediate, depending on the encoding, see [Encoding](encoding)
- `XXX` should be set to 0 for future compatibility

## Encoding

all instructions follow the same format   
`OOOOJFFF'DDDDSSSS IIIIIIII'IIIIIIII`  
    
    O - main opcode  
    J - is immediate  
    F - funct field  
    D - destination register  
    S - source register  
    I - immediate field  

if J = 1 then second operand is immediate and S is ignored, the instruction length is two words  
 `OOOO1FFF'DDDD____ IIIIIIII'IIIIIIII`  

if J = 0 then second operand is register and I field **is not present**, the instruction length is one word  
 `OOOO0FFF'DDDDSSSS`  


## Memory

### General

There are 128kiB of memory addressable, 64kiB are general purpose (see address map below).

The memory is word addressable, it is not possible to access only a single byte.

### Address Map

```
0x0000 to 0x7FFF:
    writes - video memory
    reads  - instruction memory
0x8000 to 0xFFFF:
    general purpose memory
```

instruction memory is dedicated to hold some basic procedures used by the processor, however instructions can be located anywhere in readable memory

video memory is write only memory from which video card reads data to be displayed
the exact data that video card expects depends on that video card and is therefore not specified here

## Registers

PISS16 provides 16 registers, each 16 bits wide:

| Register | Name | Usage | Access |
|:--------:|:----:|:------:|:------:|
| x0 | CR0 | Control Register 0 | [Special](#control-register-0) |
| x1 | CR1 | Control Register 1 | [Special](#control-register-1) |
| x2 | SP | Stack Pointer | [General](#stack-pointer) |
| x3-x15 | - | General purpose registers | General |

**General** means that a register can be used in any instruction requiring register and will behave as expected
stack pointer (x2) is implicitly used by `push` `pull` `call` `ret` 


### Control Register 0 

This register is divided into two 8-bit sections:

```
 15            8 7               0
┌───────────────┬────────────────┐
│   int flags   │   busy flags   │
└───────────────┴────────────────┘
```

- *Bits 15-8*: int flags, read only, set by hardware, info which interrupt occured (from left to right)
- *Bits 7-0*: busy flags, read only, set by hardware, info whether IO device is processing request

Each flag corresponds to a specific interrupt source. The mask bits (in CR1) control whether the corresponding interrupt is enabled (1) or disabled (0).
Write is ignored.

see [IO](IO)

### Control Register 1 

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

see [IO](IO)

### Stack Pointer 
*Stack Pointer (x1)* is automatically managed by stack operations (CALL/RET/PUSH/PULL). Can also be used by any instruction requiring GPR.


### IO

IO is based on 2 mechanisms:
- polling
- interrupts 

#### polling

Communication is initiated by the processor via `in` and `out` instructions, these instructions can only be executed when IO device has its busy flag set to 0, otherwise the behavior is undefined.  
Whenever processor executes `in` or `out` a device writes a value to or reads a value from specified register (respectively).  
The exact behavior depends on the device but there is **no** delay from these instructions, any delay must happen *before* or *after* them via busy flag.  

#### Interrupts

Interrupt happens ALWAYS AFTER the current instruction, IRET can be followed by the interrupt, the next IRET with no following interrupt will properly return to IP that first IRET was supposed to return to.  

When a device signals readiness to a processor, it sets an interrupt, this interrupt appears as a flag in CR0 (the bit position from the left - most significant one - indicates the device id, count starts from 0). 

IF `interrupt_flag[device_id] AND interrupt_mask[device_id] = 1` THEN an interrupt takes place, in case more than one interrupt were to occur, the leftmost one (lowest ID) takes priority.  

On interrupt:
- IP is pushed to the stack, THEN it is changed to the proper address (see IHT below)
- int mask is saved internally, THEN it is changed to x00
- flags are saved internally

On IRET:
- IP is pulled from the stack
- int mask is restored 
- flags are restored

importantly, only int MASK is affected, to disable interrupt service the IO that caused the interrupt

If flags are not a concern then it is not necessary to execute IRET to return from interrupt, altough it is usually not a correct action due to flag behavior.  


#### Interrupt Handler Table

An 8 entry table is located in RAM at addresses xFFF0 to xFFFF, each entry is 2 Words - enough to store `JMP immediate`. 
On interrupt with id N, IP switches to address `xFFF0 + N * 2`, this address then contains jump to actual Interrupt Handler Routine.
An interrupt handling routine may be located anywhere in readable memory.

#### Interrupt Handler Routine

The Instruction Pointer is automatically handled by HW, all other registers are NOT SAVED, therefore it is the job of programmer to ensure proper register saving via PUSH and PULL.
The interrupt information is received by using `in` with appropriate device id. Device stops signaling interrupt when **it** decides that it is handled, usually that means reading from it via `in`.

Generally IHR should end with IRET.

#### IO device List

| id | device |
| :---: | :---: |
| 000 | timer |
| 001 | keyboard |
| 010 | gpu |
| 011 | persistent storage |
| 100 | reserved |
| 101 | reserved |
| 110 | reserved |
| 111 | reserved |


For more detailed description of behavior see corresponding manual.

## Flags

The processor maintains four condition flags that are automatically updated by arithmetic and logical operations:

| Flag | Name | Set When | Defined For |
|:----:|:----:|:---------|:-----------|
| FS | Sign | Result bit 15 = 1 | All flag-modifying instructions |
| FO | Overflow | Carry out of MSb ≠ carry into MSb | ADD, SUB, CMP |
| FC | Carry | Carry out of MSb = 1 | ADD, SUB, CMP |
| FZ | Zero | All result bits = 0 | All flag-modifying instructions |

even though a flag may be undefined for given instruction, it is always modified.  

## Detailed Instruction Description

### MOV
- instruction: mov Rd src 
- opcode: 0001 
- funct: XXX 
- flags: unmodified
- description: copies src into Rd

### ADD
- instruction: add Rd src 
- opcode: 0010 
- funct: 000 
- flags: modified
- description: adds src to Rd, stores result into Rd

### SUB
- instruction: sub Rd src 
- opcode: 0010 
- funct: 001 
- flags: modified
- description: subtracts src from Rd, stores result into Rd

### AND
- instruction: and Rd src 
- opcode: 0010 
- funct: 010 
- flags: modified
- description: performs bitwise logical AND on src with Rd, stores result into Rd

### OR
- instruction: or Rd src 
- opcode: 0010 
- funct: 011 
- flags: modified
- description: performs bitwise logical OR on src with Rd, stores result into Rd

### XOR
- instruction: xor Rd src 
- opcode: 0010 
- funct: 100 
- flags: modified
- description: performs bitwise logical XOR on src with Rd, stores result into Rd

### NOT
- instruction: not Rd src 
- opcode: 0010 
- funct: 101 
- flags: modified
- description: performs bitwise logical NOT on src, stores result into Rd

### LSL
- instruction: lsl Rd src 
- opcode: 0010 
- funct: 110 
- flags: modified
- description: logically shifts Rd by src (lower 4 bits, upper 12 bits are ignored) to the left, stores result into Rd

### LSR
- instruction: lsr Rd src 
- opcode: 0010 
- funct: 111 
- flags: modified
- description: logically shifts Rd by src (lower 4 bits, upper 12 bits are ignored) to the right, stores result into Rd

### CMP
- instruction: CMP Rd src 
- opcode: 0010 
- funct: 001 
- flags: modified
- description: exactly like SUB, but does not affect Rd

### TEST
- instruction: test Rd src 
- opcode: 0010 
- funct: 010 
- flags: modified
- description: exactly like AND, but does not affect Rd

### JMP 
- instruction: jmp src 
- opcode: 0100 
- funct: 000 
- flags: unmodified
- description: IP <-- src (unconditional jump)

### BEE 
- instruction: bee src 
- opcode 0100 
- funct: 001 
- flags: unmodified
- description: if(FZ = 1) IP <-- src (branch if equal) 

### BNE 
- instruction: bne src 
- opcode 0100 
- funct: 010 
- flags: unmodified
- description: if(FZ = 0) IP <-- src (branch if not equal) 

### BGE 
- instruction: bge src 
- opcode 0100 
- funct: 011 
- flags: unmodified
- description: if(FS = FO) IP <-- src (branch if greater or equal (signed)) 

### BLE 
- instruction: ble src 
- opcode 0100 
- funct: 100 
- flags: unmodified
- description: if(FZ = 1 or FS /= FO) IP <-- src (branch if less or equal (signed)) 

### BGG 
- instruction: bgg src 
- opcode 0100 
- funct: 101 
- flags: unmodified
- description: if(FZ = 0 and FS = FO) IP <-- src (branch if greater (signed)) 

### BLL 
- instruction: bll src 
- opcode 0100 
- funct: 110 
- flags: unmodified
- description: if(FS /= FO) IP <-- src (branch if less (signed)) 

### BOO 
- instruction: boo src 
- opcode 0100 
- funct: 111 
- flags: unmodified
- description: if(FO = 1) IP <-- src (branch if overflow)

### BBS 
- instruction: bbs src 
- opcode 0101 
- funct: 000 
- flags: unmodified
- description: if(busy flags /= x0000) IP <-- src (branch if busy)

### BSS 
- instruction: bss src 
- opcode 0101 
- funct: 001 
- flags: unmodified
- description: if(FS = 1) IP <-- src (branch if sign) 

### BNS 
- instruction: bns src 
- opcode 0101 
- funct: 010 
- flags: unmodified
- description: if(FS = 0) IP <-- src (branch if not sign) 

### BAE 
- instruction: bae src 
- opcode 0101 
- funct: 011 
- flags: unmodified
- description: if(FC = 0) IP <-- src (branch if above or equal (unsigned)) 

### BBE 
- instruction: bbe src 
- opcode 0101 
- funct: 100 
- flags: unmodified
- description: if(FC = 1 or ZF = 1) IP <-- src (branch if below or equal (unsigned)) 

### BAA 
- instruction: baa src 
- opcode 0101 
- funct: 101 
- flags: unmodified
- description: if(FC = 0 and FZ = 0) IP <-- src (branch if above (unsigned)) 

### BBB 
- instruction: bbb src 
- opcode 0101 
- funct: 110 
- flags: unmodified
- description: if(FC = 1) IP <-- src (branch if below (unsigned)) 

### BOO 
- instruction: bno src 
- opcode 0101 
- funct: 111 
- flags: unmodified
- description: if(FO = 0) IP <-- src (branch if no overflow)

### IN 
- instruction: in Rd 
- opcode 0110 
- funct: fff  
- flags: unmodified
- description: Rd <-- IO[fff] 

### OUT 
- instruction: out Rd 
- opcode 0111 
- funct: fff  
- flags: unmodified
- description: IO[fff] <--- Rd 

### LDW 
- instruction: ldw Rd src 
- opcode 1000 
- funct: XXX 
- flags: unmodified
- description: Rd <-- MEM[src] 

### STW 
- instruction: stw Rd src 
- opcode 1001 
- funct: XXX 
- flags: unmodified
- description: MEM[src] <-- Rd 

### CALL 
- instruction: call src 
- opcode 1010 
- funct: XXX 
- flags: unmodified
- description: IP <-- src ; MEM[SP - 2] <-- IP ; SP <-- SP - 2

### RET 
- instruction: ret 
- opcode 1011 
- funct: 000 
- flags: unmodified
- description: IP <-- MEM[SP] ; SP <-- SP + 2

### IRET 
- instruction: ret 
- opcode 1011 
- funct: 001 
- flags: unmodified
- description: IP <-- MEM[SP] ; SP <-- SP + 2 ; restores int mask

### PUSH 
- instruction: push src 
- opcode 1100 
- funct: XXX 
- flags: unmodified
- description: MEM[SP - 2] <-- src ; SP <-- SP - 2  

### PULL 
- instruction: pull Rd 
- opcode 1101 
- funct: XXX 
- flags: unmodified
- description: Rd <-- MEM[SP] ; SP <-- SP + 2  