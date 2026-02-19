#include <iostream>
#include <format>
#include <stdint.h>

#include "cpu.h"
#include "memory.h"


// Graphics
bool pixel_buffer[32][64] = { false };

// Memory
uint8_t memory[4096] = { 0x0 };

// Registers
uint16_t index_register = 0x0;
uint8_t registers[16] = { 0x0 };
uint16_t program_counter = 0x0;
uint8_t stack_pointer = 0x0;

/**
 * @brief Fetch the next instruction specified by the program counter, then
 * increment the PC.
 *
 * @return uint16_t The instruction to execute.
 */
uint16_t fetch() {
    uint16_t instruction = (memory[program_counter] << 8) | memory[program_counter + 1];
    program_counter += 2;

    return instruction;
}

/**
 * @brief Test if the specified nibble in the 16bit instruction equals some
 * value. Useful for decoding. The nibble at index 0 is the least significant
 * nibble.
 *
 * @param instr_bytes The instruction to test (16 bits or 4 nibbles).
 * @param nib_index The nibble index (0 is least-significant nibble).
 * @param nib_val Value to test for.
 * @return bool Return if the nibble at the index is equal to nib_val.
 */
bool test_instr_nibble(uint16_t instr_bytes, int nib_index, uint8_t nib_val) {
    return ((instr_bytes & (0xF << (4 * nib_index))) == (nib_val << (4 * nib_index)));
}

/**
 * @brief Decode the instruction and return an instruction context.
 *
 * @param instr_bytes The raw instruction bytes.
 * @return struct Instruction
 */
