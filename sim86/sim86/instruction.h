#pragma once

#include <stdio.h>
#include "registers.h"

enum operation_type {
	Op_None,

	// Ignore INSTALT to avoid duplication.
#define INST(Mnemonic, ...) Op_##Mnemonic,
#define INSTALT(...)
#include "sim86_instruction_table.inl"

	Op_Count
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
	i16 displacement;
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
};
