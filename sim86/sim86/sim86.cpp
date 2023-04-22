#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sim86.h"
#include "memory.h"
#include "instruction.h"
#include "sim86_print.h"
#include "sim86_decode.h"

void disassemble(Memory memory, size_t byte_count) {
	while (registers.IP < byte_count) {
		instruction instr = decode_instruction(memory);
		if (instr.op) {
			if (registers.IP > byte_count) {
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

size_t load_memory_from_file(const char* path, Memory memory) {
	FILE* file;
	fopen_s(&file, path, "rb");

	if (file) {
		size_t read = fread(&memory(registers.CS, 0), 1, memory.m_size, file);
		fclose(file);
		return read;
	}
	else {
		fprintf(stderr, "ERROR: Unable to open %s.\n", path);
		return -1;
	}
}

int main(int argc, char* argv[]) {
	Memory memory;
	if (!memory.allocate(1024 * 1024)) {
		fprintf(stderr, "ERROR: Failed to allocate main memory for 8086.\n");
		return -1;
	}

	/* Set up segment registers.
	*   0x0      0x10000   0x20000  0x30000 ... 0x100000
	*	+--------+---------+--------+------
	*	|  code  |  stack  |  data  |
	*	+--------+---------+--------+------
	*   ^- CS    ^- SS     ^- DS/ES
	* Code is loaded into memory starting from address 0.
	* After the code comes the 64k stack segment
	* Finally the 64k data segment starts at 0x20000.
	* The extra segment (ES) is initialized to point DS;
	*/
	registers.CS = 0x00000;
	registers.SS = 0x10000;
	registers.DS = 0x20000;
	registers.ES = registers.DS;

#ifdef _DEBUG
	size_t bytes_read = load_memory_from_file(LISTING_50, memory);
#else
	if (argc != 2) {
		fprintf(stderr, "USAGE: %s [8086 machine code file]\n", argv[0]);
		return -1;
	}

	u32 bytes_read = load_memory_from_file(argv[1], memory);
	if (bytes_read == -1) { return -1; }
#endif
	
	printf("; %s disassembly:\n", argv[1]);
	printf("bits 16\n");
	disassemble(memory, bytes_read);
	printf("\n");
	registers.dump_registers(stdout);
}