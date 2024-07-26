# CHIP8 EMU 
A Chip-8 Emulator written in C++ with SDL2 for rendering.

## VM Specifications

### Memory
* Main Memory: 4KB, Readable Writable RAM, implemented as 8 bit cell array
    * 512B reserved for ROM compatibility
* Registers:
    * 16 General Purpose (V0 - VF), 8 bit (VF may be used as a flag register)
    * Program Counter(PC), 16 bit
    * Index Register(I), 16 bit
* Stack: 32B or more, implemented as 16 bit cell array

### Timers
60Hz decrement , with minimum value of 0
* Delay Timer(DT): 8 bit
* Sound Timer(ST): 8 bit

### Keypad
|     |     |     |     |
| --- | --- | --- | --- |
| 1   | 2   | 3   | C   |
| 4   | 5   | 6   | D   |
| 7   | 8   | 9   | E   |
| A   | 0   | B   | F   |

### Display
* 64x32 px
* Black and White

### Font
* For Hexadecimal characters 0...F
* 4px wide and 5px high
* May be stored somewhere in the 512B reserved in the main memory

### Instruction Set
1. Group 0: Exec machine instruction. Only 2 shall be implemented
    * 00E0: Clear screen
    * 00EE: Return from function call
2. 1NNN: Jump to NNN
3. 2NNN: Jump to NNN as function call
4. 3XNN: Skip the next instruction if VX == NN
5. 4XNN: Complement of 3XNN
6. 5XY0: Skip if VX == VY
7. 6XNN: Set VX = NN
8. 7XNN: Set VX = VX + NN
9. Group 8XY: Arithmetic and Logical
    * 0: Set VX = VY
    * 1: Set VX = VX | VY
    * 2: Set VX = VX & VY
    * 3: Set VX = VX ^ VY (XOR)
    * 4: Set VX = VX + VY
    * 5: Set VX = VX - VY
    * 6: Set VX = VY >> 1, may ignore VY and bitshift VX
    * 7: Set VX = VY - VX
    * E: Set VX = VY << 1, may ignore VY and bitshift VX
10. 9XY0: Complement of 5XY0
11. ANNN: Set I = NNN
12. BNNN: Jump to (V0 + NNN)
    * or, BXNN: Jump to (VX + NN)
13. CXNN: Set VX = (RANDOM NUMBER) & NN
14. DXYN: Draw at (VX, VY) an N pixels tall image found at address I
15. Group E: Key press
    * EX9E: Skip next instruction if key VX is pressed
    * EXA1: Complement of EX9E
16. Group F:
    * FX07: Set VX = DT
    * FX15: Set DT = VX
    * FX18: Set ST = VX
    * FX1E: Set I = I + VX, set VF = 1 if I exceeds 0x1000
    * FX0A: Blocking Input, set VX = Input
    * FX29: Point I to the address of the font for the character in VX
    * FX33: Copy the decimal representation of VX to address I... (String)
    * FX55: Copy V0...VX to address I...(I+X)
    * FX65: Copy address I...(I+X) to V0...VX
