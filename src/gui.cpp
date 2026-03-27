#include <iostream>
#include <format>
#include <SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include "gui.h"
#include "cpu.h"
#include "logger.h"
#include "memory.h"
#include "config.h"


// Configurable options
const int WINDOW_HEIGHT = 720;
const int WINDOW_WIDTH = 1280;

const int EMU_SCALE = 5;
const int EMU_HEIGHT = 64;
const int EMU_WIDTH = 128;


// Sound
bool beep_playing = false;


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
void GUI::setup_audio() {
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

/**
 * @brief Initialize SDL objects.
 *
 */
void GUI::setup_GUI() {
    SDL_Init(SDL_INIT_EVERYTHING);

    m_window = SDL_CreateWindow("CHIP8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    m_renderer = SDL_CreateRenderer(m_window, -1, 0);

    if (m_renderer == NULL) {
        std::cerr << "Could not initialize renderer.\n" << std::endl;

        const char* error_msg = SDL_GetError();

        std::cout << error_msg << std::endl;
    }

    // Set default color to black
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);

    // Create the ImGui objects
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForSDLRenderer(m_window, m_renderer);
    ImGui_ImplSDLRenderer2_Init(m_renderer);

    // Create a texture for the emulator screen
    m_chip8_texture = SDL_CreateTexture(m_renderer,
        SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
}


/**
 * @brief Deallocate and close all SDL objects. Then quit SDL.
 *
 */
void GUI::close_GUI() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Deallocate the SDL objects
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);

    // Set the pointers to ensure they can't be used.
    m_window = NULL;
    m_renderer = NULL;

    SDL_Quit();
}

/**
 * @brief Write all pixel values of the CHIP8 graphics to the screen.
 *
 */
void GUI::write_CHIP8_buffer() {
    uint32_t colors[64 * 32];

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            if (pixel_buffer[i][j]) {
                colors[i * 64 + j] = 0x346856;
            } else {
                colors[i * 64 + j] = 0x88C070;
            }
        }
    }

    SDL_UpdateTexture(m_chip8_texture, nullptr, colors, 64 * sizeof(uint32_t));

    bool center_screen = false;

    if (center_screen) {
    // This code centers the CHIP8 screen to the parent window.
    SDL_Rect dest_rect{
        (WINDOW_WIDTH / 2) - ((EMU_WIDTH * EMU_SCALE) / 2),
        (WINDOW_HEIGHT / 2) - ((EMU_HEIGHT * EMU_SCALE) / 2),
        EMU_WIDTH * EMU_SCALE, EMU_HEIGHT * EMU_SCALE};

        SDL_RenderCopy(m_renderer, m_chip8_texture, NULL, &dest_rect);
    } else {
        SDL_Rect dest_rect{
        0,
        0,
        EMU_WIDTH * EMU_SCALE, EMU_HEIGHT * EMU_SCALE};

        SDL_RenderCopy(m_renderer, m_chip8_texture, NULL, &dest_rect);
    }
}

void GUI::render_gui_controls() {
    ImGui::Begin("Controls");

    if(ImGui::Button(run_fast ? "Stop" : "Run")) {
        run_fast ^= true;
    }

    ImGui::SameLine();

    if(ImGui::Button("Step")) {
        execute_next = true;
    }

    ImGui::End();
}

void GUI::render_gui_cpu() {
    ImGui::Begin("CPU");

    ImGui::Checkbox("Sound?", &beep_playing);
    ImGui::Text(std::format("Delay timer {:02X}", delay_timer).c_str());
    ImGui::Text(std::format("Sound timer {:02X}", sound_timer).c_str());

    ImGui::Text(std::format("Program Counter {:02X}", program_counter).c_str());
    ImGui::Text(std::format("Index register {:02X}", index_register).c_str());

    for (int i = 0; i < 16; i++) {
        ImGui::InputScalar(std::format("V{:01X}", i).c_str(), ImGuiDataType_U8, (registers + i));
    }

    ImGui::End();
}

void GUI::render_gui_memory() {
    ImGui::Begin("Memory");

    // Display a grid of memory locations (16 wide?)

    ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
    uint16_t address;
    if (ImGui::BeginTable("memory", 17, flags)) {
        for (int row = 0; row < 256; row++) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text(std::format("{:02X}x", row).c_str());
            for (int col = 0; col < 16; col++) {
                address = row * 16 + col;

                ImGui::TableSetColumnIndex(col + 1);

                // Highlight memory containing positive values
                if (memory[address] > 0x0) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.2f)));
                } else {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.0f)));
                }

                // Highlight where the PC is pointing in memory
                if (address == program_counter || address == (program_counter + 1)) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.8f)));
                }

                if (address == index_register) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 0.8f)));
                }

                ImGui::Text(std::format("{:02X}", memory[address]).c_str());
            }
        }
    }

    ImGui::EndTable();

    ImGui::End();
}

/**
 * @brief Main render loop of the emulator. Handles the rendering of the CHIP8
 * pixels to the screen.
 */
void GUI::render() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // add imgui windows here
    render_gui_cpu();
    render_gui_controls();
    render_gui_memory();

    // After creating all ImGui windows, this will finalize the ImGui render data
    ImGui::Render();

    SDL_RenderSetLogicalSize(m_renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

    SDL_RenderClear(m_renderer);

    write_CHIP8_buffer();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), m_renderer);

    SDL_RenderPresent(m_renderer);
}

uint8_t GUI::translate_sdl_to_scancode(SDL_Scancode scancode) {
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

void GUI::sync_with_clock(double frame_duration) {
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

void GUI::handle_key_events() {
    // Check for events.
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        ImGui_ImplSDL2_ProcessEvent(&e);

        if (e.type == SDL_QUIT) {
            running = false;
        } else if (e.type == SDL_KEYDOWN) {
            keys_pressed[translate_sdl_to_scancode(e.key.keysym.scancode)] = true;
        } else if (e.type == SDL_KEYUP) {
            keys_pressed[translate_sdl_to_scancode(e.key.keysym.scancode)] = false;
            keys_released[translate_sdl_to_scancode(e.key.keysym.scancode)] = true;
        }
    }
}


void GUI::run_emulator() {
    // Create an SDL timer to keep track of time
    uint64_t frame_start_time, frame_end_time, frame_time_delta;
    frame_time_delta = 0;

    while (running) {
        frame_start_time = SDL_GetTicks64();

        // Decide how to execute instruction, depends on whether the
        // debugger is being used to step through or it's fast execution.
        if (run_fast) {
            for (int i = 0; i < INSTR_PER_FRAME; i++) {
                cpu_execute_instruction();
            }
        } else if (execute_next) {
            cpu_execute_instruction();

            execute_next = false;
        }

        // Handle all of the rendering.
        render();

        // Reset key releases
        for (int i = 0; i < 16; i++) {
            keys_released[i] = false;
        }

        // Check for key press and release events
        handle_key_events();

        // Assumes this main loop runs at 60Hz
        beep_playing = sound_timer > 0;

        // Determine how much time has passed in this frame (in milliseconds)
        frame_end_time = SDL_GetTicks64();
        frame_time_delta = frame_end_time - frame_start_time;

        // Sync the loop, so it stays in sync with clock as closely as
        // possible.
        sync_with_clock(frame_time_delta);
    }
}


void GUI::start_gui() {
    // Initialize SDL, audio and other things.
    setup_GUI();
    setup_audio();
    load_fonts();

    run_emulator();

    // If the emulator is closed, then clean up all the allocated resources.
    close_GUI();
}