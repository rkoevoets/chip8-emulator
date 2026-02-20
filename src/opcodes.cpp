#include <stdint.h>
#include <cstdlib>

#include "cpu.h"
#include "opcodes.h"
#include "memory.h"


void opcode_execute_routine(Instruction instr) {

}

void opcode_clear_screen(Instruction instr) {
    // TODO: It would be faster to copy a block of false values over this memory
    // space, instead of looping over each value.
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            pixel_buffer[i][j] = false;
        }
    }
}

void opcode_jump_subr(Instruction instr) {
 // Don't implement
}

void opcode_jump_address(Instruction instr) {
    program_counter = instr.nnn;
}

void opcode_return(Instruction instr) {
    program_counter = pop_stack();
}

void opcode_call_subr(Instruction instr) {
    push_stack(program_counter);
    program_counter = instr.nnn;
}

void opcode_skip_val_eq(Instruction instr) {
    if (registers[instr.x] == instr.nn) {
        program_counter += 2;
    }
}

void opcode_skip_val_neq(Instruction instr) {
    if (registers[instr.x] != instr.nn) {
        program_counter += 2;
    }
}

void opcode_skip_reg_eq(Instruction instr) {
    if (registers[instr.x] == registers[instr.y]) {
        program_counter += 2;
    }
}

void opcode_skip_reg_neq(Instruction instr) {
    if (registers[instr.x] != registers[instr.y]) {
        program_counter += 2;
    }
}

void opcode_set_x(Instruction instr) {
    registers[instr.x] = instr.nn;
}

void opcode_add_x(Instruction instr) {
    registers[instr.x] += instr.nn;
}

void opcode_add_x_to_y(Instruction instr) {
    registers[instr.y] += registers[instr.x];
}

void opcode_set_x_y(Instruction instr) {
    registers[instr.x] = registers[instr.y];
}

void opcode_or(Instruction instr) {
    registers[instr.x] = registers[instr.x] | registers[instr.y];
}

void opcode_and(Instruction instr) {
    registers[instr.x] = registers[instr.x] & registers[instr.y];
}

void opcode_xor(Instruction instr) {
    registers[instr.x] = registers[instr.x] ^ registers[instr.y];
}

void opcode_add_y_to_x(Instruction instr) {
    registers[instr.x] += registers[instr.y];
}

void opcode_sub_y_from_x(Instruction instr) {
    registers[instr.x] -= registers[instr.y];
}

void opcode_sub_x_from_y(Instruction instr) {
    registers[instr.y] -= registers[instr.x];
}

void opcode_shift_right(Instruction instr) {
    registers[instr.x] = registers[instr.y] > 1;
}

void opcode_shift_left(Instruction instr) {
    registers[instr.x] = registers[instr.y] < 1;
}

void opcode_set_index(Instruction instr) {
    index_register = instr.nnn;
}

void opcode_jump_offset(Instruction instr) {
    program_counter = instr.nnn + registers[0];
}

void opcode_set_x_random(Instruction instr) {
    registers[instr.x] = (rand() % 255) & instr.nn;
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

void opcode_skip_kp(Instruction instr) {

}

void opcode_skip_not_kp(Instruction instr) {

}

// 0xF...
void opcode_set_x_to_delay(Instruction instr) {
    registers[instr.x] = delay_timer;
}

void opcode_wait_keypress(Instruction instr) {

}

void opcode_set_delay_to_x(Instruction instr) {
    delay_timer = registers[instr.x];
}

void opcode_set_sound_to_x(Instruction instr) {
    sound_timer = registers[instr.x];
}

void opcode_add_x_to_index(Instruction instr) {
    index_register += registers[instr.x];
}

void opcode_set_index_sprite(Instruction instr) {

}

void opcode_write_bcd(Instruction instr) {

}

void opcode_write_regs(Instruction instr) {

}

void opcode_read_regs(Instruction instr) {

}