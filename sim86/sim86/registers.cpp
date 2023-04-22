#include <stdio.h>

#include "registers.h"

u8& Registers::operator[](reg8_t reg) { 
	assert(reg < 8); 
	return reg < 4 ? regs[reg].reg8_l : regs[reg - 4].reg8_h; 
};
const u8& Registers::operator[](reg8_t reg) const { 
	assert(reg < 8); 
	return reg < 4 ? regs[reg].reg8_l : regs[reg - 4].reg8_h; 
};

u16& Registers::operator[](reg16_t reg) { 
	assert(reg < REG_COUNT); 
	return regs[reg].reg16; 
};
const u16& Registers::operator[](reg16_t reg) const { 
	assert(reg < REG_COUNT); 
	return regs[reg].reg16; 
};

bool Registers::operator[](flag_t flag) { 
	assert((flag & ALL_FLAGS) && !(flag & (~ALL_FLAGS)));
	return flags & flag; 
}

void Registers::set_flag(flag_t flag, bool b) {
	if (b) {
		flags |= flag;
	}
	else {
		flags &= ~(flag);
	}
}

void Registers::flags_dbg_str(char* p) const {
	const char flag_chars[] = { 'C', 0, 'P', 0, 'A', 0, 'Z', 'S', 'T', 'I', 'D', 'O', 0, 0, 0, 0 };
	for (int i = 0; i < 16; i++) {
		if (flags & (1 << i)) {
			*p++ = flag_chars[i];
		}
	}
	*p++ = '\0';
}

void Registers::dump_registers(FILE* stream) const {
	fprintf(stream, "Final registers:\n");
	const reg16_t print_order[13] = { reg16_t::AX, reg16_t::BX, reg16_t::CX, reg16_t::DX, 
									  reg16_t::SP, reg16_t::BP, reg16_t::SI, reg16_t::DI, 
									  reg16_t::CS, reg16_t::DS, reg16_t::SS, reg16_t::ES,
									  reg16_t::IP };
	
	for (reg16_t* r = (reg16_t*)&print_order; r < (reg16_t*)&print_order + 13; r++) {
		if(registers[*r])
			fprintf(stream, "%8s: 0x%04x (%d)\n", registers.name(*r), registers[*r], registers[*r]);
	}

	char flag_str[16];
	registers.flags_dbg_str(flag_str);
	fprintf(stream, "   flags: %s\n", flag_str);
}

Registers registers;