#include "chip8.hpp"
#include <iostream>

namespace chip8 {

    Chip8Display::~Chip8Display() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    int Chip8Display::init(std::string program) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError() << '\n';
            return -1;
        }
        window = SDL_CreateWindow(program.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == NULL) {
            std::cerr << "Failed to create window: " << SDL_GetError() << '\n';
            return -1;
        }
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        SDL_RenderSetLogicalSize(renderer, SCALE_PX(2), SCALE_PX(1));
        return 0;
    }

    void Chip8Display::clear() {
        for (size_t i = 0; i < REAL_HEIGHT; i++) {
            for (size_t j = 0; j < REAL_WIDTH; j++) {
                pixels_on_screen[i][j] = 0;
            }
        }
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);
    }

    void Chip8Display::draw(u_int8_t *sprite_base_addr, int X, int Y, int rows) {
        for (int row = 0; row < rows; row++) {
            int x = X;
            for (int col = 0; col < 8; col++) {
                bool pixel_in_sprite = ((sprite_base_addr[row] >> (7 - col)) & 1);
                if (pixels_on_screen[Y][x] ^= pixel_in_sprite) {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
                    SDL_RenderDrawPoint(renderer, x, Y);
                }
                if (x == 63) break;
                ++x;
            }
            if (Y == 31) break;
            ++Y;
        }
        SDL_RenderPresent(renderer);
    }

}
