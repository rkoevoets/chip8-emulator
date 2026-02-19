#include <stdint.h>

#include "cpu.h"
#include "opcodes.h"
#include "memory.h"


void opcode_clear_screen(Instruction instr) {
    // TODO: It would be faster to copy a block of false values over this memory
    // space, instead of looping over each value.
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            pixel_buffer[i][j] = false;
        }
    }
}

void opcode_jump_address(Instruction instr) {
    program_counter = instr.nnn;
}

void opcode_set_x(Instruction instr) {
    registers[instr.x] = instr.nn;
}

void opcode_add_x(Instruction instr) {
    registers[instr.x] += instr.nn;
}

void opcode_set_index(Instruction instr) {
    index_register = instr.nnn;
}

void opcode_draw(Instruction instr) {
    uint8_t height = instr.n;
    uint16_t sprite_addr = index_register;
    uint8_t x_start = registers[instr.x];
    uint8_t y_start = registers[instr.y];

    // Reset the flag register
    registers[0xF] = 0x0;

    // Write the sprite to the pixel buffer, it is always 8 bits/pixels wide.
    for (int row = 0; row < height; row++) {
        uint8_t pixel_row = memory[sprite_addr + row];

        for (int i = 0; i < 8; i++) {
            pixel_buffer[y_start + row][x_start + i] ^= (pixel_row & (0b1 << i)) >> i;
        }
    }
}
