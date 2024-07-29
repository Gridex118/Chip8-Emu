#ifndef CHIP8_H
#define CHIP8_H

#include <string>
#include <array>
#include <SDL2/SDL.h>

// 4096 cells, 1B each = 4096B = 4KiB
#define MEMCELL_MAX 4096
// 16 cells, 2B each = 32B
#define STACK_MAX 16

#define PIXEL_SCALE_FACTOR (10)
#define SCALE_PX(px) (PIXEL_SCALE_FACTOR * px)
#define REAL_WIDTH 64
#define REAL_HEIGHT 32
#define WINDOW_WIDTH (SCALE_PX(REAL_WIDTH))
#define WINDOW_HEIGHT (SCALE_PX(REAL_HEIGHT))


namespace chip8 {

    // General purpose registers
    enum Reg {
        V0, V1, V2, V3, V4,
        V5, V6, V7, V8, V9,
        VA, VB, VC, VD, VE,
        VF, REG_MAX
    };

    // Delay and sound timers
    enum Timers {
        D, S, TIMERS_MAX
    };

    class Chip8Display {
        public:
            Chip8Display() {};
            ~Chip8Display();
            int init(std::string program);
            void clear();
        private:
            SDL_Window *window;
            SDL_Renderer *renderer;
            bool pixels_on_screen[REAL_WIDTH][REAL_WIDTH] = {};
    };

    struct Chip8Cpu {
        Chip8Cpu() {};
        ~Chip8Cpu() {};
        std::array<u_int8_t, REG_MAX> regs = {};
        u_int8_t SP;    // Stack Pointer
        u_int16_t PC;   // Program Counter
        u_int16_t I;    // Index Register
        std::array<u_int8_t, TIMERS_MAX> timers = {};
    };

    class Chip8Emu {
        public:
            Chip8Emu();
            ~Chip8Emu();
            int run_program(std::string program);
        private:
            std::string runnig_program;
            Chip8Display *display;
            std::array<u_int8_t, MEMCELL_MAX> memory = {};
            std::array<u_int16_t, STACK_MAX> stack = {};
            Chip8Cpu *cpu;
            int load_program();
    };

}

#endif
