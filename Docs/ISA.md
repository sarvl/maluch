# PISS16 Instruction Set Architecture

## Overview

PISS16 is a custom instruction set architecture designed for educational purposes. It emphasizes easily decodable CPU instructions and simple hardware implementations while avoiding significant software limitations.

### Key Features

- *16-bit*
- *Variable-length instructions* (16-bit or 32-bit)
- *Simple instruction format* for decoding simplicity
- *Educational focus* with clear, logical instruction organization

## Table of content

<!-- TOC start (generated with https://github.com/derlin/bitdowntoc) -->

- [Architecture Overview](#architecture-overview)
   * [Word Size and Addressing](#word-size-and-addressing)
- [Register Set](#register-set)
   * [Register Usage Notes](#register-usage-notes)
   * [Interrupt Register (x0) Details](#interrupt-register)
- [Processor Flags](#processor-flags)
- [Instruction Format](#instruction-format)
   * [Field Definitions](#field-definitions)
- [ALU Operations](#alu-operations)
   * [Standalone ALU Instructions](#standalone-alu-instructions)
- [Instruction Set](#instruction-set)
   * [Operations Table](#operations-table)
   * [Data Movement Instructions](#data-movement-instructions)
      + [MOV - Move Data](#mov-move-data)
      + [LDW - Load Word](#ldw-load-word)
      + [SDW - Store Word](#sdw-store-word)
   * [Control Flow Instructions](#control-flow-instructions)
      + [JMP - Unconditional Jump](#jmp-unconditional-jump)
      + [BRE - Branch if Equal](#bre-branch-if-equal)
      + [BNE - Branch if Not Equal](#bne-branch-if-not-equal)
      + [BGT - Branch if Greater Than](#bgt-branch-if-greater-than)
      + [BLS - Branch if Less Than](#bls-branch-if-less-than)
      + [CALL - Call Subroutine](#call-call-subroutine)
      + [RET - Return from Subroutine](#ret-return-from-subroutine)
   * [Stack Operations](#stack-operations)
      + [PUSH - Push to Stack](#push-push-to-stack)
      + [PULL - Pull from Stack](#pull-pull-from-stack)
   * [System Instructions](#system-instructions)
      + [INT - Software Interrupt](#int-software-interrupt)

<!-- TOC end -->

<!-- TOC --><a name="architecture-overview"></a>
## Architecture Overview

<!-- TOC --><a name="word-size-and-addressing"></a>
### Word Size and Addressing

- *Data width*: 16 bits
- *Address width*: 16 bits
- *Addressable memory*: 64kiB (2^16 bytes)
- *Instruction alignment*: 16-bit boundaries

<!-- TOC --><a name="register-set"></a>
## Register Set

PISS16 provides 16 registers, each 16 bits wide:

| Register | Name | Usage | Access |
|:--------:|:----:|:------|:------:|
| x0 | INT | Interrupt flags and mask | [Special](#interrupt-register) |
| x1 | SP | Stack Pointer | [General](#stack-register) |
| x2 | IO | I/O buffer | [General](#iobuffer) |
| x3-x15 | - | General purpose registers | General |

<!-- TOC --><a name="register-usage-notes"></a>
### Register Usage Notes

- *General Purpose Registers (GPRs)*: x1-x15 can be used in any instruction requiring a register.
<!-- TOC --><a name="interrupt-register"></a>
### Interrupt Register (x0) Details

The interrupt register (x0) is divided into two 8-bit sections:

 15    12 11     8 7      4 3      0
┌───────┬────────┬────────┬────────┐
│ Flags          │  Mask           │
└───────┴────────┴────────┴────────┘

- *Bits 15-8*: Interrupt flags (read-only, set by hardware)
- *Bits 7-0*: Interrupt mask (write-only, controls which interrupts are enabled)

Each flag corresponds to a specific interrupt source. The mask bits control whether the corresponding interrupt is enabled (1) or disabled (0).
Write to flags part is ignored

<!-- TOC --><a name="stack-pointer"></a>
### Stack Pointer (x1) Details
*Stack Pointer (x1)* is automatically managed by stack operations (PUSH/PULL). Can also be used by any instruction requiring GPR.


<!-- TOC --><a name="io-buffer"></a>
### IO Buffer (x2) Details
*I/O Buffer (x2)*: Dedicated register for I/O operations - stores incoming data from I/O ports and holds outgoing data to be sent to I/O ports. Can be used by any instruction requiring GPR.


<!-- TOC --><a name="processor-flags"></a>
## Processor Flags

The processor maintains four condition flags that are automatically updated by arithmetic and logical operations:

| Flag | Name | Description | Set When | Defined For |
|:----:|:----:|:------------|:---------|:-----------|
| FS | Sign | Most significant bit is set | Result bit 15 = 1 | All flag-modifying instructions |
| FO | Overflow | Signed overflow occurred | Carry out of MSb ≠ carry into MSb | ADD, SUB, CMP, ADI |
| FC | Carry | Unsigned overflow occurred | Carry out of MSb = 1 | ADD, SUB, CMP, ADI |
| FZ | Zero | Result is zero | All result bits = 0 | All flag-modifying instructions |

<!-- TOC --><a name="instruction-format"></a>
## Instruction Format

All PISS16 instructions share a unified format for decoding simplicity:

Core Instruction (16 bits):
 15  12 11 10  9  8  7  4  3  0
┌─────┬──┬────┬──┬─────┬─────┐
│ OOO │a │AAA │i │XXXX │YYYY │
└─────┴──┴────┴──┴─────┴─────┘

Optional Immediate (16 bits):
 15                         0
┌─────────────────────────────┐
│ IIIIIIIIIIIIIIIIIIIIIIIIII  │
└─────────────────────────────┘

<!-- TOC --><a name="field-definitions"></a>
### Field Definitions

| Field | Bits | Description |
|:-----:|:----:|:------------|
| OOO | 15-13 | Operation code (8 possible operations) |
| a | 12 | ALU active flag (1 = use ALU result) |
| AAA | 11-9 | ALU operation code (8 ALU operations) |
| i | 8 | Immediate present flag (1 = 32-bit instruction) |
| XXXX | 7-4 | Register 1 (source/destination) |
| YYYY | 3-0 | Register 2 (source) |
| I | 31-16 | 16-bit immediate value (optional) |

<!-- TOC --><a name="alu-operations"></a>
## ALU Operations

The ALU performs operations on two source operands and produces a 16-bit result. 

<!-- TOC --><a name="standalone-alu-instructions"></a>
### Standalone ALU Instructions

ALU operations can be used as standalone instructions by setting the ALU active flag (a = 1) with the MOV instruction format:

MOV rX, rY    ; rX ← rY (a = 0)
ADD rX, rY    ; rX ← rX + rY (a = 1, ALU = 000)

| Code | Mnemonic | Operation | Description |
|:----:|:--------:|:---------:|:------------|
| 000 | ADD | s₁ + s₂ | Addition |
| 001 | SUB | s₁ - s₂ | Subtraction |
| 010 | AND | s₁ ∧ s₂ | Bitwise AND |
| 011 | OR | s₁ ∨ s₂ | Bitwise OR |
| 100 | XOR | s₁ ⊕ s₂ | Bitwise XOR |
| 101 | NOT | ~s₂ | Bitwise NOT (source 1 ignored) |
| 110 | LSR | s₁ >> 1 | Logical shift right |
| 111 | LSL | s₁ << 1 | Logical shift left |

Where s₁ = source 1, s₂ = source 2

<!-- TOC --><a name="instruction-set"></a>
## Instruction Set

<!-- TOC --><a name="operations-table"></a>
### Operations Table

|  | Operation | OP Code | ALU Code | Def | Def w/ Imm |
| ---: | :---: | :---: | :---: | :--- | :--- |
| 1 | MOV | 000 | X | $r_{1} \leftarrow r_{2}$ | $r_{1} \leftarrow I$ |
| 2 | LDW | 001 | X | $r_{1} \leftarrow \text{mem}[r_{2}]$ | $r_{1} \leftarrow \text{mem}[I]$ |
| 3 | SDW | 010 | X | $\text{mem}[r_{2}] \leftarrow r_{1}$ | $\text{mem}[I] \leftarrow r_{1}$ |
| 4 | JMP | 011 | 001 | $PC \leftarrow r_{1}$ | $PC \leftarrow I$ |
| 5 | BRE | 011 | 010 | $PC \leftarrow r_{1}$ | $PC \leftarrow I$ |
| 4.3 | BNE | 011 | 011 | $PC \leftarrow r_{1}$ | $PC \leftarrow I$ |
| 4.4 | BGT | 011 | 100 | $PC \leftarrow r_{1}$ | $PC \leftarrow I$ |
| 4.5 | BLS | 011 | 101 | $PC \leftarrow r_{1}$ | $PC \leftarrow I$ |
| 4.6 | CALL | 011 | 110 | $PC \leftarrow r_{1}; \text{stack} \leftarrow PC$ | $PC \leftarrow I; \text{stack} \leftarrow PC$ |
| 4.7 | RET | 011 | 111 | $PC \leftarrow \text{stack}$ | X |
| 5 | PUSH | 100 | X | $\text{stack} \leftarrow r_{1}$ | X |
| 6 | PULL | 101 | X | $r_{1} \leftarrow \text{stack}$ | X |
| 7 | INT | 110 | X |  |  |

<!-- TOC --><a name="data-movement-instructions"></a>
### Data Movement Instructions

<!-- TOC --><a name="mov-move-data"></a>
#### MOV - Move Data
MOV rX, rY    ; rX ← rY
MOV rX, #imm  ; rX ← immediate
- *Opcode*: 000
- *Flags*: Not affected
- *Description*: Transfers data between registers or loads immediate value

<!-- TOC --><a name="ldw-load-word"></a>
#### LDW - Load Word
LDW rX, rY    ; rX ← memory[rY]
LDW rX, #addr ; rX ← memory[addr]
- *Opcode*: 001
- *Flags*: Not affected
- *Description*: Loads 16-bit word from memory

<!-- TOC --><a name="sdw-store-word"></a>
#### SDW - Store Word
SDW rX, rY    ; memory[rY] ← rX
SDW rX, #addr ; memory[addr] ← rX
- *Opcode*: 010
- *Flags*: Not affected
- *Description*: Stores 16-bit word to memory

<!-- TOC --><a name="control-flow-instructions"></a>
### Control Flow Instructions

<!-- TOC --><a name="jmp-unconditional-jump"></a>
#### JMP - Unconditional Jump
JMP rX        ; PC ← rX
JMP #addr     ; PC ← addr
- *Opcode*: 011, *ALU Code*: 001
- *Flags*: Not affected
- *Description*: Unconditional branch to target address

<!-- TOC --><a name="bre-branch-if-equal"></a>
#### BRE - Branch if Equal
BRE rX        ; if (FZ = 1) PC ← rX
BRE #addr     ; if (FZ = 1) PC ← addr
- *Opcode*: 011, *ALU Code*: 010
- *Flags*: Not affected
- *Description*: Branch if zero flag is set

<!-- TOC --><a name="bne-branch-if-not-equal"></a>
#### BNE - Branch if Not Equal
BNE rX        ; if (FZ = 0) PC ← rX
BNE #addr     ; if (FZ = 0) PC ← addr
- *Opcode*: 011, *ALU Code*: 011
- *Flags*: Not affected
- *Description*: Branch if zero flag is clear

<!-- TOC --><a name="bgt-branch-if-greater-than"></a>
#### BGT - Branch if Greater Than
BGT rX        ; if (FZ = 0 AND FS = 0) PC ← rX
BGT #addr     ; if (FZ = 0 AND FS = 0) PC ← addr
- *Opcode*: 011, *ALU Code*: 100
- *Flags*: Not affected
- *Description*: Branch if result is positive and non-zero

<!-- TOC --><a name="bls-branch-if-less-than"></a>
#### BLS - Branch if Less Than
BLS rX        ; if (FS = 1) PC ← rX
BLS #addr     ; if (FS = 1) PC ← addr
- *Opcode*: 011, *ALU Code*: 101
- *Flags*: Not affected
- *Description*: Branch if sign flag is set

<!-- TOC --><a name="call-call-subroutine"></a>
#### CALL - Call Subroutine
CALL rX       ; stack ← PC; PC ← rX
CALL #addr    ; stack ← PC; PC ← addr
- *Opcode*: 011, *ALU Code*: 110
- *Flags*: Not affected
- *Description*: Pushes return address and jumps to subroutine

<!-- TOC --><a name="ret-return-from-subroutine"></a>
#### RET - Return from Subroutine
RET           ; PC ← stack
- *Opcode*: 011, *ALU Code*: 111
- *Flags*: Not affected
- *Description*: Returns from subroutine by popping return address

<!-- TOC --><a name="stack-operations"></a>
### Stack Operations

<!-- TOC --><a name="push-push-to-stack"></a>
#### PUSH - Push to Stack
PUSH rX       ; stack ← rX; SP ← SP - 1
- *Opcode*: 100
- *Flags*: Not affected
- *Description*: Pushes register value onto stack

<!-- TOC --><a name="pull-pull-from-stack"></a>
#### PULL - Pull from Stack
PULL rX       ; rX ← stack; SP ← SP + 1
- *Opcode*: 101
- *Flags*: Not affected
- *Description*: Pops value from stack into register

<!-- TOC --><a name="system-instructions"></a>
### System Instructions

<!-- TOC --><a name="int-software-interrupt"></a>
#### INT - Software Interrupt
INT           ; Trigger software interrupt
- *Opcode*: 110
- *Flags*: Not affected
- *Description*: Triggers a software interrupt. The specific behavior is implementation-dependent but typically involves jumping to an interrupt service routine.