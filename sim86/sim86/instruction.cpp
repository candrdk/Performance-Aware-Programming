#include "instruction.h"
#include <intrin.h>

void (*exec_op_table[Op_Count])(instruction) = {
	nullptr,
#define INST(Mnemonic, ...) exec_##Mnemonic,
#define INSTALT(...)
#include "sim86_instruction_table.inl"
};

void execute_instruction(instruction instr) {
	exec_op_table[instr.op](instr);
}

u16 load_op_value(instruction_operand op) {
	switch (op.type) {
		case operand_type::REGISTER:
			return op.Register.wide ? registers[reg16_t(op.Register.reg)] : registers[reg8_t(op.Register.reg)];
		case operand_type::IMMEDIATE:
			return op.Immediate.value;
		case operand_type::MEMORY:
			assert(false); //TODO: not implemented
			break;
	}
}

void store_op_value(instruction_operand op, u16 res) {
	switch (op.type) {
		case operand_type::REGISTER: {
			// printing
			reg16_t reg16 = op.Register.reg16;
			if (reg16 < 8 && !(op.Register.wide)) reg16 = reg16_t(reg16 & 0b11);
			printf("%s:0x%x->", registers.name(reg16), registers[reg16]);

			if (op.Register.wide) {
				registers[op.Register.reg16] = res;
			}
			else {
				registers[op.Register.reg8] = (u8)res;
			}

			printf("0x%x ", registers[reg16]);
			break;
		}

		case operand_type::MEMORY: {
			assert(false); //TODO: not implemented
		}
	}
}

void exec_mov(instruction instr) {
	u16 value = load_op_value(instr.operands[1]);
	store_op_value(instr.operands[0], value);

	// print IP
	printf("ip:0x%x->0x%x ", registers.IP - instr.size, registers.IP);
}

void exec_push(instruction instr) {}
void exec_pop(instruction instr) {}
void exec_xchg(instruction instr) {}
void exec_in(instruction instr) {}
void exec_out(instruction instr) {}
void exec_xlat(instruction instr) {}
void exec_lea(instruction instr) {}
void exec_lds(instruction instr) {}
void exec_les(instruction instr) {}
void exec_lahf(instruction instr) {}
void exec_sahf(instruction instr) {}
void exec_pushf(instruction instr) {}
void exec_popf(instruction instr) {}

void exec_add(instruction instr) {
	char flag_str[16] = {};
	registers.flags_dbg_str(flag_str);

	// load operand values
	u16 dst = load_op_value(instr.operands[0]);
	u16 src = load_op_value(instr.operands[1]);
	u32 res = dst + src;

	// update flags
	if (instr.flags & Inst_Wide) {
		registers.set_flag(CF, res & (1 << 16));
		registers.set_flag(PF, !(__popcnt16(res & 0xFF) & 1));
		registers.set_flag(ZF, (res & 0xFFFF) == 0);
		registers.set_flag(SF, res & (1 << 15));

		registers.set_flag(AF, 15 < ((dst & 0xF) + (src & 0xF)));
		registers.set_flag(OF, ((dst >> 15) == (src >> 15)) && ((res >> 15) != (dst >> 15)));
	} 
	else {
		registers.set_flag(CF, res & (1 << 8));
		registers.set_flag(PF, !(__popcnt16(res & 0xFF) & 1));
		registers.set_flag(ZF, (res & 0xFF) == 0);
		registers.set_flag(SF, res & (1<<7));

		registers.set_flag(AF, 15 < ((dst & 0xF) + (src & 0xF)));
		registers.set_flag(OF, ((dst & 0x80) == (src & 0x80)) && ((res & 0x80) != (dst & 0x80)));
	}

	// store the result
	store_op_value(instr.operands[0], res);

	// print IP
	printf("ip:0x%x->0x%x ", registers.IP - instr.size, registers.IP);

	// print if any flags are set or have been changed
	if(registers.flags || flag_str[0] != 0) {
		printf("flags:%s->", flag_str);
		registers.flags_dbg_str(flag_str);
		printf("%s", flag_str);
	}
}

