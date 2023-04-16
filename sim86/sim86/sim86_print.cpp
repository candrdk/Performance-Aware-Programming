#include <stdio.h>

#include "sim86.h"
#include "registers.h"
#include "instruction.h"

constexpr const char* mnemonics_table[] = {
	"",
#define INST(Mnemonic, ...) #Mnemonic,
#define INSTALT(...)
#include "sim86_instruction_table.inl"
};

void print_effective_address(FILE* stream, EffectiveAddress address) {
	fprintf(stream, "[");
	if (address.register1 != register_index::NONE) {
		fprintf(stream, registers.get_name(address.register1, true));
	}
	if (address.register2 != register_index::NONE) {
		fprintf(stream, "+%s", registers.get_name(address.register2, true));
	}
	if (address.displacement != 0) {
		if (address.register1 == register_index::NONE) {
			fprintf(stream, "%d", address.displacement);
		}
		else {
			fprintf(stream, "%+d", address.displacement);
		}
	}
	fprintf(stream, "]");
}

void print_instruction(FILE* stream, instruction instr) {
	bool W = instr.flags & 1; //TODO: check instr_wide flag

	fprintf(stream, "%s ", mnemonics_table[instr.op]);

	char const* sep = "";
	for (int i = 0; i < 2; i++) {
		instruction_operand operand = instr.operands[i];

		if (operand.type == operand_type::NONE) {
			continue;
		}

		fprintf(stream, "%s", sep);
		sep = ", ";

		switch (operand.type) {
		case operand_type::REGISTER: {
			fprintf(stream, registers.get_name(operand.Register.index, operand.Register.wide));
		} break;

		case operand_type::MEMORY: {
			//TODO: handle instr.flags & Instr_Far
			//TODO: handle address.flags & Address_ExplicitSegment

			if (instr.operands[0].type != operand_type::REGISTER) {
				fprintf(stream, "%s ", W ? "word" : "byte");
			}

			//TODO: handle instr.flags & Inst_Segment
			print_effective_address(stream, operand.Address);
		} break;

		case operand_type::IMMEDIATE: {
			if (operand.Immediate.relative_jump) {
				fprintf(stream, "$%+d", operand.Immediate.value + instr.size);
			}
			else {
				fprintf(stream, "%d", operand.Immediate.value);
			}
		} break;
		}
	}

	fprintf(stream, "\n");
}