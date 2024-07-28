#include "chip8.hpp"
#include <iostream>
#include <cstdlib>

#define arrlen(arr) (sizeof arr / sizeof arr[0])

#define REG_X(instr) ((instr & 0x0F00) >> 8)
#define REG_Y(instr) ((instr & 0x00F0) >> 4)
#define NNN(instr) (instr & 0x0FFF)
#define NN(instr) (instr & 0x00FF)
#define N(instr) (instr & 0x000F)

uint8_t FONT_DATA[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,   // 0
    0x20, 0x60, 0x20, 0x20, 0x70,   // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,   // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,   // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,   // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,   // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,   // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,   // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,   // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,   // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,   // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,   // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,   // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,   // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,   // E
    0xF0, 0x80, 0xF0, 0x80, 0x80    // F
};

inline size_t filesize(const char *file_name) {
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) return 0;
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fclose(file);
    return size;
}

namespace chip8 {

    Chip8Emu::Chip8Emu() {
        display = new Chip8Display();
        cpu = new Chip8Cpu();
        for (size_t i = 0; i < arrlen(FONT_DATA); i++) {
            memory[i] = FONT_DATA[i];
        }
    }

    Chip8Emu::~Chip8Emu() {
        delete display;
        delete cpu;
    }

    int Chip8Emu::load_program() {
        FILE *source = fopen(runnig_program.c_str(), "rb");
        if (source == NULL) {
            std::cout << "Could not open file\n";
            std::cout << errno << '\n';
            return -1;
        }
        size_t size = filesize(runnig_program.c_str());
        if (size == 0) {
            std::cout << "Empty program source\n";
            return -1;
        }
        fread(&memory[0x200], sizeof(u_int8_t), size, source);
        fclose(source);
        return 0;
    }

    int Chip8Emu::run_program(std::string program) {
        runnig_program = program;
        if (load_program() != 0) {
            std::cout << "Error while loading program to memory\n";
            return -1;
        }
        while (true) {
            u_int16_t instruction = (memory[0x200 + cpu->PC] << 8) + (memory[0x200 + cpu->PC + 1]);
            cpu->PC += 2;
            switch ((instruction & 0xF000) >> 12) {
                case 0x0:
                    switch (NNN(instruction)) {
                        case 0xE0:
                            // Clear Screen
                            break;
                        case 0xEE:
                            // Return from function call
                            cpu->PC = stack[cpu->SP--];
                            break;
                        default:
                            // Ignore; unimplemented machine instructions
                            break;
                    }
                    break;
                case 0x2:
                    stack[cpu->SP++] = cpu->PC;
                    // Fallthrough here; 0x2NNN requires jumping
                case 0x1:
                    // Jump to address
                    cpu->PC = NNN(instruction) - 0x0200;
                    break;
                case 0x3:
                    if (cpu->regs[REG_X(instruction)] == NN(instruction)) {
                        cpu->PC += 2;
                    }
                    break;
                case 0x4:
                    if (cpu->regs[REG_X(instruction)] != NN(instruction)) {
                        cpu->PC += 2;
                    }
                    break;
                case 0x5:
                    if (cpu->regs[REG_X(instruction)] == cpu->regs[REG_Y(instruction)]) {
                        cpu->PC += 2;
                    }
                    break;
                case 0x9:
                    if (cpu->regs[REG_X(instruction)] != cpu->regs[REG_Y(instruction)]) {
                        cpu->PC += 2;
                    }
                    break;
                case 0x6:
                    cpu->regs[REG_X(instruction)] = NN(instruction);
                    break;
                case 0x7:
                    cpu->regs[REG_X(instruction)] += NN(instruction);
                    break;
                case 0x8:
                    switch (instruction & 0x000F) {
                        case 0x0:
                            cpu->regs[REG_X(instruction)] = cpu->regs[REG_Y(instruction)];
                            break;
                        case 0x1:
                            cpu->regs[REG_X(instruction)] |= cpu->regs[REG_Y(instruction)];
                            break;
                        case 0x2:
                            cpu->regs[REG_X(instruction)] &= cpu->regs[REG_Y(instruction)];
                            break;
                        case 0x3:
                            cpu->regs[REG_X(instruction)] ^= cpu->regs[REG_Y(instruction)];
                            break;
                        case 0x4:
                            cpu->regs[REG_X(instruction)] += cpu->regs[REG_Y(instruction)];
                            // If, after an addition, the reg goes to 0, it must have overflown
                            cpu->regs[VF] = cpu->regs[REG_X(instruction)] == 0;
                            break;
                        case 0x5:
                            cpu->regs[REG_X(instruction)] -= cpu->regs[REG_Y(instruction)];
                            cpu->regs[VF] = cpu->regs[REG_X(instruction)] > cpu->regs[REG_Y(instruction)];
                            break;
                        case 0x6:
                            cpu->regs[VF] = cpu->regs[REG_Y(instruction)] & 0x0001;
                            cpu->regs[REG_X(instruction)] = cpu->regs[REG_Y(instruction)] >> 1;
                            break;
                        case 0x7:
                            cpu->regs[REG_Y(instruction)] -= cpu->regs[REG_X(instruction)];
                            cpu->regs[VF] = cpu->regs[REG_Y(instruction)] > cpu->regs[REG_X(instruction)];
                            break;
                        case 0xe:
                            cpu->regs[VF] = cpu->regs[REG_Y(instruction)] & 0x8000;
                            cpu->regs[REG_X(instruction)] = cpu->regs[REG_Y(instruction)] << 1;
                            break;
                        default:
                            std::cerr << "Illegal operation " << std::hex << instruction << '\n';
                            return -1;
                            break;
                    }
                    break;
                case 0xa:
                    cpu->I = NNN(instruction);
                    break;
                case 0xb:
                    cpu->PC = cpu->regs[V0] + NNN(instruction) - 0x200;
                    break;
                case 0xc:
                    cpu->regs[REG_X(instruction)] = rand() & NN(instruction);
                    break;
                case 0xd:
                    break;
                case 0xe:
                    break;
                case 0xf:
                    break;
                default:
                    break;
            }
        }
        return 0;
    }

}
