#include "chip8.hpp"
#include <iostream>

#define arrlen(arr) (sizeof arr / sizeof arr[0])

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
                    switch (instruction & 0x0FFF) {
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
                    cpu->PC = (instruction & 0x0FFF) - 0x0200;
                    break;
                case 0x3:
                    if (cpu->regs[(instruction & 0x0F00) >> 8] == (instruction & 0x00FF)) {
                        cpu->PC += 2;
                    }
                    break;
                case 0x4:
                    if (cpu->regs[(instruction & 0x0F00) >> 8] != (instruction & 0x00FF)) {
                        cpu->PC += 2;
                    }
                    break;
                case 0x5:
                    if (cpu->regs[(instruction & 0x0F00) >> 8] == cpu->regs[instruction & 0x00F0]) {
                        cpu->PC += 2;
                    }
                    break;
                case 0x9:
                    if (cpu->regs[(instruction & 0x0F00) >> 8] != cpu->regs[instruction & 0x00F0]) {
                        cpu->PC += 2;
                    }
                    break;
                case 0x6:
                    cpu->regs[(instruction & 0x0F00) >> 8] = (instruction & 0x00FF);
                    break;
                case 0x7:
                    cpu->regs[(instruction & 0x0F00) >> 8] += (instruction & 0x00FF);
                    break;
                case 0x8:
                    break;
                case 0xa:
                    break;
                case 0xb:
                    break;
                case 0xc:
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