Instruction decode(uint16_t instr_bytes) {
    struct Instruction instr;

    // Collect potential instruction arguments for later use in execution.
    instr.n = instr_bytes & 0x000F;
    instr.nn = instr_bytes & 0x00FF;
    instr.nnn = instr_bytes & 0x0FFF;

    instr.x = (instr_bytes & 0x0F00) >> 8;
    instr.y = (instr_bytes & 0x00F0) >> 4;

    if (test_instr_nibble(instr_bytes, 3, 0x0)) {
        if (test_instr_nibble(instr_bytes, 2, 0x0)) {
            if (test_instr_nibble(instr_bytes, 0, 0x0)) {
                // 00E0
                instr.op_id = OP_CLEAR_SCREEN;
            } else {
                // 00EE
                instr.op_id = OP_RETURN;
            }
        } else {
            // 0NNN
            instr.op_id = OP_JUMP_SUBR;
        }
    } else if (test_instr_nibble(instr_bytes, 3, 0x1)) {
        // 1NNN
        instr.op_id = OP_JUMP_ADDR;
    } else if (test_instr_nibble(instr_bytes, 3, 0x2)) {
        // 2NNN
        instr.op_id = OP_CALL_SUBR;
    } else if (test_instr_nibble(instr_bytes, 3, 0x3)) {
        // 3XNN
        instr.op_id = OP_SKIP_VAL_EQ;
    } else if (test_instr_nibble(instr_bytes, 3, 0x4)) {
        // 4XNN
        instr.op_id = OP_SKIP_VAL_NEQ;
    } else if (test_instr_nibble(instr_bytes, 3, 0x5)) {
        // 5XY0
        instr.op_id = OP_SKIP_REG_EQ;
    } else if (test_instr_nibble(instr_bytes, 3, 0x6)) {
        // 6XNN
        instr.op_id = OP_SET_X;
    } else if (test_instr_nibble(instr_bytes, 3, 0x7)) {
        // 7XNN
        instr.op_id = OP_ADD_X;
    } else if (test_instr_nibble(instr_bytes, 3, 0x8)) {
        if (test_instr_nibble(instr_bytes, 0, 0x0)) {
            // 8XY0
            instr.op_id = OP_SET_X_Y;
        } else if (test_instr_nibble(instr_bytes, 0, 0x1)) {
            // 8XY1
            instr.op_id = OP_OR;
        } else if (test_instr_nibble(instr_bytes, 0, 0x2)) {
            // 8XY2
            instr.op_id = OP_AND;
        } else if (test_instr_nibble(instr_bytes, 0, 0x3)) {
            // 8XY3
            instr.op_id = OP_XOR;
        } else if (test_instr_nibble(instr_bytes, 0, 0x4)) {
            // 8XY4
            instr.op_id = OP_ADD_Y_TO_X;
        } else if (test_instr_nibble(instr_bytes, 0, 0x5)) {
            // 8XY5
            instr.op_id = OP_SUB_Y_X;
        } else if (test_instr_nibble(instr_bytes, 0, 0x6)) {
            // 8XY6
            instr.op_id = OP_SHIFT_RIGHT;
        } else if (test_instr_nibble(instr_bytes, 0, 0x7)) {
            // 8XY7
            instr.op_id = OP_SUB_X_Y;
        } else if (test_instr_nibble(instr_bytes, 0, 0xE)) {
            // 8XYE
            instr.op_id = OP_SHIFT_LEFT;
        }
    } else if (test_instr_nibble(instr_bytes, 3, 0x9)) {
        // 9XY0
        instr.op_id = OP_SKIP_REQ_NEQ;
    } else if (test_instr_nibble(instr_bytes, 3, 0xA)) {
        // ANNN
        instr.op_id = OP_SET_INDEX;
    } else if (test_instr_nibble(instr_bytes, 3, 0xB)) {
        // BNNN
        instr.op_id = OP_JUMP_OFFSET;
    } else if (test_instr_nibble(instr_bytes, 3, 0xC)) {
        // CXNN
        instr.op_id = OP_SET_X_RAND;
    } else if (test_instr_nibble(instr_bytes, 3, 0xD)) {
        // DXYN
        instr.op_id = OP_DRAW;
    } else if ((instr_bytes & 0xF0FF) == 0xE09E) {
        // EX9E
        instr.op_id = OP_SKIP_KP;
    } else if ((instr_bytes & 0xF0FF) == 0xE0A1) {
        // EXA1
        instr.op_id = OP_SKIP_NOT_KP;
    } else if ((instr_bytes & 0xF0FF) == 0xF007) {
        // FX07
        instr.op_id = OP_SET_X_DELAY;
    } else if ((instr_bytes & 0xF0FF) == 0xF00A) {
        // FX0A
        instr.op_id = OP_WAIT_KP;
    } else if ((instr_bytes & 0xF0FF) == 0xF015) {
        // FX15
        instr.op_id = OP_SET_DELAY_X;
    } else if ((instr_bytes & 0xF0FF) == 0xF018) {
        // FX18
        instr.op_id = OP_SET_SOUND_X;
    } else if ((instr_bytes & 0xF0FF) == 0xF01E) {
        // FX1E
        instr.op_id = OP_ADD_X_I;
    } else if ((instr_bytes & 0xF0FF) == 0xF029) {
        // FX29
        instr.op_id = OP_SET_I_SPRITE;
    } else if ((instr_bytes & 0xF0FF) == 0xF033) {
        // FX33
        instr.op_id = OP_WRITE_BCD;
    } else if ((instr_bytes & 0xF0FF) == 0xF055) {
        // FX55
        instr.op_id = OP_WRITE_REGS;
    } else if ((instr_bytes & 0xF0FF) == 0xF065) {
        // FX65
        instr.op_id = OP_READ_REGS;
    } else {
        std::cerr << "Could not identify opcode." << std::endl;
        instr.op_id = OP_UNDEFINED;
    }

    return instr;
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


void execute(Instruction instr) {
    switch (instr.op_id)
    {
    case OP_CLEAR_SCREEN:
        opcode_clear_screen(instr);
        break;
    case OP_JUMP_ADDR:
        opcode_jump_address(instr);
        break;
    case OP_SET_X:
        opcode_set_x(instr);
        break;
    case OP_ADD_X:
        opcode_add_x(instr);
        break;
    case OP_SET_INDEX:
        opcode_set_index(instr);
        break;
    case OP_DRAW:
        opcode_draw(instr);
        break;
    default:
        break;
    }
}

void cpu_loop() {
    // Fetch
    uint16_t instruction = fetch();

    std::cout << std::format("instr. {:04X} ; PC={:04X}", instruction, program_counter) << std::endl;

    // Decode
    Instruction instr = decode(instruction);

    std::cout << instr.op_id << std::endl;

    // Execute
    execute(instr);
}
