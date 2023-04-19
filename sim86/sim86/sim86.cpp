#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sim86.h"
#include "instruction.h"
#include "sim86_print.h"
#include "sim86_decode.h"

void disassemble(u32 byte_count, u8* ptr) {
	while (byte_count) {
		instruction instr = decode_instruction(ptr);
		if (instr.op) {
			if (byte_count >= instr.size) {
				ptr += instr.size;
				byte_count -= instr.size;
			}
			else {
				fprintf(stderr, "ERROR: Instruction extends outside disassembly region.\n");
				break;
			}

			print_instruction(stdout, instr);
			execute_instruction(instr);
			fprintf(stdout, "\n");
		}
		else {
			fprintf(stderr, "ERROR: Unrecognized binary in instruction stream.\n");
			break;
		}
	}
}

u32 load_memory_from_file(const char* path, u8* memory, u32 size) {
	FILE* file;
	fopen_s(&file, path, "rb");

	if (file) {
		u32 read = fread(memory, 1, size, file);
		fclose(file);
		return read;
	}
	else {
		fprintf(stderr, "ERROR: Unable to open %s.\n", path);
		return -1;
	}
}

int main(int argc, char* argv[]) {
	u8* memory = (u8*)malloc(1024 * 1024);
	if (!memory) {
		fprintf(stderr, "ERROR: Failed to allocate main memory for 8086.\n");
		return -1;
	}

#ifdef _DEBUG
	u32 bytes_read = load_memory_from_file(LISTING_45, memory, 1024 * 1024);
#else
	if (argc != 2) {
		fprintf(stderr, "USAGE: %s [8086 machine code file]\n", argv[0]);
		return -1;
	}

	u32 bytes_read = load_memory_from_file(argv[1], memory, 1024 * 1024);
	if (bytes_read == -1) { return -1; }
#endif
	
	printf("; %s disassembly:\n", argv[1]);
	printf("bits 16\n");
	disassemble(bytes_read, memory);
}