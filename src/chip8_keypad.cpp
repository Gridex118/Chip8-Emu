#include "chip8.hpp"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <unordered_map>

const std::unordered_map<u_int8_t, int> KEYS = {
    {SDL_SCANCODE_1, 0x1}, {SDL_SCANCODE_2, 0x2},
    {SDL_SCANCODE_3, 0x3}, {SDL_SCANCODE_4, 0xc},
    {SDL_SCANCODE_Q, 0X4}, {SDL_SCANCODE_W, 0X5},
    {SDL_SCANCODE_E, 0X6}, {SDL_SCANCODE_R, 0XD},
    {SDL_SCANCODE_A, 0X7}, {SDL_SCANCODE_S, 0X8},
    {SDL_SCANCODE_D, 0X9}, {SDL_SCANCODE_F, 0XE},
    {SDL_SCANCODE_Z, 0XA}, {SDL_SCANCODE_X, 0X0},
    {SDL_SCANCODE_C, 0XB}, {SDL_SCANCODE_V, 0XF}
};

const std::unordered_map<int, u_int8_t> KEYS_REV = {
    {0x1, SDL_SCANCODE_1}, {0x2, SDL_SCANCODE_2},
    {0x3, SDL_SCANCODE_3}, {0xc, SDL_SCANCODE_4},
    {0X4, SDL_SCANCODE_Q}, {0X5, SDL_SCANCODE_W},
    {0X6, SDL_SCANCODE_E}, {0XD, SDL_SCANCODE_R},
    {0X7, SDL_SCANCODE_A}, {0X8, SDL_SCANCODE_S},
    {0X9, SDL_SCANCODE_D}, {0XE, SDL_SCANCODE_F},
    {0XA, SDL_SCANCODE_Z}, {0X0, SDL_SCANCODE_X},
    {0XB, SDL_SCANCODE_C}, {0XF, SDL_SCANCODE_V},
};


namespace chip8 {

    void Chip8Keypad::init() {
        this->kbstate = SDL_GetKeyboardState(NULL);
    }

    void Chip8Keypad::request_halting_input(u_int8_t *store_at) {
        halting_input_requested = true;
        storage_reg = store_at;
    }

    void Chip8Keypad::request_key(u_int8_t key, bool xor_mask) {
        key_check_requested = true;
        requested_scancode = KEYS_REV.at(key);
        key_skip_xor_mask = xor_mask;
    }

    void Chip8Keypad::handle_input(u_int16_t *program_counter) {
        if (halting_input_requested) {
            for (const auto &[scancode, key] : KEYS) {
                if (kbstate[scancode]) {
                    *storage_reg = key;
                    halting_input_requested = false;
                    return;
                }
            }
            return;
        } else if (key_check_requested) {
            if (!(kbstate[requested_scancode] ^ key_skip_xor_mask)) {
                *program_counter += 2;
            }
            key_check_requested = false;
        }
    }

}
