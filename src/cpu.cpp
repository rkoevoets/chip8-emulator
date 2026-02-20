#include <iostream>
#include <format>
#include <stdint.h>

#include "cpu.h"
#include "opcodes.h"
#include "memory.h"
#include "logger.h"


// Graphics
bool pixel_buffer[32][64] = { false };

// Memory
uint8_t memory[4096] = { 0x0 };

// Registers
uint16_t index_register = 0x0;
uint8_t registers[16] = { 0x0 };
uint16_t program_counter = 0x200;

// Timers
uint8_t delay_timer = 0x0;
uint8_t sound_timer = 0x0;

// Stack
uint8_t stack_pointer = 0x0;
uint16_t stack[16] = {0x0};


void push_stack(uint16_t val) {
    stack[stack_pointer] = val;

    stack_pointer++;
}

uint16_t pop_stack() {
    stack_pointer--;

    return stack[stack_pointer];
}

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


void execute(Instruction instr) {
    switch (instr.op_id)
    {
    case OP_EXEC_ROUTINE:
        opcode_execute_routine(instr);
        break;
    case OP_CLEAR_SCREEN:
        opcode_clear_screen(instr);
        break;
    case OP_JUMP_SUBR:
        opcode_jump_subr(instr);
        break;
    case OP_JUMP_ADDR:
        opcode_jump_address(instr);
        break;
    case OP_RETURN:
        opcode_return(instr);
        break;
    case OP_CALL_SUBR:
        opcode_call_subr(instr);
        break;
    case OP_SKIP_VAL_EQ:
        opcode_skip_val_eq(instr);
        break;
    case OP_SKIP_VAL_NEQ:
        opcode_skip_val_neq(instr);
        break;
    case OP_SKIP_REG_EQ:
        opcode_skip_reg_eq(instr);
        break;
    case OP_SKIP_REQ_NEQ:
        opcode_skip_reg_neq(instr);
        break;
    case OP_SET_INDEX:
        opcode_set_index(instr);
        break;
    case OP_SET_X:
        opcode_set_x(instr);
        break;
    case OP_ADD_X:
        opcode_add_x(instr);
        break;
    case OP_ADD_X_TO_Y:
        opcode_add_x_to_y(instr);
        break;
    case OP_SET_X_Y:
        opcode_set_x_y(instr);
        break;
    case OP_OR:
        opcode_or(instr);
        break;
    case OP_AND:
        opcode_and(instr);
        break;
    case OP_XOR:
        opcode_xor(instr);
        break;
    case OP_ADD_Y_TO_X:
        opcode_add_y_to_x(instr);
        break;
    case OP_SUB_Y_X:
        opcode_sub_y_from_x(instr);
        break;
    case OP_SUB_X_Y:
        opcode_sub_x_from_y(instr);
        break;
    case OP_SHIFT_RIGHT:
        opcode_shift_right(instr);
        break;
    case OP_SHIFT_LEFT:
        opcode_shift_left(instr);
        break;
    case OP_JUMP_OFFSET:
        opcode_jump_offset(instr);
        break;
    case OP_SET_X_RAND:
        opcode_set_x_random(instr);
        break;
    case OP_DRAW:
        opcode_draw(instr);
        break;
    case OP_SKIP_KP:
        opcode_skip_kp(instr);
        break;
    case OP_SKIP_NOT_KP:
        opcode_skip_not_kp(instr);
        break;
    case OP_SET_X_DELAY:
        opcode_set_x_to_delay(instr);
        break;
    case OP_WAIT_KP:
        opcode_wait_keypress(instr);
        break;
    case OP_SET_DELAY_X:
        opcode_set_delay_to_x(instr);
        break;
    case OP_SET_SOUND_X:
        opcode_set_sound_to_x(instr);
        break;
    case OP_ADD_X_I:
        opcode_add_x_to_index(instr);
        break;
    case OP_SET_I_SPRITE:
        opcode_set_index_sprite(instr);
        break;
    case OP_WRITE_BCD:
        opcode_write_bcd(instr);
        break;
    case OP_WRITE_REGS:
        opcode_write_regs(instr);
        break;
    case OP_READ_REGS:
        opcode_read_regs(instr);
        break;
    default:
        break;
    }
}

void cpu_loop() {
    // Fetch
    uint16_t opcode = fetch();

    // Decode
    Instruction instr = decode(opcode);

    // Execute
    execute(instr);
}
