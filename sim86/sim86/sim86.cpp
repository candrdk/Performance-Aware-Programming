#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;

static const char* register_names[] = {
	"al", "cl",
	"dl", "bh",
	"ah", "ch",
	"dh", "bh",
	"ax", "cx",
	"dx", "bx",
	"sp", "bp",
	"si", "di"
};

static char const* get_reg_name(u8 reg, bool word) {
	return register_names[reg | (word << 3)];
}

namespace REGISTER {
	enum : u8 {
		AX, CX, DX, BX,
		SP, BP, SI, DI
	};
	enum : u8 {
		AL, CL, DL, BL,
		AH, CH, DH, BH
	};
}

static struct {
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
} registers;

u8* data;
u8* data_end;
size_t data_count;

constexpr u8 rm_table[8][2] = {
	{ 0b011, 0b110 },	// bx + si
	{ 0b011, 0b111 },	// bx + di
	{ 0b101, 0b110 },	// bp + si
	{ 0b101, 0b111 },	// bp + di
	{ 0b110 },			// si
	{ 0b111 },			// di
	{ 0b101 },			// direct address
	{ 0b011 }			// bx
};

size_t read_data(const char* path) {
	FILE* file; 
	fopen_s(&file, path, "rb");
	if (!file) return 0;

	fseek(file, 0, SEEK_END);
	const int file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	data = (u8*)malloc(file_size);
	if (data == 0) return 0;

	data_count = fread(data, 1, file_size, file);
	data_end = data + data_count;

	fclose(file);
}

#define LISTING_37 "..\\computer_enhance\\perfaware\\part1\\listing_0037_single_register_mov"
#define LISTING_38 "..\\computer_enhance\\perfaware\\part1\\listing_0038_many_register_mov"
#define LISTING_39 "..\\computer_enhance\\perfaware\\part1\\listing_0039_more_movs"
#define LISTING_40 "..\\computer_enhance\\perfaware\\part1\\listing_0040_challenge_movs"

void print_mov(bool D, const char* dst, const char* src) {
	printf("mov %s, %s\n", D ? dst : src, D ? src : dst);
}

template <size_t N>
void format_effective_address(char(&address)[N], u8 MOD, u8 RM, u8* imm) {
	switch (MOD) {
	case 0b00: {	// No displacement follows.
		if (RM == 0b110) {						// Special case for direct addresses.
			assert(imm + 1 < data_end);
			sprintf_s(address, "[%i]", *(u16*)imm);
		}
		else if (RM & 0b100) {					// Only one reg in effective address calculation.
			sprintf_s(address, "[%s]", get_reg_name(rm_table[RM][0], true));
		}
		else {									// Two regs in effective address calculation.
			sprintf_s(address, "[%s + %s]", get_reg_name(rm_table[RM][0], true), get_reg_name(rm_table[RM][1], true));
		}
		break;
	}

	case 0b01: {	// Byte displacement follows.
		assert(imm < data_end);
		i8 disp = *(i8*)imm;
		if (RM & 0b100) {						// Only one reg in effective address calculation.
			if (disp < 0) {
				sprintf_s(address, "[%s - %i]", get_reg_name(rm_table[RM][0], true), disp * -1);
			}
			else {
				sprintf_s(address, "[%s + %i]", get_reg_name(rm_table[RM][0], true), disp);
			}
		}
		else {									// Two regs in effective address calculation.
			if (disp < 0) {
				sprintf_s(address, "[%s + %s - %i]", get_reg_name(rm_table[RM][0], true), get_reg_name(rm_table[RM][1], true), disp * -1);
			}
			else {
				sprintf_s(address, "[%s + %s + %i]", get_reg_name(rm_table[RM][0], true), get_reg_name(rm_table[RM][1], true), disp);
			}
		}
		break;
	}

	case 0b10: {	// Word displacement follows.
		assert(imm + 1 < data_end);
		i16 disp = *(i16*)imm;
		if (RM & 0b100) {						// Only one reg in effective address calculation.
			if (disp < 0) {
				sprintf_s(address, "[%s - %i]", get_reg_name(rm_table[RM][0], true), disp * -1);
			}
			else {
				sprintf_s(address, "[%s + %i]", get_reg_name(rm_table[RM][0], true), disp);
			}
		}
		else {									// Two regs in effective address calculation.
			if (disp < 0) {
				sprintf_s(address, "[%s + %s - %i]", get_reg_name(rm_table[RM][0], true), get_reg_name(rm_table[RM][1], true), disp * -1);
			}
			else {
				sprintf_s(address, "[%s + %s + %i]", get_reg_name(rm_table[RM][0], true), get_reg_name(rm_table[RM][1], true), disp);
			}
		}
		break;
	}
	}
}

