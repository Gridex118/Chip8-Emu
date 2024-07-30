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

inline u_int16_t font_addr(u_int8_t font) {
    return FONT_DATA[5 * font];
}

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

    inline u_int16_t Chip8Emu::fetch_instr() {
        u_int16_t instruction = (memory[0x200 + cpu->PC] << 8) + (memory[0x200 + cpu->PC + 1]);
        cpu->PC += 2;
        return instruction;
    }

    int Chip8Emu::exec_instr(u_int16_t &instruction) {
        switch ((instruction & 0xF000) >> 12) {
            case 0x0:
                switch (NNN(instruction)) {
                    case 0xE0:
                        display->clear();
                        break;
                    case 0xEE:
                        // Return from function call
                        cpu->PC = stack[--cpu->SP];
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
                display->draw(&memory[cpu->I], cpu->regs[REG_X(instruction)], cpu->regs[REG_Y(instruction)], N(instruction));
                break;
            case 0xe:
                // Skipping based on key presses
                break;
            case 0xf:
                switch (NN(instruction)) {
                    case 0x07:
                        cpu->regs[REG_X(instruction)] = cpu->timers[D];
                        break;
                    case 0x15:
                        cpu->timers[D] = cpu->regs[REG_X(instruction)];
                        break;
                    case 0x18:
                        cpu->timers[S] = cpu->regs[REG_X(instruction)];
                        break;
                    case 0x1E:
                        cpu->I += cpu->regs[REG_X(instruction)];
                        cpu->regs[VF] = (cpu->I > 0x1000)? 1 : 0;
                        break;
                    case 0x0A:
                        // Blocking input
                        break;
                    case 0x29:
                        {
                            u_int8_t font = REG_X(instruction);
                            if (font > 0xf) {
                                std::cerr << "Trying to access unknown font\n";
                                return -1;
                            }
                            cpu->I = font_addr(font);
                        }
                        break;
                    case 0x33:
                        // copy decimal fonts for VX to I
                        break;
                    case 0x55:
                        for (int x = 0; x <= REG_X(instruction); x++) {
                            memory[cpu->I + x] = cpu->regs[x];
                        }
                        break;
                    case 0x65:
                        for (int x = 0; x <= REG_X(instruction); x++) {
                            cpu->regs[x] = memory[cpu->I + x];
                        }
                        break;
                    default:
                        std::cerr << "Invalid Operation requested: " << std::hex << instruction << '\n';
                        return -1;
                }
                break;
            default:
                break;
        }
        return 0;
    }

    int Chip8Emu::run_program(std::string program) {
        runnig_program = program;
        display->init(runnig_program);
        if (load_program() != 0) {
            std::cout << "Error while loading program to memory\n";
            return -1;
        }
        while (true) {
            if ((SDL_PollEvent(&event) && event.type == SDL_QUIT)) {
                break;
            }
            u_int16_t instruction = fetch_instr();
            if (exec_instr(instruction) != 0) {
                std::cerr << "Error in execution stage\n";
                return -1;
            }
        }
        return 0;
    }

}
