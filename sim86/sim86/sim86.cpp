#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sim86.h"
#include "instruction.h"
#include "memory.h"
#include "sim86_print.h"
#include "sim86_decode.h"

void disassemble(size_t byte_count) {
	while (registers.IP < byte_count) {
		instruction instr = decode_instruction();
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

int main(int argc, char* argv[]) {
	if (!memory.allocate(1024 * 1024)) {
		fprintf(stderr, "ERROR: Failed to allocate main memory for 8086.\n");
		return -1;
	}

	/* Set up segment registers.
	* Physical address:		0x0      0x10000   0x20000  0x30000 ... 0x100000
	*						+--------+---------+--------+------
	*						|  code  |  stack  |  data  |
	*						+--------+---------+--------+------
	* Segment bases:	    
	*	CS = 0x0000 SS = 0x1000 DS = 0x2000
	* 
	* Physical address calculated by shifting up the segment base and adding an
	* offset. Thus, SS starts at the physical address: 0x1000 << 4 = 0x10000
	* 
	* Code is loaded into memory starting from address 0.
	* After the code comes the 64k stack segment
	* Finally the 64k data segment starts at 0x20000.
	* The extra segment (ES) is initialized to point DS;
	*/
	registers.CS = 0x0000;
	registers.SS = 0x1000;
	registers.DS = 0x2000;
	registers.ES = registers.DS;

#ifdef _DEBUG
	size_t bytes_read = memory.load_from_file(LISTING_55, registers.CS, registers.IP);
#else
	if (argc != 2) {
		fprintf(stderr, "USAGE: %s [8086 machine code file]\n", argv[0]);
		return -1;
	}

	size_t bytes_read = memory.load_from_file(argv[1], registers.CS, registers.IP);
	if (bytes_read == -1) { return -1; }
#endif
	
	printf("; %s disassembly:\n", argv[1]);
	printf("bits 16\n");
	disassemble(bytes_read);
	printf("\n");
	registers.dump_registers(stdout);

	size_t bytes_written = memory.dump_to_file("memory.DATA");
	if (bytes_written != memory.m_size) {
		fprintf(stderr, "ERROR: Failed to dump memory. Wrote %d of %d bytes", bytes_written, memory.m_size);
		return -1;
	}

	return 0;
}