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

}