void exec_adc(instruction instr) {}
void exec_inc(instruction instr) {}
void exec_aaa(instruction instr) {}
void exec_daa(instruction instr) {}

void exec_sub(instruction instr) {
	char flag_str[16] = {};
	registers.flags_dbg_str(flag_str);

	// load operand values
	u16 dst = load_op_value(instr.operands[0]);
	u16 src = load_op_value(instr.operands[1]);
	u32 res = dst - src;

	// update flags
	if (instr.flags & Inst_Wide) {
		registers.set_flag(CF, res & (1 << 16));
		registers.set_flag(PF, !(__popcnt16(res & 0xFF) & 1));
		registers.set_flag(ZF, (res & 0xFFFF) == 0);
		registers.set_flag(SF, res & (1 << 15));

		registers.set_flag(AF, 0 != (((dst & 0xF) - (src & 0xF)) & ~0xF));
		registers.set_flag(OF, ((dst >> 15) != (src >> 15)) && ((res >> 15) != (dst >> 15)));
	} 
	else {
		registers.set_flag(CF, res & (1 << 8));
		registers.set_flag(PF, !(__popcnt16(res & 0xFF) & 1));
		registers.set_flag(ZF, (res & 0xFF) == 0);
		registers.set_flag(SF, res & (1 << 7));

		registers.set_flag(AF, 0 != (((dst & 0xF) - (src & 0xF)) & ~0xF));
		registers.set_flag(OF, ((dst & 0x80) != (src & 0x80)) && ((res & 0x80) != (dst & 0x80)));
	}

	// store the result
	store_op_value(instr.operands[0], res);
	
	// print IP
	printf("ip:0x%x->0x%x ", registers.IP - instr.size, registers.IP);

	// print if any flags are set or have been changed
	if(registers.flags || flag_str[0] != 0) {
		printf("flags:%s->", flag_str);
		registers.flags_dbg_str(flag_str);
		printf("%s", flag_str);
	}
}

void exec_sbb(instruction instr) {}
void exec_dec(instruction instr) {}
void exec_neg(instruction instr) {}

void exec_cmp(instruction instr) {
	char flag_str[16] = {};
	registers.flags_dbg_str(flag_str);

	// load operand values
	u16 dst = load_op_value(instr.operands[0]);
	u16 src = load_op_value(instr.operands[1]);
	u32 res = dst - src;

	// update flags
	if (instr.flags & Inst_Wide) {
		registers.set_flag(CF, res & (1 << 16));
		registers.set_flag(PF, !(__popcnt16(res & 0xFF) & 1));
		registers.set_flag(ZF, (res & 0xFFFF) == 0);
		registers.set_flag(SF, res & (1 << 15));

		registers.set_flag(AF, 0 != (((dst & 0xF) - (src & 0xF)) & ~0xF));
		registers.set_flag(OF, ((dst >> 15) != (src >> 15)) && ((res >> 15) != (dst >> 15)));
	} 
	else {
		registers.set_flag(CF, res & (1 << 8));
		registers.set_flag(PF, !(__popcnt16(res & 0xFF) & 1));
		registers.set_flag(ZF, (res & 0xFF) == 0);
		registers.set_flag(SF, res & (1 << 7));
		
		registers.set_flag(AF, 0 != (((dst & 0xF) - (src & 0xF)) & ~0xF));
		registers.set_flag(OF, ((dst & 0x80) != (src & 0x80)) && ((res & 0x80) != (dst & 0x80)));
	}

	// print IP
	printf("ip:0x%x->0x%x ", registers.IP - instr.size, registers.IP);

	// print if any flags are set or have been changed
	if(registers.flags || flag_str[0] != 0) {
		printf(" ; flags:%s->", flag_str);
		registers.flags_dbg_str(flag_str);
		printf("%s", flag_str);
	}
}

