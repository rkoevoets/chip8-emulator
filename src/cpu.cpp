#include <iostream>
#include <format>
#include <stdint.h>

#include "cpu.h"
#include "memory.h"

// Memory
uint8_t memory[4096] = { 0x0 };


// Registers
uint16_t index_register = 0x0;
uint8_t registers[16] = { 0x0 };
uint16_t program_counter = 0x0;
uint8_t stack_pointer = 0x0;


void cpu_loop() {
    // Fetch
    uint16_t instruction = (memory[program_counter] << 8) | memory[program_counter + 1];
    program_counter += 2;

    std::cout << std::format("instr. {:04X}", instruction) << std::endl;

    // Decode
    // Execute
}
