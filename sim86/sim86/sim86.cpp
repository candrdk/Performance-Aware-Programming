#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define LISTING_37 "..\\computer_enhance\\perfaware\\part1\\listing_0037_single_register_mov"
#define LISTING_38 "..\\computer_enhance\\perfaware\\part1\\listing_0038_many_register_mov"
#define LISTING_39 "..\\computer_enhance\\perfaware\\part1\\listing_0039_more_movs"
#define LISTING_40 "..\\computer_enhance\\perfaware\\part1\\listing_0040_challenge_movs"
#define LISTING_41 "..\\computer_enhance\\perfaware\\part1\\listing_0041_add_sub_cmp_jnz"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

enum : u8 {
	AX, CX, DX, BX,
	SP, BP, SI, DI
};
enum : u8 {
	AL, CL, DL, BL,
	AH, CH, DH, BH
};

constexpr const char register_names[][3] = {
	"al", "cl",
	"dl", "bh",
	"ah", "ch",
	"dh", "bh",
	"ax", "cx",
	"dx", "bx",
	"sp", "bp",
	"si", "di"
};

constexpr const char* get_reg_name(u8 reg, bool word = true) {
	return register_names[reg | (word << 3)];
}

constexpr const u8 rm_table[8][2] = {
	{ 0b011, 0b110 },	// bx + si
	{ 0b011, 0b111 },	// bx + di
	{ 0b101, 0b110 },	// bp + si
	{ 0b101, 0b111 },	// bp + di
	{ 0b110 },			// si
	{ 0b111 },			// di
	{ 0b101 },			// bp / direct address
	{ 0b011 }			// bx
};

constexpr const char* rm_str_table[8] = {
  "bx + si",
  "bx + di",
  "bp + si",
  "bp + di",
  "si",
  "di",
  "bp",
  "bx"
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
} registers;

u8* data;
u8* data_end;
size_t data_count;

bool read_data(const char* path) {
	FILE* file; 
	fopen_s(&file, path, "rb");
	if (!file) return false;

	fseek(file, 0, SEEK_END);
	const int file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	data = (u8*)malloc(file_size);
	if (data == 0) return false;

	data_count = fread(data, 1, file_size, file);
	data_end = data + data_count;

	fclose(file);
	return true;
}


void print_op(const char* op, bool D, const char* dst, const char* src) {
	printf("%s %s, %s\n", op, D ? dst : src, D ? src : dst);
}

template <size_t N>
void format_effective_address(char(&address)[N], u8 MOD, u8 RM, u8* imm) {
	if (MOD == 0b00) {	// No displacement follows.
		if (RM == 0b110) {	// Special case for direct addresses
			sprintf_s(address, "[%u]", *(u16*)imm);
		}
		else {	// Only regs in effective address calculation
			sprintf_s(address, "[%s]", rm_str_table[RM]);
		}
	}
	else {		// Word or byte displacement follows
		i16 idisp = (MOD == 1) ? (*(i8*)imm) : (*(i16*)imm);	// Sign extension
		sprintf_s(address, "[%s %c %u]", rm_str_table[RM], idisp < 0 ? '-' : '+', idisp < 0 ? -idisp : idisp);
	}
}

int decode_mov_immediate_to_register(u8* ptr) {
	bool W = *ptr & 0b1000;
	u8 REG = *ptr & 0b111;
	ptr++;

	char immediate[8] = {};

	if (W) {
		sprintf_s(immediate, "%u", *(u16*)ptr);
		print_op("mov", true, get_reg_name(REG, W), immediate);
		return 3;
	}
	else {
		sprintf_s(immediate, "%u", *ptr);
		print_op("mov", true, get_reg_name(REG, W), immediate);
		return 2;
	}
}

