#include <iostream>
#include <ostream>
#include <istream>
#include <stdint.h>
#include <stdio.h>

#include "SDL2/SDL.h"


// Memory
uint8_t memory[4096] = { 0x0 };

// Registers
uint16_t index_register = 0x0;
uint8_t registers[16] = { 0x0 };
uint16_t program_counter = 0x0;
uint8_t stack_pointer = 0x0;

// Timers
uint8_t delay_timer = 0x0;
uint8_t sound_timer = 0x0;


// SDL objects
SDL_Window* window;
SDL_Renderer* renderer;


void init_SDL() {
    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow("CHIP8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);

    if (renderer == NULL) {
        std::cerr << "Could not initialize renderer.\n" << std::endl;

        const char* error_msg = SDL_GetError();

        std::cout << error_msg << std::endl;
    }

    // Set default color to black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}


void close_SDL() {
    // Deallocate the SDL objects
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    // Set the pointers to ensure they can't be used.
    window = NULL;
    renderer = NULL;

    SDL_Quit();
}


void write_CHIP8_buffer() {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawPoint(renderer, 32, 16);
}


void cpu_loop() {
    // Fetch
    uint16_t instruction = (memory[program_counter] << 8) | memory[program_counter + 1];

    std::cout << instruction << std::endl;

    // Decode
    // Execute
}


void render() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Draw the CHIP8 pixel buffer
    SDL_RenderSetLogicalSize(renderer, 64, 32);
    write_CHIP8_buffer();

    SDL_RenderSetLogicalSize(renderer, 640, 320);
    SDL_RenderPresent(renderer);
}


int main() {
    init_SDL();

    bool quit = false;
    SDL_Event e;
    while (!quit) {
        // Update the emulator state.
        cpu_loop();

        // Handle all of the rendering.
        render();

        // Check for events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
    }

    close_SDL();

    return 0;
}
