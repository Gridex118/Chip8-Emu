#include "chip8.hpp"
#include <iostream>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 320

namespace chip8 {

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
        screen_surface = SDL_GetWindowSurface(window);
        return 0;
    }

}
