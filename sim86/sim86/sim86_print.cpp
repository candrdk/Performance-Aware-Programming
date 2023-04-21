#include <stdio.h>

#include "sim86.h"
#include "instruction.h"

constexpr const char* mnemonics_table[] = {
	"",
#define INST(Mnemonic, ...) #Mnemonic,
#define INSTALT(...)
#include "sim86_instruction_table.inl"
};

void print_effective_address(FILE* stream, EffectiveAddress address) {
	fprintf(stream, "[");
	if (address.register1 != reg16_t::NONE) {
		fprintf(stream, registers.name(address.register1));
	}
	if (address.register2 != reg16_t::NONE) {
		fprintf(stream, "+%s", registers.name(address.register2));
	}
	if (address.displacement != 0) {
		if (address.register1 == reg16_t::NONE) {
			fprintf(stream, "%d", (u16)address.displacement);
		}
		else {
			fprintf(stream, "%+d", address.displacement);
		}
	}
	fprintf(stream, "]");
}

void print_instruction(FILE* stream, instruction instr) {
	bool W = instr.flags & Inst_Wide;

	if (instr.flags & Inst_Lock) {
		if (instr.op == operation_type::Op_xchg) {
			instruction_operand temp = instr.operands[0];
			instr.operands[0] = instr.operands[1];
			instr.operands[1] = temp;
		}
		fprintf(stream, "lock ");
	}

	const char* mnemonic_suffix = "";
	if (instr.flags & Inst_Rep) {
		fprintf(stream, "rep ");
		mnemonic_suffix = W ? "w" : "b";
	}

	fprintf(stream, "%s%s ", mnemonics_table[instr.op], mnemonic_suffix);

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
			fprintf(stream, registers.name(operand.Register.reg, operand.Register.wide));
		} break;

		case operand_type::MEMORY: {
			if (instr.flags & Inst_Far) {
				fprintf(stream, "far ");
			}
			if (operand.Address.explicit_segment) {
				fprintf(stream, "%u:%u", operand.Address.segment, operand.Address.displacement);
			}
			else {
				if (instr.operands[0].type != operand_type::REGISTER) {
					fprintf(stream, "%s ", W ? "word" : "byte");
				}

				if (instr.flags & Inst_Segment) {
					fprintf(stream, "%s:", registers.name(instr.segment_override));
				}
				
				print_effective_address(stream, operand.Address);
			}
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

	fprintf(stream, " ; ");
}