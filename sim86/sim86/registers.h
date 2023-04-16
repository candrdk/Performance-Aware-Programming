#pragma once

enum register_index : u8 {
	AX, CX, DX, BX,
	SP, BP, SI, DI,
	NONE, COUNT
};

constexpr const register_index rm_table[8][2] = {
	{ register_index::BX, register_index::SI },		// bx + si
	{ register_index::BX, register_index::DI },		// bx + di
	{ register_index::BP, register_index::SI },		// bp + si
	{ register_index::BP, register_index::DI },		// bp + di
	{ register_index::SI, register_index::NONE },	// si
	{ register_index::DI, register_index::NONE },	// di
	{ register_index::BP, register_index::NONE },	// bp / direct address
	{ register_index::BX, register_index::NONE }	// bx
};

struct {
	union {
		struct {
			union {
				u16 AX;
				struct { u8 AH; u8 AL; };
			};
			union {
				u16 CX;
				struct { u8 CH; u8 CL; };
			};
			union {
				u16 DX;
				struct { u8 DH; u8 DL; };
			};
			union {
				u16 BX;
				struct { u8 BH; u8 BL; };
			};

			u16 SP;
			u16 BP;
			u16 SI;
			u16 DI;
		};

		u8 bytes[16];
		u8 words[8];
	};

	u8 get_byte(u8 reg) { return bytes[(reg << 1) | (reg >> 2)]; }
	u16 get_word(u8 reg) { return words[reg]; }

	static constexpr const char register_names[][3] = {
		"al", "cl",
		"dl", "bh",
		"ah", "ch",
		"dh", "bh",
		"ax", "cx",
		"dx", "bx",
		"sp", "bp",
		"si", "di"
	};

	constexpr const char* get_name(u8 reg, bool word = true) {
		return register_names[reg | (word << 3)];
	}
} registers;