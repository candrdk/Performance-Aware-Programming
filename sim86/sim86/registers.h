#pragma once


enum register_index {
	AX = 0, AH = 0, AL = 1,
	CX = 1, CH = 2, CL = 3,
	DX = 2, DH = 4, DL = 5,
	BX = 3, BH = 6, BL = 7,
	SP = 4,
	BP = 5,
	SI = 6,
	DI = 7,

	ES = 8,
	CS = 9,
	SS = 10,
	DS = 11,

	IP = 12,
	FLAGS = 13,
	NONE
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

			u16 ES;
			u16 CS;
			u16 SS;
			u16 DS;

			u16 IP;
			u16 FLAGS;
		};

		u8 bytes[28];
		u16 words[14];
	};

	u8 get_byte(u8 reg) { return bytes[(reg << 1) | (reg >> 2)]; }
	u16 get_word(u8 reg) { return words[reg]; }

	static constexpr const char register_names[][3] = {
		"al", "cl",
		"dl", "bl",
		"ah", "ch",
		"dh", "bh",
		"ax", "cx",
		"dx", "bx",
		"sp", "bp",
		"si", "di",
		"es", "cs",
		"ss", "ds"
	};

	constexpr const char* get_name(u8 reg, bool word = true) {
		return register_names[word ? reg + 8 : reg];
	}
} registers;