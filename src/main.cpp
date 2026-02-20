#include <iostream>
#include <ostream>
#include <format>
#include <istream>
#include <fstream>
#include <string>
#include <stdint.h>
#include <stdio.h>

#include "SDL2/SDL.h"

#include "cpu.h"
#include "memory.h"
#include "logger.h"

// SDL objects
SDL_Window* window;
SDL_Renderer* renderer;


/**
 * @brief Initialize SDL objects.
 *
 */
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

/**
 * @brief Deallocate and close all SDL objects. Then quit SDL.
 *
 */
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

    int i = 0;
    while (file) {
        mem_start[i] = file.get();

        i++;
    }
}

/**
 * @brief Write all pixel values of the CHIP8 graphics to the screen.
 *
 */
void write_CHIP8_buffer() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            if (pixel_buffer[i][j])
                SDL_RenderDrawPoint(renderer, j, i);
        }
    }
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

void load_fonts() {
    const uint8_t FONT_DATA[] = {
                0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                0x20, 0x60, 0x20, 0x20, 0x70, // 1
                0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                0xF0, 0x80, 0xF0, 0x80, 0x80  // F}
    };

    std::copy(FONT_DATA, FONT_DATA + (5 * 16), memory + 0x050);
}

int main(int argc, char *argv[]) {
    std::string rom_path;

    if (argc != 2) {
        log_err("No ROM provided, please provide a ROM path...");

        return 1;
    } else {
        rom_path = argv[1];
    }

    // Initialize SDL
    init_SDL();

    // Read the provided rom into memory
    read_rom(rom_path, memory + 0x200);
    load_fonts();

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
