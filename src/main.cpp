#include <iostream>
#include <ostream>
#include <format>
#include <istream>
#include <fstream>
#include <string>
#include <map>
#include <stdint.h>
#include <stdio.h>
#include <SDL.h>
#include "imgui.h"

#include "cpu.h"
#include "memory.h"
#include "logger.h"


const int ROM_MAX_SIZE = 4096;

// Timing
const float TIMER_FREQ = 60.f;
const int INSTR_PER_FRAME = 30;

// Sound
bool beep_playing = false;

// SDL objects
SDL_Window* window;
SDL_Renderer* renderer;

/**
 * @brief Generate a beep noise.
 */
void generate_beep(void* userdata, Uint8* stream, int len_bytes) {
    static double phase = 0.f;

    // Audio wave characteristics
    const double SOUND_AMP = 500;
    const double SOUND_TONE = 440; // Hz

    Sint16* buffer = (Sint16*) stream;

    int sample_count = len_bytes / sizeof(Sint16);

    if (beep_playing) {
        for(int i = 0; i < sample_count; i++) {
            buffer[i] = SOUND_AMP * SDL_sin(phase * 2.0f * M_PI);
            phase += SOUND_TONE / 44100;

            if (phase >= 1.0f) {
                phase -= 1.0f;
            }
        }
    } else {
        for(int i = 0; i < sample_count; i++) {
            buffer[i] = 0;
        }
    }
}

/**
 * @brief This function opens the audio devices and attaches a callback.
 */
void setup_audio() {
    // Define the audio specifications
    SDL_AudioSpec spec = {0};
    spec.freq = 44100;
    spec.format = AUDIO_S16SYS;
    spec.channels = 1;
    spec.samples = 4096;
    spec.callback = generate_beep;

    // Retrieve the audio device ID
    SDL_AudioDeviceID device_id = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);

    SDL_PauseAudioDevice(device_id, 0); // Starts audio
}


void print_memory() {
    for (int i = 0; i < 4096; i+=2) {
        std::cout << std::format("{:04x}: {:02X}{:02X}", i, memory[i], memory[i+1]) << std::endl;
    }
}

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
void read_rom(std::string filepath, uint16_t offset) {
    std::ifstream file(filepath, std::ios_base::binary);

    if (!file) {
        log_err("Could not read ROM.");
        exit(1);
    }

    file.read((char*) (memory + offset), ROM_MAX_SIZE);
}

/**
 * @brief Write all pixel values of the CHIP8 graphics to the screen.
 *
 */
void write_CHIP8_buffer() {
    SDL_SetRenderDrawColor(renderer, 48, 98, 48, 255);

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
    SDL_SetRenderDrawColor(renderer, 155, 188, 15, 255);
    SDL_RenderClear(renderer);

    // Draw the CHIP8 pixel buffer
    SDL_RenderSetLogicalSize(renderer, 64, 32);
    write_CHIP8_buffer();

    SDL_RenderSetLogicalSize(renderer, 640, 320);
    SDL_RenderPresent(renderer);
}

/**
 * @brief Load font data into the CHIP8 memory.
 *
 */
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

uint8_t translate_sdl_to_scancode(SDL_Scancode scancode) {
    switch (scancode)
    {
    case SDL_SCANCODE_X:
        return 0x0;
        break;
    case SDL_SCANCODE_1:
        return 0x1;
        break;
    case SDL_SCANCODE_2:
        return 0x2;
        break;
    case SDL_SCANCODE_3:
        return 0x3;
        break;
    case SDL_SCANCODE_Q:
        return 0x4;
        break;
    case SDL_SCANCODE_W:
        return 0x5;
        break;
    case SDL_SCANCODE_E:
        return 0x6;
        break;
    case SDL_SCANCODE_A:
        return 0x7;
        break;
    case SDL_SCANCODE_S:
        return 0x8;
        break;
    case SDL_SCANCODE_D:
        return 0x9;
        break;
    case SDL_SCANCODE_Z:
        return 0xA;
        break;
    case SDL_SCANCODE_C:
        return 0xB;
        break;
    case SDL_SCANCODE_4:
        return 0xC;
        break;
    case SDL_SCANCODE_R:
        return 0xD;
        break;
    case SDL_SCANCODE_F:
        return 0xE;
        break;
    case SDL_SCANCODE_V:
        return 0xF;
        break;
    default:
        log_err("Did not recognize SDL_SCANCODE input, returning zero.");
        return 0x0;
        break;
    }
}

void sync_with_clock(double frame_duration) {
    static float time_to_delay_ms = 0.f;

    time_to_delay_ms += (1000.f / TIMER_FREQ) - frame_duration;

    // It is not possible to have a negative desired delay, so clamp it to 0.
    if (time_to_delay_ms < 0) {
        time_to_delay_ms = 0;
    }

    // Wait if necessary to keep timing, don't if the time has surpassed the FPS time.
    if (time_to_delay_ms >= 1.f) {
        int temp = (int) time_to_delay_ms;  // Casting done to round towards zero, which we need in this case

        SDL_Delay(temp);

        time_to_delay_ms -= temp;
    }
}

/**
 * @brief Handle graphics setup, arguments passed and initialize the emulator.
 */
int main(int argc, char *argv[]) {
    std::string rom_path;

    open_log_file();

    if (argc != 2) {
        log_err("No ROM provided, please provide a ROM path...");

        return 1;
    } else {
        rom_path = argv[1];
    }

    // Initialize SDL
    init_SDL();

    // Prime the memory with the provided ROM and font data.
    read_rom(rom_path, 0x200);
    load_fonts();

    // Create an SDL timer to keep track of time
    uint64_t frame_start_time, frame_end_time, frame_time_passed;
    frame_time_passed = 0;

    // Keep track of the amount of frames passed for the timers
    uint32_t timer_frames_passed = 0;

    // Start the audio streaming
    setup_audio();

    bool quit = false;
    SDL_Event e;
    while (!quit) {
        // Update the emulator state.
        frame_start_time = SDL_GetTicks64();

        for (int i = 0; i < INSTR_PER_FRAME; i++) {
            update_emulator_state();
        }

        // Handle all of the rendering.
        render();

        // Reset key releases
        for (int i = 0; i < 16; i++) {
            keys_released[i] = false;
        }

        // Check for events.
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                keys_pressed[translate_sdl_to_scancode(e.key.keysym.scancode)] = true;
            } else if (e.type == SDL_KEYUP) {
                keys_pressed[translate_sdl_to_scancode(e.key.keysym.scancode)] = false;
                keys_released[translate_sdl_to_scancode(e.key.keysym.scancode)] = true;
            }
        }

        // Assumes this main loop runs at 60Hz
        beep_playing = sound_timer > 0;

        if (delay_timer > 0) delay_timer--;
        if (sound_timer > 0) sound_timer--;

        // Determine how much time has passed in this frame
        frame_end_time = SDL_GetTicks64();
        frame_time_passed = frame_end_time - frame_start_time;

        // Sync the loop, so it stays in sync with clock as closely as
        // possible.
        sync_with_clock(frame_time_passed);

        timer_frames_passed++;
    }

    close_SDL();
    close_log_file();

    return 0;
}
