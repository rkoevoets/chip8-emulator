#pragma once
#include "cpu.h"

void opcode_clear_screen(Instruction instr);
void opcode_jump_address(Instruction instr);
void opcode_set_x(Instruction instr);
void opcode_add_x(Instruction instr);
void opcode_set_index(Instruction instr);
void opcode_draw(Instruction instr);
