#pragma once

#include <stdio.h>
#include "registers.h"

enum operation_type : u32 {
	Op_None,

	// Ignore INSTALT to avoid duplication.
#define INST(Mnemonic, ...) Op_##Mnemonic,
#define INSTALT(...)
#include "sim86_instruction_table.inl"

	Op_Count
};

enum instruction_flag : u16 {
	Inst_Lock = 0x1,
	Inst_Rep = 0x2,
	Inst_Segment = 0x4,
	Inst_Wide = 0x8,
	Inst_Far = 0x10,
};

enum class operand_type : u16 {
	NONE,
	REGISTER,
	MEMORY,
	IMMEDIATE
};

struct Register {
	register_index index;
	bool wide;
};

struct EffectiveAddress {
	register_index register1;
	register_index register2;
	i32 displacement;
	u32 segment;
	bool explicit_segment;
};

struct Immediate {
	i32 value;
	bool relative_jump;
};

struct instruction_operand {
	operand_type type;
	union {
		Register Register;
		EffectiveAddress Address;
		Immediate Immediate;
	};
};

struct instruction {
	u32 address;
	u32 size;

	operation_type op;
	u32 flags;

	instruction_operand operands[2];

	u32 segment_override;
};
