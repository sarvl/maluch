## mALUch - 16 bit simple CPU

mALUch is a simple 16 bit cpu, created mostly as a design exercise 

## Running Simulation and Compiling Tools
All that is necessary to compile existing tooling is
```
make init
```

then assemble test program as 
```
./assemble.sh Progs/os_test.cpp out.bin
```

and finally run it as 
```
./simulate out.bin
```

to get list of commands type
```
help
```
to switch between windows use TAB

## Assembler  
in Soft/  
Created as sort of an experiment as a `.cpp` file, to create an assembly see for example in `Progs/` that includes `asm.h`.  
Use via `./assembly.sh`, this compiles `.cpp` "assembly" file that creates binary, execution of that binary outputs text file with encoded instructions. This later can be simulated by simulator.

## Physical Implementation
work in progress 😎

## Software Implementation
in Simulator/  
<img width="480" height="260" alt="image" src="https://github.com/user-attachments/assets/f5a14a92-ca31-491f-be20-6069195eb470" />  
mostly straightforward implementation with no interesting quirks, type `help` for list of commands, use `tab` to switch between COMMANDS window and SCREEN window.

## Authors
[@sarvl](https://github.com/sarvl)  
[@Racuun](https://github.com/Racuun)  
