# PCB mALUch Structure and Interface

This document focuses primarily on physical interface provided by specific modules and overall design choices. Some rationale is provided wherever decision choice may seem odd, however detailed description of individual modules is not provided, because the implementation is very straightforward with named labels and connections, providing detailed documentation for that seriously risks becoming obsolete in case of series of minor bug fixes, leading to being more confusing than helpful.


# Block Diagram
                                                              
```
+---------------------------------------------------------------+
:  CORE                                                         :
:                        ┌─────────────────┐                    : 
:                        │                 │                    : 
:                        │      ALU        │                    : 
:                        │                 │                    : 
:                        └─────────────────┘                    : 
:                                ▲▲                             : 
:                                ││                             : 
:                                ││                             : 
:                                ││                             : 
:                                ▼▼                             : 
: ┌──────────────┐       ┌─────────────────┐      ┌───────────┐ : 
: │              │       │                 │      │           │ : 
: │              │       │                 │      │           │ : 
: │    MEMORY    │◄─────►│     CONTROL     │◄────►│  REGFILE  │ : 
: │    CONTROL   │◄─────►│      UNIT       │◄────►│           │ : 
: │              │       │                 │      │           │ : 
: │              │       │                 │      │           │ : 
: └──────────────┘       └─────────────────┘      └───────────┘ : 
:        ▲▲                      ▲▲                     ▲▲      :   
:        ││                      ││                     ││      :   
+--------││----------+-----------││---------------------││------+ 
:        ││          :INTERFACE  ││                     ││      : 
:        ▼▼          :           ▼▼                     ▼▼      : 
: ┌──────────────┐   :   ┌────────────────────────────────────┐ :                  
: │              │   :   │                                    │ :                  
: │              │◄─────►│                                    │ :                  
: │     GPU      │◄─────►│       IO CONTROL                   │ :                  
: │              │   :   │                                    │ :                  
: │              │   :   │                                    │ :                  
: └──────────────┘   :   └────────────────────────────────────┘ :                  
:                    :           ▲▲                     ▲▲      : 
:                    :           ││                     ││      : 
:                    +-----------││---------------------││------+ 
:                                ││                     ││      :
:                                ▼▼                     ▼▼      :
:                        ┌────────────────────────────────────┐ :                 
:                        │                                    │ :                 
:                        │                                    │ :                 
:                        │       PERIPHERALS                  │ :                 
:                        │                                    │ :                 
:                        │                                    │ :                 
:                        └────────────────────────────────────┘ :                 
: IO                                                            :                
+---------------------------------------------------------------+
```


# Overall Structure

The Core is combination of ALU, Memory Control, Control Unit, and Regfile. Core is divided into these 4 parts roughly by function, allowing parallel work on each component by independent people. All these components are necessary for computer to function.    
IO Control is the interface between most IO devices and Core, its purpose is to unify the physical interfaces provided by various devices and adjust them to work with the one expected by ISA and Core. While not strictly necessary for Computer to function, it containes timer and ability to control external devices, which are crucial for interactability.  
The GPU is the special kind of peripheral which has dedicated memory address range, for this reason it is listed separately from peripherals, but it is not necessary for computer to function.  
The peripherals are general group of any IO supported currently by ISA, all can be connected to IO Control board and have their communication mediated by it. No peripheral is necessary for computer to function.  

# Interfaces

Each board must obviously receive power, the exact structure of that is not yet decided and not particularly important at this stage of development, for these reasons power and ground are not listed in the interfaces.

## ALU
    
Has only 1 interface to and from Control.

### Control interface

#### Connections
clock (1b)
bidirectional bus (16b)
flags (4b)
op (4B)

#### Behavior
    
Op is always written by the Control.

Control loads values to registers A and B which are stored until they get overwritten by Control again.  
Control picks operation to perform via op connection
- 0xxx - perform operation, xxx corresponds 1:1 to desired operation defined by ISA funct field, ALU writes the result onto bus
- 1000 - load register A, ALU reads the bus
- 1001 - load register B, ALU reads the bus
- else - undefined, ALU does not affect the bus

Flags are defined only for op = 0xxx, but are always written by ALU, for exact definition see the ISA.

Read is always asynchronous, meaning that read from ALU happens as soon as result is available.  
Write to register is always synchronous, meaning that it happens at clock rising edge.  

