#pragma once


/**
 * @brief An enum to identify each instruction easily. Also provides an enum
 * for undefined opcodes if necessary.
 *
 */
enum Opcode {
    OP_EXEC_ROUTINE,
    OP_CLEAR_SCREEN,
    OP_JUMP_SUBR,
    OP_JUMP_ADDR,
    OP_RETURN,
    OP_CALL_SUBR,
    OP_SKIP_VAL_EQ,
    OP_SKIP_VAL_NEQ,
    OP_SKIP_REG_EQ,
    OP_SKIP_REQ_NEQ,
    OP_SET_X,
    OP_ADD_X,
    OP_ADD_X_TO_Y,
    OP_SET_X_Y,
    OP_OR,
    OP_AND,
    OP_XOR,
    OP_ADD_Y_TO_X,
    OP_SUB_Y_X,
    OP_SUB_X_Y,
    OP_SHIFT_RIGHT,
    OP_SHIFT_LEFT,
    OP_SET_INDEX,
    OP_JUMP_OFFSET,
    OP_SET_X_RAND,
    OP_DRAW,
    OP_SKIP_KP,
    OP_SKIP_NOT_KP,
    OP_SET_X_DELAY,
    OP_WAIT_KP,
    OP_SET_DELAY_X,
    OP_SET_SOUND_X,
    OP_ADD_X_I,
    OP_SET_I_SPRITE,
    OP_WRITE_BCD,
    OP_WRITE_REGS,
    OP_READ_REGS,
    OP_UNDEFINED
};

struct Instruction {
    Opcode op_id;

    // Store the values in the instruction context.
    uint16_t nnn;
    uint8_t nn;
    uint8_t n;

    uint8_t x;
    uint8_t y;
};


/**
 * @brief A simple data type to store instructions. It stores the instruction
 * identifier provided by an enum. It also stores all possible arguments that
 * could be used by the instruction.
 */
typedef struct Instruction Instruction;


// Graphics
extern bool pixel_buffer[32][64];

// Registers
extern uint16_t index_register;
extern uint8_t registers[16];
extern uint16_t program_counter;
extern uint8_t stack_pointer;

// Timers
extern uint8_t delay_timer;
extern uint8_t sound_timer;

// Stack
extern uint16_t stack[16];

// CPU methods
void push_stack(uint16_t val);
uint16_t pop_stack();


void cpu_loop();
