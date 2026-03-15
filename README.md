## mALUch - 16 bit simple CPU

mALUch is a simple 16 bit cpu, created mostly as a design exercise 

## Running Simulation and Compiling Tools
To compile simulator it is necessary to have ncurses installed  

```bash
cd Soft/Simulator
make
```

To run a file it's as simple as
```bash
./simulate path/to/file
```

## Assembler  
To use assembler it is necessary to have fasmg installed

The assembler is implemented as a fasmg .inc file, so to write your own program you have to have `include "path/to/maluch.inc"` included in your file.  
To compile a program do
```bash
fasmg main.asm [output]
```
The output name is optional, the default is the name of the input file sans the extension.

## Physical Implementation
work in progress 😎

## Software Implementation
in Simulator/  
<img width="480" height="260" alt="image" src="https://github.com/user-attachments/assets/f5a14a92-ca31-491f-be20-6069195eb470" />  
mostly straightforward implementation with no interesting quirks, type `help` for list of commands, use `tab` to switch between COMMANDS window and SCREEN window.

## Authors
[@sarvl](https://github.com/sarvl)  
[@Racuun](https://github.com/Racuun)  
