#include <stdint.h>
#include <cstdlib>
#include <format>

#include "logger.h"
#include "cpu.h"
#include "opcodes.h"
#include "memory.h"


void opcode_execute_routine(Instruction instr) {

}

void opcode_clear_screen(Instruction instr) {
    log_info(std::format("CLEAR_SCRN"));

    // TODO: It would be faster to copy a block of false values over this memory
    // space, instead of looping over each value.
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            pixel_buffer[i][j] = false;
        }
    }
}

void opcode_jump_subr(Instruction instr) {
    log_info(std::format("JUMP_SUBR"));
 // Don't implement
}

void opcode_jump_address(Instruction instr) {
    log_info(std::format("JUMP_ADDR 0x{:04X}", instr.nnn));

    program_counter = instr.nnn;
}

void opcode_return(Instruction instr) {
    log_info(std::format("RETURN", instr.nnn));

    program_counter = pop_stack();
}

void opcode_call_subr(Instruction instr) {
    log_info(std::format("CALL SUBR 0x{:04X}", instr.nnn));

    push_stack(program_counter);
    program_counter = instr.nnn;
}

void opcode_skip_val_eq(Instruction instr) {
    log_info(std::format("SKIP? {:02X} == {:02X}", registers[instr.x], instr.nn));

    if (registers[instr.x] == instr.nn) {
        program_counter += 2;
    }
}

void opcode_skip_val_neq(Instruction instr) {
    log_info(std::format("SKIP? {:02X} != {:02X}", registers[instr.x], instr.nn));

    if (registers[instr.x] != instr.nn) {
        program_counter += 2;
    }
}

void opcode_skip_reg_eq(Instruction instr) {
    log_info(std::format("SKIP? {:02X} == {:02X}", registers[instr.x], registers[instr.y]));

    if (registers[instr.x] == registers[instr.y]) {
        program_counter += 2;
    }
}

void opcode_skip_reg_neq(Instruction instr) {
    log_info(std::format("SKIP? {:02X} != {:02X}", registers[instr.x], registers[instr.y]));

    if (registers[instr.x] != registers[instr.y]) {
        program_counter += 2;
    }
}

void opcode_set_x(Instruction instr) {
    log_info(std::format("SET REG({:02X}) = {:02X}", instr.x, instr.nn));

    registers[instr.x] = instr.nn;
}

void opcode_add_x(Instruction instr) {
    log_info(std::format("ADD REG({:02X}) = {:02X}", instr.x, instr.nn));

    registers[instr.x] += instr.nn;
}

void opcode_add_x_to_y(Instruction instr) {
    log_info(std::format("ADD REG({:02X}) REG({:02X})", instr.y, instr.x));

    registers[instr.y] += registers[instr.x];
}

void opcode_set_x_y(Instruction instr) {
    log_info(std::format("SET REG({:02X}) = REG({:02X})", instr.x, instr.y));

    registers[instr.x] = registers[instr.y];
}

void opcode_or(Instruction instr) {
    log_info(std::format("OR REG({:02X}) REG({:02X})", instr.x, instr.y));

    registers[instr.x] = registers[instr.x] | registers[instr.y];
}

void opcode_and(Instruction instr) {
    log_info(std::format("AND REG({:02X}) REG({:02X})", instr.x, instr.y));

    registers[instr.x] = registers[instr.x] & registers[instr.y];
}

void opcode_xor(Instruction instr) {
    log_info(std::format("XOR REG({:02X}) REG({:02X})", instr.x, instr.y));

    registers[instr.x] = registers[instr.x] ^ registers[instr.y];
}

void opcode_add_y_to_x(Instruction instr) {
    log_info(std::format("ADD REG({:02X}) REG({:02X})", instr.x, instr.y));

    registers[instr.x] += registers[instr.y];
}

