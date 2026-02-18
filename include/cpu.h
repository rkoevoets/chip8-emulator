#pragma once

// Registers
extern uint16_t index_register;
extern uint8_t registers[16];
extern uint16_t program_counter;
extern uint8_t stack_pointer;

// CPU methods
void cpu_loop();
