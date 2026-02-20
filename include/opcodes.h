#pragma once

#include "cpu.h"

// starts with 0x0...
void opcode_execute_routine(Instruction instr);
void opcode_clear_screen(Instruction instr);
void opcode_jump_subr(Instruction instr);

void opcode_jump_address(Instruction instr);

void opcode_return(Instruction instr);
void opcode_call_subr(Instruction instr);

void opcode_skip_val_eq(Instruction instr);
void opcode_skip_val_neq(Instruction instr);

void opcode_skip_reg_eq(Instruction instr);
void opcode_skip_reg_neq(Instruction instr);

void opcode_set_x(Instruction instr);
void opcode_add_x(Instruction instr);

// 0x8...
void opcode_add_x_to_y(Instruction instr);
void opcode_set_x_y(Instruction instr);
void opcode_or(Instruction instr);
void opcode_and(Instruction instr);
void opcode_xor(Instruction instr);
void opcode_add_y_to_x(Instruction instr);
void opcode_sub_y_from_x(Instruction instr);
void opcode_sub_x_from_y(Instruction instr);
void opcode_shift_right(Instruction instr);
void opcode_shift_left(Instruction instr);

void opcode_set_index(Instruction instr);

void opcode_jump_offset(Instruction instr);

void opcode_set_x_random(Instruction instr);

void opcode_draw(Instruction instr);

void opcode_skip_kp(Instruction instr);
void opcode_skip_not_kp(Instruction instr);

// 0xF...
void opcode_set_x_to_delay(Instruction instr);
void opcode_wait_keypress(Instruction instr);
void opcode_set_delay_to_x(Instruction instr);
void opcode_set_sound_to_x(Instruction instr);
void opcode_add_x_to_index(Instruction instr);
void opcode_set_index_sprite(Instruction instr);
void opcode_write_bcd(Instruction instr);
void opcode_write_regs(Instruction instr);
void opcode_read_regs(Instruction instr);
