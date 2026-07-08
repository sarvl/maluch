# Overview

The Keyboard controller (refered to simply as the controller) is responsible for translating data from PS2 format to ASCII format expected by the CPU.  
The controller operates in two states: regular and shift. The difference between states is additional input to translation Lookup Table between keyboard scancodes and ascii output.  

## PS2 Interface

Whenever the controller is ready to receive the data, it leaves the clock pin floating.  
When the key is pressed or released, the scancode is shifted into register and depending on the value different action is taken:
- left shift pressed      - state is switched to shift  
- left shift released     - state is switched to regular  
- regular key is released - the key is translated into ascii sequence and stored internally  

After regular key is pressed, the controller stops receiving data by setting clock pin low until the CPU interface is handled.

## CPU Interface

Once the data is in the output buffer, the controller signals data availability with interrupt pin. When CPU accesses port 1, it outputs its contents, then signals back to the PS2 interface that controller is now ready to receive data again.
