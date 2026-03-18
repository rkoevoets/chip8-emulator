#include <iostream>
#include <ostream>
#include <format>
#include <istream>
#include <fstream>
#include <string>
#include <map>
#include <stdint.h>
#include <stdio.h>

#include "gui.h"
#include "cpu.h"
#include "memory.h"
#include "logger.h"


const int ROM_MAX_SIZE = 4096;


void print_memory() {
    for (int i = 0; i < 4096; i+=2) {
        std::cout << std::format("{:04x}: {:02X}{:02X}", i, memory[i], memory[i+1]) << std::endl;
    }
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
 * @brief Handle graphics setup, arguments passed and initialize the emulator.
 */
int main(int argc, char *argv[]) {
    GUI gui;
    std::string rom_path;

    open_log_file();

    // Check if a ROM path was provided.
    if (argc != 2) {
        log_err("No ROM provided, please provide a ROM path...");

        return 1;
    } else {
        rom_path = argv[1];
    }

    // Prime the memory with the provided ROM and font data.
    read_rom(rom_path, 0x200);

    // Start the graphical interface and the emulator with it
    gui.start_gui();

    close_log_file();

    return 0;
}
