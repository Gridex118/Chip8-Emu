#include "chip8.hpp"
#include <iostream>

#define REG_X(instr) ((instr & 0x0F00) >> 8)
#define REG_Y(instr) ((instr & 0x00F0) >> 4)
#define NNN(instr) (instr & 0x0FFF)
#define NN(instr) (instr & 0x00FF)
#define N(instr) (instr & 0x000F)

inline void store_decimal(u_int8_t *storage_base_addr, int number) {
    int i = 2;
    while (i >= 0) {
        storage_base_addr[i--] = number % 10;
        number /= 10;
    }
}

inline u_int16_t font_addr(u_int8_t font) {
    return (5 * font);
}

namespace chip8 {

Chip8Cpu::Chip8Cpu(std::shared_ptr<Memory> memory, std::shared_ptr<Chip8Display> display, std::shared_ptr<Chip8Keypad> keypad) {
    bus.memory = memory;
    bus.display = display;
    bus.keypad = keypad;
}

void Chip8Cpu::decrement_timers() {
    auto current_time_stamp = Clock::now();
    milliseconds duration = std::chrono::duration_cast<milliseconds>(current_time_stamp - last_time_stamp);
    milliseconds min_ms_elapsed = static_cast<milliseconds>(17);
    if (duration >= min_ms_elapsed) {
        last_time_stamp = current_time_stamp;
        if (timers[D]) --timers[D];
        if (timers[S]) --timers[S];
    }
}

inline void Chip8Cpu::fetch_instr() {
    instruction = (bus.memory->ram[PC] << 8) + (bus.memory->ram[PC + 1]);
    PC += 2;
}

int Chip8Cpu::exec_next() {
    fetch_instr();
    switch ((instruction & 0xF000) >> 12) {
        case 0x0:
            switch (NNN(instruction)) {
                case 0xE0:
                    bus.display->clear();
                    break;
                case 0xEE:
                    // Return from function call
                    PC = bus.memory->stack[--SP];
                    break;
                default:
                    // Ignore; unimplemented machine instructions
                    break;
            }
            break;
        case 0x2:
            bus.memory->stack[SP++] = PC;
            [[fallthrough]];
        case 0x1:
            // Jump to address
            PC = NNN(instruction);
            break;
        case 0x3:
            if (regs[REG_X(instruction)] == NN(instruction)) {
                PC += 2;
            }
            break;
        case 0x4:
            if (regs[REG_X(instruction)] != NN(instruction)) {
                PC += 2;
            }
            break;
        case 0x5:
            if (regs[REG_X(instruction)] == regs[REG_Y(instruction)]) {
                PC += 2;
            }
            break;
        case 0x9:
            if (regs[REG_X(instruction)] != regs[REG_Y(instruction)]) {
                PC += 2;
            }
            break;
        case 0x6:
            regs[REG_X(instruction)] = NN(instruction);
            break;
        case 0x7:
            regs[REG_X(instruction)] += NN(instruction);
            break;
        case 0x8:
            {
                int regx = REG_X(instruction);
                int regy = REG_Y(instruction);
                switch (instruction & 0x000F) {
                    case 0x0:
                        regs[regx] = regs[regy];
                        break;
                    case 0x1:
                        regs[regx] |= regs[regy];
                        break;
                    case 0x2:
                        regs[regx] &= regs[regy];
                        break;
                    case 0x3:
                        regs[regx] ^= regs[regy];
                        break;
                    case 0x4:
                        {
                            u_int8_t reg_pre_addition = regs[regx];
                            regs[regx] += regs[regy];
                            // If, after an addition, the reg is smaller than before, it must have overflown
                            regs[VF] = (reg_pre_addition > regs[regx]);
                        }
                        break;
                    case 0x5:
                        {
                            bool flag = regs[regx] >= regs[regy];
                            regs[regx] -= regs[regy];
                            regs[VF] = flag;
                        }
                        break;
                    case 0x7:
                        {
                            bool flag = regs[regy] >= regs[regx];
                            regs[regx] = regs[regy] - regs[regx];
                            regs[VF] = flag;
                        }
                        break;
                    case 0x6:
                        {
                            bool flag = regs[regx] & 0x01;
                            regs[regx] = regs[regx] >> 1;
                            regs[VF] = flag;
                        }
                        break;
                    case 0xe:
                        {
                            bool flag = regs[regx] & 0x80;
                            regs[regx] = regs[regx] << 1;
                            regs[VF] = flag;
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
            I = NNN(instruction);
            break;
        case 0xb:
            PC = regs[V0] + NNN(instruction);
            break;
        case 0xc:
            regs[REG_X(instruction)] = rand() & NN(instruction);
            break;
        case 0xd:
            regs[VF] = bus.display->draw(&bus.memory->ram[I], regs[REG_X(instruction)],
                                         regs[REG_Y(instruction)], N(instruction)
                                         );
            break;
        case 0xe:
            switch (NN(instruction)) {
                case 0x9e:
                    // Skip next instruction if key VX is down
                    bus.keypad->request_key(regs[REG_X(instruction)], 1);
                    break;
                case 0xa1:
                    // Skip next instruction if key VX is up
                    bus.keypad->request_key(regs[REG_X(instruction)], 0);
                    break;
                default:
                    std::cerr << "Illegal Operation requested\n";
                    return -1;
            }
            break;
        case 0xf:
            switch (NN(instruction)) {
                case 0x07:
                    regs[REG_X(instruction)] = timers[D];
                    break;
                case 0x15:
                    timers[D] = regs[REG_X(instruction)];
                    break;
                case 0x18:
                    timers[S] = regs[REG_X(instruction)];
                    break;
                case 0x1E:
                    I += regs[REG_X(instruction)];
                    break;
                case 0x0A:
                    bus.keypad->request_halting_input(&regs[REG_X(instruction)]);
                    break;
                case 0x29:
                    {
                        u_int8_t font = regs[REG_X(instruction)];
                        if (font > 0xf) {
                            std::cerr << "Trying to access unknown font\n";
                            return -1;
                        }
                        I = font_addr(font);
                    }
                    break;
                case 0x33:
                    store_decimal(&bus.memory->ram[I], regs[REG_X(instruction)]);
                    break;
                case 0x55:
                    for (int x = 0; x <= REG_X(instruction); x++) {
                        bus.memory->ram[I + x] = regs[x];
                    }
                    break;
                case 0x65:
                    for (int x = 0; x <= REG_X(instruction); x++) {
                        regs[x] = bus.memory->ram[I + x];
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

}