int decode_mov_immediate_to_memory(u8* ptr) {
	int used_bytes = 2;

	bool W = *ptr & 1;
	ptr++;

	u8 MOD = *ptr >> 6;					// Mode
	u8 RM = *ptr & 0b111;				// Register/Memory
	ptr++;

	char address[32] = {};
	if (MOD == 0b11) {
		sprintf_s(address, get_reg_name(RM, W));
	}
	else {
		format_effective_address(address, MOD, RM, ptr);
		used_bytes += (MOD == 0 && RM == 0b110) ? 2 : MOD;
		ptr += (MOD == 0 && RM == 0b110) ? 2 : MOD;
	}

	char immediate[16] = {};
	if (W) {
		sprintf_s(immediate, "word %u", *(u16*)ptr);
		used_bytes += 2;
	}
	else {
		sprintf_s(immediate, "byte %i", *(i8*)ptr);
		used_bytes += 1;
	}

	print_op("mov", true, address, immediate);

	return used_bytes;
}

int decode_mov_accumulator_memory(u8* ptr) {
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

	print_op("mov", !D, get_reg_name(AX, W), address);
	return 2 + W;
}

int decode(u8* ptr) {
	int used_bytes = 2;

	// Figure out what op we are decoding
	char op[4];
	if ((*ptr >> 2) == 0b100010) { memcpy(op, "mov", 4); }
	else {
		switch ((*ptr >> 3) & 0b111) {
		case 0b000: // ADD
			memcpy(op, "add", 4);
			break;
		case 0b101: // SUB
			memcpy(op, "sub", 4);
			break;
		case 0b111: // CMP
			memcpy(op, "cmp", 4);
			break;
		}
	}

	bool D = *ptr & 0b10;
	bool W = *ptr & 0b01;
	ptr++;

	u8 MOD = *ptr >> 6;
	u8 REG = (*ptr >> 3) & 0b111;
	u8 RM = *ptr & 0b111;
	ptr++;

	if (MOD == 0b11) {	// Register to register - no displacement.
		print_op(op, D, get_reg_name(REG, W), get_reg_name(RM, W));
		return used_bytes;
	}

	char address[32] = {};
	format_effective_address(address, MOD, RM, ptr);

	print_op(op, D, get_reg_name(REG, W), address);
	used_bytes += (MOD == 0 && RM == 0b110) ? 2 : MOD;

	return used_bytes;
}

int decode_imm_to_reg_mem(u8* ptr) {
	int used_bytes = 2;

	bool S = *ptr & 0b10;
	bool W = *ptr & 0b01;
	ptr++;

	u8 MOD = *ptr >> 6;
	u8 OP = (*ptr >> 3) & 0b111;
	u8 RM = *ptr & 0b111;
	ptr++;

	char address[32] = {};
	if (MOD == 0b11) {	// Register mode
		sprintf_s(address, get_reg_name(RM, W));
	}
	else {				// Memory mode
		// Since format_ takes a c-array, we cant write the effective address at 'address + 5', 
		// so we have to do some ugly moves and copies to fix it up after.
		format_effective_address(address, MOD, RM, ptr);
		memmove(address + 5, address, strlen(address) + 1);
		memcpy(address, W ? "word " : "byte ", 5);
		used_bytes += (MOD == 0 && RM == 0b110) ? 2 : MOD;
		ptr += (MOD == 0 && RM == 0b110) ? 2 : MOD;
	}

	char immediate[16] = {};
	if (!S && W) {
		sprintf_s(immediate, "%u", *(u16*)ptr);
		used_bytes += 2;
	}
	else {
		sprintf_s(immediate, "%i", S ? *(i8*)ptr : *ptr);
		used_bytes += 1;
	}

	switch (OP) {
	case 0b000:
		print_op("add", true, address, immediate);
		break;
	case 0b101:
		print_op("sub", true, address, immediate);
		break;
	case 0b111:
		print_op("cmp", true, address, immediate);
		break;
	}

	return used_bytes;
}

