#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

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

static struct {
	union {
		struct {
			union {
				u16 AX;
				struct { u8 AL; u8 AH; };
			};
			union {
				u16 CX;
				struct { u8 CL; u8 CH; };
			};
			union {
				u16 DX;
				struct { u8 DL; u8 DH; };
			};
			union {
				u16 BX;
				struct { u8 BL; u8 BH; };
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
size_t data_count;

size_t read_data(const char* path) {
	FILE* file; 
	fopen_s(&file, path, "rb");
	if (!file) return 0;

	fseek(file, 0, SEEK_END);
	const int file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	data = (u8*)malloc(file_size);
	if (data == 0) return 0;

	size_t read = fread(data, 1, file_size, file);
	fclose(file);

	return read;
}

#define LISTING_37 "..\\computer_enhance\\perfaware\\part1\\listing_0037_single_register_mov"
#define LISTING_38 "..\\computer_enhance\\perfaware\\part1\\listing_0038_many_register_mov"

void decode_mov(u8* ptr) {
	printf("mov ");

	bool D = *ptr & 0b10;	// 1: REG is destination. 0: REG is source.
	bool W = *ptr & 0b01;	// 1: word operation. 0: byte operation.

	ptr++;
	u8 MOD = *ptr >> 6;					// register mode or memory mode?
	u8 REG = (*ptr & 0b111000) >> 3;	// register
	u8 RM = *ptr & 0b111;				// register

	ptr++;
	if (D) {
		printf("%s ", get_reg_name(REG, W));
		printf("%s\n", get_reg_name(RM, W));
	}
	else {
		printf("%s ", get_reg_name(RM, W));
		printf("%s\n", get_reg_name(REG, W));
	}
}

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	data_count = read_data(LISTING_38);
	if (!data_count) {
		return -1;
	}
#else
	if (argc != 2) { return; }

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
		switch(*ptr >> 2)
		{
		case 0b100010:
			assert(ptr + 1 < data + data_count);
			decode_mov(ptr);
			ptr += 2;
			break;

		default:
			printf("\nUnknown opcode.\n\n");
			return -1;
		}
	}

	return 0;
}