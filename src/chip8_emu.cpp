#include "chip8.hpp"
#include <iostream>
#include <cstdlib>

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
    keypad = new Chip8Keypad();
    memory = new Memory;
    cpu = new Chip8Cpu(memory, display, keypad);
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

int Chip8Emu::run_program(std::string program, const short display_scaling_factor, const short cpu_freq) {
    const double FRAMEDELAY = 1000 / cpu_freq;
    runnig_program = program;
    display->init(runnig_program, display_scaling_factor);
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
        if (cpu->exec_next() != 0) {
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
