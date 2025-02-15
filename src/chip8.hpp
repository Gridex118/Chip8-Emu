#ifndef CHIP8_H
#define CHIP8_H

#include <string>
#include <array>
#include <SDL2/SDL.h>
#include <chrono>

// 4096 cells, 1B each = 4096B = 4KiB
#define MEMCELL_MAX 4096
// 16 cells, 2B each = 32B
#define STACK_MAX 16

#define REAL_WIDTH 64
#define REAL_HEIGHT 32

using Clock = std::chrono::steady_clock;
using std::chrono::milliseconds;

extern uint8_t FONT_DATA[];

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
            ~Chip8Display();
            int init(std::string program, const short scaling_factor);
            void clear();
            bool draw(u_int8_t *sprite_base_addr, int x, int y, int rows);
        private:
            SDL_Window *window;
            SDL_Renderer *renderer;
            bool pixels_on_screen[REAL_HEIGHT][REAL_WIDTH] = {};
            void render_screen();
    };

    class Chip8Keypad {
        public:
            void request_halting_input(u_int8_t *store_at);
            void request_key(u_int8_t key, bool xor_mask);
            void handle_input(SDL_Event *event, const Uint8 *kbstate, u_int16_t *program_counter);
        private:
            bool halting_input_requested = false;
            u_int8_t *storage_reg;
            bool key_check_requested = false;
            u_int8_t requested_key;
            bool key_skip_xor_mask = 0;
    };

    struct Memory {
        std::array<u_int8_t, MEMCELL_MAX> ram = {};
        std::array<u_int16_t, STACK_MAX> stack = {};
    };

    struct Bus {
        Memory *memory;
        Chip8Display *display;
        Chip8Keypad *keypad;
    };

    struct Chip8Cpu {
        Chip8Cpu(Memory *memory, Chip8Display *display, Chip8Keypad *keypad);
        Bus bus;
        std::array<u_int8_t, REG_MAX> regs = {};
        u_int8_t SP;    // Stack Pointer
        u_int16_t PC;   // Program Counter
        u_int16_t I;    // Index Register
        std::array<u_int8_t, TIMERS_MAX> timers = {};
        void decrement_timers();
        int exec_next();
        private:
            u_int16_t instruction;
            inline void fetch_instr();
            std::chrono::time_point<Clock> last_time_stamp = Clock::now();
    };

    class Chip8Emu {
        public:
            Chip8Emu();
            ~Chip8Emu();
            int run_program(std::string program, const short display_scaling_factor, const short cpu_freq);
        private:
            std::string runnig_program;
            Chip8Display *display;
            Chip8Keypad *keypad;
            Memory *memory;
            Chip8Cpu *cpu;
            int load_program();
    };

}

#endif
