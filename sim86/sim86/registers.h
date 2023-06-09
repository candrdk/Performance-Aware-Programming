#pragma once

#include "sim86.h"

enum flag_t: u16 {
	CF = 1 << 0,
	PF = 1 << 2,
	AF = 1 << 4,
	ZF = 1 << 6,
	SF = 1 << 7,
	TF = 1 << 8,
	IF = 1 << 9,
	DF = 1 << 10,
	OF = 1 << 11,
	ALL_FLAGS = 0b0000111111010101
};

enum reg16_t : u32 { AX, CX, DX, BX, SP, BP, SI, DI, ES, CS, SS, DS, IP, FLAGS, REG_COUNT, NONE };
enum reg8_t : u32 { AL, CL, DL, BL, AH, CH, DH, BH };

constexpr const reg16_t rm_table[8][2] = {
	{ BX, SI },		// bx + si
	{ BX, DI },		// bx + di
	{ BP, SI },		// bp + si
	{ BP, DI },		// bp + di
	{ SI, NONE },	// si
	{ DI, NONE },	// di
	{ BP, NONE },	// bp / direct address
	{ BX, NONE }	// bx
};

union gp_reg_t {
	u16 reg16;
	struct {
		u8 reg8_l;
		u8 reg8_h;
	};
};

struct Registers {
private:
	static constexpr const char* reg16_names[] = {
		"ax", "cx", "dx", "bx", 
		"sp", "bp", "si", "di", 
		"es", "cs", "ss", "ds",
		"ip", "flags"
	};
	static constexpr const char* reg8_names[] = {
		"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"
	};
public:
	static constexpr const char* name(reg8_t reg) { assert(reg < 8); return reg8_names[reg]; }
	static constexpr const char* name(reg16_t reg) { assert(reg < REG_COUNT); return reg16_names[reg]; }
	static constexpr const char* name(u32 reg, bool wide) {
		assert(wide ? reg < REG_COUNT : reg < 8);
		return wide ? reg16_names[reg] : reg8_names[reg & 0b111];
	}

	union {
		struct {
			union {
				u16 AX;
				struct { u8 AL, AH; };
			};
			union {
				u16 CX;
				struct { u8 CL, CH; };
			};
			union {
				u16 DX;
				struct { u8 DL, DH; };
			};
			union {
				u16 BX;
				struct { u8 BL, BH; };
			};

			u16 SP, BP, SI, DI, ES, CS, SS, DS, IP;
			u16 flags;
		};
		gp_reg_t regs[REG_COUNT];
	};
	u8& operator[](reg8_t reg);
	const u8& operator[](reg8_t reg) const;

	u16& operator[](reg16_t reg);
	const u16& operator[](reg16_t reg) const;

	bool operator[](flag_t flag);

	void set_flag(flag_t flag, bool b);

	void flags_dbg_str(char* p) const;
	void dump_registers(FILE* stream) const;
};

extern Registers registers;