## Memory Controller

Has 2 interfaces: 1 to and from Control, 1 to and from GPU.

### Control interface

#### Connections

clock (1b)
bidirectional bus (16b) 
mode (3b)

#### Behavior

Mode is always written by Control, depending on its value different operation may happen
- 000 - idle ; does not write anything to the bus, does not read anything from it 
- 001 - undefined
- 010 - addres load ; address is read from bidirectional bus and stored internally
- 011 - address increment ; internal address is incremented, bus is not affected
- 100 - data store ; data is read from bidirectional bus and stored into memory
- 101 - data read ;  data is read from memory and put onto bus 
- 110 - undefined
- 111 - data read addr inc ; data is read from memory and put onto bus, then address is incremented

Read is always asynchronous, meaning that data is available as soon as possible.  
Write is always synchronous, meaning that data in memory is updated always at clock rising edge.

Very importantly, the "memory" refers to 3 kinds of memories used by ISA  
- READS  in range x0000 to x7FFF are directed to ROM   
- READS  in range x8000 to xFFFF are directed to RAM   
- WRITES in range x0000 to x7FFF are directed to GPU VRAM via its interface
- WRITES in range x8000 to xFFFF are directed to RAM

### GPU interface

#### Connections

clock (1b)
address bus (15b) 
data bus (8b) 
write_enable (1b)

#### Behavior
    
Whenever memory controller receives write on Control Interface to address in the range x0000 to x7FFF, it must be forwarded onto GPU interface.  
Adress is forwarded without most significant bit which is always zero.  
Only 8 least significant bits of data are transfered, as all valid characters are in this range.  

Write is always synchronous, meaning that data in memory is updated always at clock rising edge.


## Register File

Has 2 interfaces: 1 to and from Control, 1 to and from IO control.  
The name is slightly misleading as register file, for convienience, provides simplified interface for receiving interrupts.  

### Control Interface

#### Connections

clock (1b)  
bidirectional bus (16b)  
regid (4b)  
write_enable (1b)  
int (1b)  
intid (3b)  
busy (1b)  
mask_save (1b)  
mask_restore (1b)  

#### Behavior

Regid is always written to by Control, it chooses specific register to write to or read from.  
if write_enable is asserted, regfile reads data from bus and puts it into proper register, otherwise it always reads register and outputs it onto bus.  

It is extremely important to ensure updates to R1 from control do not clash with updates from IO control.  

Int is asserted when int_asserted ANDed with int_mask results in non zero value, int_id is set according to ISA rules.  
busy is asserted when ANY busy flag is non zero.  

Mask_save is used to save current mask and set the one in register 1 to x0000.  
Mask_restore is used to read saved mask and load it into registers 1.  

Read is always asynchronous, meaning that data is available as soon as possible.  
Write is always synchronous, meaning that data in registers is updated always at clock rising edge.

### IO Control Interface

#### Connections

int_asserted (8b)  
busy_asserted (8b)

#### Behavior

Both connections are only read and directed towards register 0 without any further processing.  

## IO Control

Has 3 interfaces: 1 to and from Control, 1 to and from Register File, 1 to and from GPU

### Control Interface

#### Connections

clock (1b)
bidirectional bus (16b)
io id (3b)
op (2b)

#### Behavior

Depending on op, the behavior is different
- 00 idle ; do nothing, bus is not affected
- 01 undefined
- 10 read ; reads from io pointed to by io id, puts result onto bus
- 11 write ; writes to io pointed to by io id, reads result from bus

Provides unified interface to IOs mentioned by the ISA.  

Read is always asynchronous, meaning that data is available as soon as possible, HOWEVER, read *acknowledge* by IO is always synchronous, because reading from IO may change its state.
Write is always synchronous, meaning that data in registers is updated always at clock rising edge.

### Register File Interface

#### Connections

int_asserted (8b)  
busy_asserted (8b)

#### Behavior

Both connections are only written to, they take the interrupt and busy info from IO directly and forward them to register file.  

### GPU Interface

#### Connections

write_enable (1b)  
write bus (16b)

#### Behavior

whenever write_enable is asserted, data is forwarded to GPU via write_bus. Write enable depends on Control ioid.  

Write is always synchronous, meaning that data in GPU is updated always at clock rising edge.
