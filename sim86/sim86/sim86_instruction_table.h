#pragma once
/*
	Instruction tables stolen from the reference implementation
*/

#include "instruction.h"

enum instruction_bits_usage {
	Bits_End, // 0 indicates end of the instruction array

	Bits_Literal,

	Bits_D,
	Bits_S,
	Bits_W,
	Bits_V,
	Bits_Z,

	Bits_MOD,
	Bits_REG,
	Bits_RM,
	Bits_SR,

	Bits_Disp,
	Bits_Data,

	// Instruction tags
	Bits_DispAlwaysW,	// displacement is always 16 bits
	Bits_WMakesDataW,	// SW=01 makes the data field become 16 bits
	Bits_RMRegAlwaysW,	// register encoded in RM is always 16-bit width
	Bits_RelJMPDisp,	// instructions that require address adjustment to go through NASM properly
	Bits_Far,			// instructions that require a "far" keyword in their ASM to select the right opcode

	Bits_Count
};

struct instruction_bits {
	instruction_bits_usage usage;
	unsigned char count;
	unsigned char shift;
	unsigned char value;
};

struct instruction_encoding {
	operation_type op;
	instruction_bits bits[16];
};

static constexpr const instruction_encoding instruction_table[] = {
#include "sim86_instruction_table.inl"
};