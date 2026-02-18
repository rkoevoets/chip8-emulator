#include <iostream>
#include <ostream>
#include <format>
#include <istream>
#include <fstream>
#include <string>
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

/**
 * @brief Read a ROM into a memory array starting at mem_start.
 *
 * @param filepath file path to the ROM
 * @param mem_start starting address of the memory array
 */
void read_rom(std::string filepath, uint8_t* mem_start) {
    std::ifstream file{filepath};
    uint8_t next_byte;

    int i = 0;
    while (file) {
        next_byte = file.get();

        std::cout << std::format("instruction {:02X}", next_byte) << std::endl;

        mem_start[i] = next_byte;

        i++;
    }
}

void write_CHIP8_buffer() {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawPoint(renderer, 32, 16);
}


void cpu_loop() {
    // Fetch
    uint16_t instruction = (memory[program_counter] << 8) | memory[program_counter + 1];

    // Decode
    // Execute
}

/**
 * @brief Main render loop of the emulator. Handles the rendering of the CHIP8
 * pixels to the screen.
 *
 */
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

    // Read the provided rom into memory
    read_rom("roms/test_opcode.ch8", memory);

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