int decode_mov(u8* ptr) {
	assert(ptr + 1 < data_end);

	bool D = *ptr & 0b10;	// 1: REG is destination. 0: REG is source.
	bool W = *ptr & 0b01;	// 1: word operation. 0: byte operation.
	ptr++;

	u8 MOD = *ptr >> 6;					// Mode
	u8 REG = (*ptr & 0b111000) >> 3;	// Register
	u8 RM = *ptr & 0b111;				// Register/Memory
	ptr++;
	int used_bytes = 2;

	if (MOD == 0b11) {	// Register to register - no displacement.
		print_mov(D, get_reg_name(REG, W), get_reg_name(RM, W));
		return used_bytes;
	}

	char address[32] = {};
	used_bytes += (MOD == 0 && RM == 0b110) ? 2 : MOD;

	format_effective_address(address, MOD, RM, ptr);
	print_mov(D, get_reg_name(REG, W), address);

	return used_bytes;
}

int decode_immediate_to_register(u8* ptr) {
	bool W = *ptr & 0b1000;
	u8 REG = *ptr & 0b111;
	ptr++;

	char immediate[8] = {};

	if (W) {
		sprintf_s(immediate, "%u", *(u16*)ptr);
		print_mov(true, get_reg_name(REG, W), immediate);
		return 3;
	}
	else {
		sprintf_s(immediate, "%u", *ptr);
		print_mov(true, get_reg_name(REG, W), immediate);
		return 2;
	}
}

int decode_immediate_to_memory(u8* ptr) {
	bool W = *ptr & 1;
	ptr++;

	u8 MOD = *ptr >> 6;					// Mode
	u8 RM = *ptr & 0b111;				// Register/Memory
	ptr++;

	int used_bytes = 2;

	char address[32] = {};
	if (MOD == 0b11) {	// Register to register - no displacement.
		sprintf_s(address, "%s", get_reg_name(RM, W));
	}
	else {
		format_effective_address(address, MOD, RM, ptr);
		used_bytes += (MOD == 0 && RM == 0b110) ? 2 : MOD;
		ptr += MOD;
	}

	char immediate[16] = {};
	if (W) {
		sprintf_s(immediate, "word %u", *(u16*)ptr);
		print_mov(true, address, immediate);
		return used_bytes + 2;
	}
	else {
		sprintf_s(immediate, "byte %u", *ptr);
		print_mov(true, address, immediate);
		return used_bytes + 1;
	}
}

int decode_accumulator_memory(u8* ptr) {
	bool W = *ptr & 0b01;
	bool D = *ptr & 0b10;
	ptr++;

	char address[16] = {};
	if (W) {
		sprintf_s(address, "[%u]", *(u16*)ptr);
	}
	else {
		sprintf_s(address, "[%u]", *ptr);
	}

	print_mov(!D, get_reg_name(REGISTER::AX, W), address);
	return 2 + W;
}

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	read_data(LISTING_40);
	if (!data_count) {
		return -1;
	}
#else
	if (argc != 2) { return -1; }

	switch (atoi(argv[1])) {
	case 37:
		read_data(LISTING_37);
		break;
	case 38:
		read_data(LISTING_38);
		break;
	default:
		printf("Unknown listing\n");
		return -1;
	}
#endif

	printf("bits 16\n\n");
	
	u8* ptr = data;
	while(ptr < data + data_count){
		if ((*ptr >> 2) == 0b101000)		{ ptr += decode_accumulator_memory(ptr); }
		else if ((*ptr >> 1) == 0b1100011)	{ ptr += decode_immediate_to_memory(ptr); } 
		else if ((*ptr >> 4) == 0b1011)		{ ptr += decode_immediate_to_register(ptr); } 
		else if ((*ptr >> 2) == 0b100010)	{ ptr += decode_mov(ptr); } 
		else {
			printf("\nUnknown opcode.\n\n");
			return -1;
		}
	}

	return 0;
}