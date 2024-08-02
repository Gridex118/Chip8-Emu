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
    return (5 * font);
}

inline size_t filesize(const char *file_name) {
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) return 0;
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fclose(file);
    return size;
}

void store_decimal(u_int8_t *storage_base_addr, int number) {
    int i = 2;
    while (i >= 0) {
        storage_base_addr[i--] = number % 10;
        number /= 10;
    }
}

namespace chip8 {

    inline void Chip8Cpu::decrement_timers() {
        auto current_time_stamp = Clock::now();
        milliseconds duration = std::chrono::duration_cast<milliseconds>(current_time_stamp - last_time_stamp);
        milliseconds min_ms_elapsed = static_cast<milliseconds>(17);
        if (duration >= min_ms_elapsed) {
            last_time_stamp = current_time_stamp;
            if (timers[D]) --timers[D];
            if (timers[S]) --timers[S];
        }
    }

    Chip8Emu::Chip8Emu() {
        display = new Chip8Display();
        cpu = new Chip8Cpu();
        keypad = new Chip8Keypad();
        memory = new Memory;
        for (size_t i = 0; i < arrlen(FONT_DATA); i++) {
            memory->ram[i] = FONT_DATA[i];
        }
    }

    Chip8Emu::~Chip8Emu() {
        delete display;
        delete cpu;
        delete keypad;
        delete memory;
    }

    int Chip8Emu::load_program() {
        FILE *source = fopen(runnig_program.c_str(), "rb");
        if (source == NULL) {
            std::cerr << "Could not open file\n";
            std::cerr << errno << '\n';
            return -1;
        }
        size_t size = filesize(runnig_program.c_str());
        if (size == 0) {
            std::cerr << "Empty program source\n";
            return -1;
        }
        fread(&memory->ram[0x200], sizeof(u_int8_t), size, source);
        fclose(source);
        return 0;
    }

    inline u_int16_t Chip8Emu::fetch_instr() {
        u_int16_t instruction = (memory->ram[0x200 + cpu->PC] << 8) + (memory->ram[0x200 + cpu->PC + 1]);
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
                        cpu->PC = memory->stack[--cpu->SP];
                        break;
                    default:
                        // Ignore; unimplemented machine instructions
                        break;
                }
                break;
            case 0x2:
                memory->stack[cpu->SP++] = cpu->PC;
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
                {
                    int regx = REG_X(instruction);
                    int regy = REG_Y(instruction);
                    switch (instruction & 0x000F) {
                        case 0x0:
                            cpu->regs[regx] = cpu->regs[regy];
                            break;
                        case 0x1:
                            cpu->regs[regx] |= cpu->regs[regy];
                            break;
                        case 0x2:
                            cpu->regs[regx] &= cpu->regs[regy];
                            break;
                        case 0x3:
                            cpu->regs[regx] ^= cpu->regs[regy];
                            break;
                        case 0x4:
                            {
                                u_int8_t reg_pre_addition = cpu->regs[regx];
                                cpu->regs[regx] += cpu->regs[regy];
                                // If, after an addition, the reg is smaller than before, it must have overflown
                                cpu->regs[VF] = (reg_pre_addition > cpu->regs[regx]);
                            }
                            break;
                        case 0x5:
                            {
                                bool flag = cpu->regs[regx] >= cpu->regs[regy];
                                cpu->regs[regx] -= cpu->regs[regy];
                                cpu->regs[VF] = flag;
                            }
                            break;
                        case 0x7:
                            {
                                bool flag = cpu->regs[regy] >= cpu->regs[regx];
                                cpu->regs[regx] = cpu->regs[regy] - cpu->regs[regx];
                                cpu->regs[VF] = flag;
                            }
                            break;
                        case 0x6:
                            {
                                bool flag = cpu->regs[regx] & 0x01;
                                cpu->regs[regx] = cpu->regs[regx] >> 1;
                                cpu->regs[VF] = flag;
                            }
                            break;
                        case 0xe:
                            {
                                bool flag = cpu->regs[regx] & 0x80;
                                cpu->regs[regx] = cpu->regs[regx] << 1;
                                cpu->regs[VF] = flag;
                            }
                            break;
                        default:
                            std::cerr << "Illegal operation " << std::hex << instruction << '\n';
                            return -1;
                            break;
                    }
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
                cpu->regs[VF] = display->draw(&memory->ram[cpu->I], cpu->regs[REG_X(instruction)],
                        cpu->regs[REG_Y(instruction)], N(instruction)
                        );
                break;
            case 0xe:
                switch (NN(instruction)) {
                    case 0x9e:
                        // Skip next instruction if key VX is down
                        keypad->request_key(cpu->regs[REG_X(instruction)], 1);
                        break;
                    case 0xa1:
                        // Skip next instruction if key VX is up
                        keypad->request_key(cpu->regs[REG_X(instruction)], 0);
                        break;
                    default:
                        std::cerr << "Illegal Operation requested\n";
                        return -1;
                }
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
                        break;
                    case 0x0A:
                        keypad->request_halting_input(&cpu->regs[REG_X(instruction)]);
                        break;
                    case 0x29:
                        {
                            u_int8_t font = cpu->regs[REG_X(instruction)];
                            if (font > 0xf) {
                                std::cerr << "Trying to access unknown font\n";
                                return -1;
                            }
                            cpu->I = font_addr(font);
                        }
                        break;
                    case 0x33:
                        store_decimal(&memory->ram[cpu->I], cpu->regs[REG_X(instruction)]);
                        break;
                    case 0x55:
                        for (int x = 0; x <= REG_X(instruction); x++) {
                            memory->ram[cpu->I + x] = cpu->regs[x];
                        }
                        break;
                    case 0x65:
                        for (int x = 0; x <= REG_X(instruction); x++) {
                            cpu->regs[x] = memory->ram[cpu->I + x];
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
            std::cerr << "Error while loading program to memory\n";
            return -1;
        }
        const Uint8 *kbstate = SDL_GetKeyboardState(NULL);
        SDL_Event event;
        bool running = true;
        int frame_time;
        while (running) {
            int frame_start = SDL_GetTicks();
            u_int16_t instruction = fetch_instr();
            if (exec_instr(instruction) != 0) {
                std::cerr << "Error in execution stage\n";
                return -1;
            }
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                }
            }
            keypad->handle_input(&event, kbstate, &cpu->PC);
            cpu->decrement_timers();
            frame_time = SDL_GetTicks() - frame_start;
            if (FRAMEDELAY > frame_time) {
                SDL_Delay(FRAMEDELAY - frame_time);
            }
        }
        return 0;
    }

}