int decode_imm_acc(u8* ptr) {
	int used_bytes = 2;

	bool W = *ptr & 0b01;
	u8 OP = (*ptr >> 3) & 0b111;
	ptr++;

	char immediate[16] = {};
	if (W) {		// cmp doesnt support word-sized data?
		sprintf_s(immediate, "%u", *(u16*)ptr);
		used_bytes += 1;
	}
	else {
		sprintf_s(immediate, "%i", *(i8*)ptr); // cmp is always unsigned?
	}

	switch (OP) {
	case 0b000:
		print_op("add", true, get_reg_name(AX, W), immediate);
		break;
	case 0b101:
		print_op("sub", true, get_reg_name(AX, W), immediate);
		break;
	case 0b111:
		print_op("cmp", true, get_reg_name(AX, W), immediate);
		break;
	}

	return used_bytes;
}

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	read_data(LISTING_41);
	if (!data_count) {
		return -1;
	}
#else
	if (argc != 2) { return -1; }
	if (!read_data(argv[1])) { return -1; }
#endif

	printf("bits 16\n\n");
	
	u8* ptr = data;
	while(ptr < data + data_count) {
		if      ( (*ptr >> 2)			   == 0b101000)		{ ptr += decode_mov_accumulator_memory(ptr); }
		else if ( (*ptr >> 1)			   == 0b1100011)	{ ptr += decode_mov_immediate_to_memory(ptr); } 
		else if ( (*ptr >> 4)			   == 0b1011)		{ ptr += decode_mov_immediate_to_register(ptr); } 
		else if ( (*ptr >> 2)              == 0b100010)		{ ptr += decode(ptr); }
		else if (((*ptr >> 2) & 0b110001)  == 0b000000)		{ ptr += decode(ptr); }
		else if ( (*ptr >> 2)			   == 0b100000)		{ ptr += decode_imm_to_reg_mem(ptr); }
		else if (((*ptr >> 1) & 0b1100011) == 0b0000010)	{ ptr += decode_imm_acc(ptr); }
		else {
			switch (*ptr++) {
			case 0b01110100: printf("je %i\n",		(*(i8*)ptr++)); break;	// jump on equal
			case 0b01111100: printf("jl %i\n",		(*(i8*)ptr++)); break;	// jump on less
			case 0b01111110: printf("jle %i\n",		(*(i8*)ptr++)); break;	// jump on less or equal
			case 0b01110010: printf("jb %i\n",		(*(i8*)ptr++)); break;	// jump on below
			case 0b01110110: printf("jbe %i\n",		(*(i8*)ptr++)); break;	// jump on below or equal
			case 0b01111010: printf("jp %i\n",		(*(i8*)ptr++)); break;	// jump on parity
			case 0b01110000: printf("jo %i\n",		(*(i8*)ptr++)); break;	// jump on overflow
			case 0b01111000: printf("js %i\n",		(*(i8*)ptr++)); break;	// jump on sign
			case 0b01110101: printf("jne %i\n",		(*(i8*)ptr++)); break;	// jump on not equal
			case 0b01111101: printf("jnl %i\n",		(*(i8*)ptr++)); break;	// jump on not less
			case 0b01111111: printf("jnle %i\n",	(*(i8*)ptr++)); break;	// jump on not less or equal
			case 0b01110011: printf("jnb %i\n",		(*(i8*)ptr++)); break;	// jump on not below
			case 0b01110111: printf("jnbe %i\n",	(*(i8*)ptr++)); break;	// jump on not below or equal
			case 0b01111011: printf("jnp %i\n",		(*(i8*)ptr++)); break;	// jump on not parity
			case 0b01110001: printf("jno %i\n",		(*(i8*)ptr++)); break;	// jump on not overflow
			case 0b01111001: printf("jns %i\n",		(*(i8*)ptr++)); break;	// jump on not sign
			case 0b11100010: printf("loop %i\n",	(*(i8*)ptr++)); break;	// loop CX times
			case 0b11100001: printf("loopz %i\n",	(*(i8*)ptr++)); break;	// loop while zero
			case 0b11100000: printf("loopnz %i\n",	(*(i8*)ptr++)); break;	// loop while not zero
			case 0b11100011: printf("jcxz %i\n",	(*(i8*)ptr++)); break;	// jump on CX zero
			default:
				printf("\nUnknown opcode.\n\n");
				return -1;
			}
		}
	}

	return 0;
}