void opcode_sub_y_from_x(Instruction instr) {
    log_info(std::format("SUB REG({:02X}) REG({:02X})", instr.x, instr.y));

    registers[instr.x] -= registers[instr.y];
}

void opcode_sub_x_from_y(Instruction instr) {
    log_info(std::format("OR REG({:02X}) REG({:02X})", instr.y, instr.x));

    registers[instr.y] -= registers[instr.x];
}

void opcode_shift_right(Instruction instr) {
    log_info(std::format("RSHIFT REG({:02X})", instr.y));

    registers[instr.x] = registers[instr.y] > 1;
}

void opcode_shift_left(Instruction instr) {
    log_info(std::format("LSHIFT REG({:02X})", instr.y));

    registers[instr.x] = registers[instr.y] < 1;
}

void opcode_set_index(Instruction instr) {
    log_info(std::format("SET INDEX = 0x{:02X}", instr.y));

    index_register = instr.nnn;
}

void opcode_jump_offset(Instruction instr) {
    log_info(std::format("JUMP_OFFSET {:04X} + {:02X}", instr.nnn, registers[0]));

    program_counter = instr.nnn + registers[0];
}

void opcode_set_x_random(Instruction instr) {
    log_info(std::format("SET_RAND"));

    registers[instr.x] = (rand() % 255) & instr.nn;
}

void opcode_draw(Instruction instr) {
    log_info(std::format("DRAW N={} X={} Y={}", instr.n, registers[instr.x], registers[instr.y]));

    uint8_t height = instr.n;
    uint16_t sprite_addr = index_register;
    uint8_t x_start = registers[instr.x] % 64;
    uint8_t y_start = registers[instr.y] % 32;

    // Reset the flag register
    registers[0xF] = 0x0;

    // Write the sprite to the pixel buffer, it is always 8 bits/pixels wide.
    for (int row = 0; row < height; row++) {
        uint8_t pixel_row = memory[sprite_addr + row];

        for (int i = 0; i < 8; i++) {
            // Set flag to true if an activated pixel was flipped.
            if (pixel_buffer[y_start + row][x_start + i] & (pixel_row & (0b1 << i)) >> i) {
                registers[0xF] = 1;
            }

            pixel_buffer[y_start + row][x_start + i] ^= (pixel_row & (0b1 << i)) >> i;
        }
    }
}

void opcode_skip_kp(Instruction instr) {
    log_info(std::format("SKIP_IF_KP"));

}

void opcode_skip_not_kp(Instruction instr) {
    log_info(std::format("SKIP_IF_NOT_KP"));
}

// 0xF...
void opcode_set_x_to_delay(Instruction instr) {
    log_info(std::format("SET REG({:02X}) DELAY", instr.x));

    registers[instr.x] = delay_timer;
}

void opcode_wait_keypress(Instruction instr) {
    log_info(std::format("WAIT_KP"));
}

void opcode_set_delay_to_x(Instruction instr) {
    log_info(std::format("SET DELAY REG({:02X})", instr.x));

    delay_timer = registers[instr.x];
}

void opcode_set_sound_to_x(Instruction instr) {
    log_info(std::format("SET SOUND REG({:02X})", instr.x));

    sound_timer = registers[instr.x];
}

void opcode_add_x_to_index(Instruction instr) {
    log_info(std::format("ADD INDEX REG({:02X})", instr.x));

    index_register += registers[instr.x];
}

void opcode_set_index_sprite(Instruction instr) {
    log_info(std::format("SET_SPRITE REG({:02X})", instr.x));

    uint8_t hex_char = registers[instr.x] & 0x0F;

    index_register = 0x050 + (hex_char * 5);
}

void opcode_write_bcd(Instruction instr) {
    log_info(std::format("WRITE_BCD"));

}

void opcode_write_regs(Instruction instr) {
    log_info(std::format("WRITE_MEMORY"));

}

void opcode_read_regs(Instruction instr) {
    log_info(std::format("READ_MEMORY"));

}