void exec_aas(instruction instr) {}
void exec_das(instruction instr) {}
void exec_mul(instruction instr) {}
void exec_imul(instruction instr) {}
void exec_aam(instruction instr) {}
void exec_div(instruction instr) {}
void exec_idiv(instruction instr) {}
void exec_aad(instruction instr) {}
void exec_cbw(instruction instr) {}
void exec_cwd(instruction instr) {}
void exec_not(instruction instr) {}
void exec_shl(instruction instr) {}
void exec_shr(instruction instr) {}
void exec_sar(instruction instr) {}
void exec_rol(instruction instr) {}
void exec_ror(instruction instr) {}
void exec_rcl(instruction instr) {}
void exec_rcr(instruction instr) {}
void exec_and(instruction instr) {}
void exec_test(instruction instr) {}
void exec_or(instruction instr) {}
void exec_xor(instruction instr) {}
void exec_rep(instruction instr) {}
void exec_movs(instruction instr) {}
void exec_cmps(instruction instr) {}
void exec_scas(instruction instr) {}
void exec_lods(instruction instr) {}
void exec_stos(instruction instr) {}
void exec_call(instruction instr) {}
void exec_jmp(instruction instr) {}
void exec_ret(instruction instr) {}
void exec_retf(instruction instr) {}

inline void conditional_jump(instruction instr, bool jump) {
	u16 original_ip = registers.IP - instr.size;
	if (jump) {
		registers.IP += instr.operands[0].Immediate.value;
	}
	printf("ip:0x%x->0x%x ", original_ip, registers.IP);
}

void exec_je(instruction instr)  { conditional_jump(instr,  registers[ZF]); }
void exec_jne(instruction instr) { conditional_jump(instr, !registers[ZF]); }

void exec_jl(instruction instr)  { conditional_jump(instr,   registers[SF] ^ registers[OF]); }
void exec_jnl(instruction instr) { conditional_jump(instr, !(registers[SF] ^ registers[OF])); }

void exec_jle(instruction instr) { conditional_jump(instr,   (registers[SF] ^ registers[OF]) || registers[ZF]); }
void exec_jg(instruction instr)  { conditional_jump(instr, !((registers[SF] ^ registers[OF]) || registers[ZF])); }

void exec_jb(instruction instr)  { conditional_jump(instr,  registers[CF]); }
void exec_jnb(instruction instr) { conditional_jump(instr, !registers[CF]); }

void exec_jbe(instruction instr) { conditional_jump(instr,   registers[CF] || registers[ZF]); }
void exec_ja(instruction instr)  { conditional_jump(instr, !(registers[CF] || registers[ZF])); }

void exec_jp(instruction instr)  { conditional_jump(instr,  registers[PF]); }
void exec_jnp(instruction instr) { conditional_jump(instr, !registers[PF]); }

void exec_jo(instruction instr)  { conditional_jump(instr,  registers[OF]); }
void exec_jno(instruction instr) { conditional_jump(instr, !registers[OF]); }

void exec_js(instruction instr)  { conditional_jump(instr,  registers[SF]); }
void exec_jns(instruction instr) { conditional_jump(instr, !registers[SF]); }

void exec_jcxz(instruction instr) { conditional_jump(instr, registers[CX] == 0); }

void exec_loop(instruction instr)   { registers[CX]--; conditional_jump(instr, registers[CX]); }
void exec_loopz(instruction instr)  { registers[CX]--; conditional_jump(instr, registers[CX] &&  registers[ZF]); }
void exec_loopnz(instruction instr) { registers[CX]--; conditional_jump(instr, registers[CX] && !registers[ZF]); }

void exec_int(instruction instr) {}
void exec_int3(instruction instr) {} 
void exec_into(instruction instr) {}
void exec_iret(instruction instr) {}
void exec_clc(instruction instr) {}
void exec_cmc(instruction instr) {}
void exec_stc(instruction instr) {}
void exec_cld(instruction instr) {}
void exec_std(instruction instr) {}
void exec_cli(instruction instr) {}
void exec_sti(instruction instr) {}
void exec_hlt(instruction instr) {}
void exec_wait(instruction instr) {}
void exec_esc(instruction instr) {}
void exec_lock(instruction instr) {}
void exec_segment(instruction instr) {}