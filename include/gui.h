#pragma once

#include <SDL.h>

class GUI {
    // SDL objects
    SDL_Texture* m_chip8_texture;
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;

    // State
    bool running = true;
    bool run_fast = false;
    bool execute_next = false;

    public:
        void start_gui();
        void setup_audio();
        void setup_GUI();
        void close_GUI();

        void run_emulator();

        void render();
        void write_CHIP8_buffer();
        void render_gui_controls();
        void render_gui_cpu();
        void render_gui_memory();

        uint8_t translate_sdl_to_scancode(SDL_Scancode scancode);
        void sync_with_clock(double frame_duration);
        void handle_key_events();
};