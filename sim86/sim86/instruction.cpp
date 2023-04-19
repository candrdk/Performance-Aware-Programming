#include "instruction.h"

void (*exec_op_table[Op_Count])(instruction) = {
	nullptr,
#define INST(Mnemonic, ...) exec_##Mnemonic,
#define INSTALT(...)
#include "sim86_instruction_table.inl"
};

void execute_instruction(instruction instr) {
	exec_op_table[instr.op](instr);
}

/* 
TODO: this is a temporary fix untill i figure all my header stuff out
*/
void dump_registers(FILE* stream) {
	fprintf(stream, "Final registers:\n");
	fprintf(stream, "%s: 0x%4x (%d)\n", registers.name(AX, true), registers[AX], registers[AX]);
	fprintf(stream, "%s: 0x%4x (%d)\n", registers.name(BX, true), registers[BX], registers[BX]);
	fprintf(stream, "%s: 0x%4x (%d)\n", registers.name(CX, true), registers[CX], registers[CX]);
	fprintf(stream, "%s: 0x%4x (%d)\n", registers.name(DX, true), registers[DX], registers[DX]);
	fprintf(stream, "%s: 0x%4x (%d)\n", registers.name(SP, true), registers[SP], registers[SP]);
	fprintf(stream, "%s: 0x%4x (%d)\n", registers.name(BP, true), registers[BP], registers[BP]);
	fprintf(stream, "%s: 0x%4x (%d)\n", registers.name(SI, true), registers[SI], registers[SI]);
	fprintf(stream, "%s: 0x%4x (%d)\n", registers.name(DI, true), registers[DI], registers[DI]);
	fprintf(stream, "%s: 0x%4x (%d)\n", registers.name(ES, true), registers[ES], registers[ES]);
	fprintf(stream, "%s: 0x%4x (%d)\n", registers.name(SS, true), registers[SS], registers[SS]);
	fprintf(stream, "%s: 0x%4x (%d)\n", registers.name(DS, true), registers[DS], registers[DS]);
}

void exec_mov(instruction instr) {
	instruction_operand dst_op = instr.operands[0];
	instruction_operand src_op = instr.operands[1];

	if (dst_op.type == operand_type::REGISTER) {
		Register dst = dst_op.Register;

		reg16_t reg16 = dst.reg16;
		if (reg16 < 8 && !dst.wide) reg16 = reg16_t(reg16 & 0b11);
		printf(" ; %s:0x%x->", registers.name(reg16), registers[reg16]);

		if (src_op.type == operand_type::REGISTER) {
			Register src = src_op.Register;
			if (dst.wide) {
				registers[dst.reg16] = registers[src.reg16];
			}
			else {
				registers[dst.reg8] = registers[src.reg8];
			}
		}
		else if (src_op.type == operand_type::IMMEDIATE) {
			Immediate src = src_op.Immediate;
			if (dst.wide) {
				registers[dst.reg16] = src.value;
			}
			else {
				registers[dst.reg8] = src.value;
			}
		}
		else if (src_op.type == operand_type::MEMORY) {
			assert(false); // TODO: not implemented
		}

		printf("0x%x", registers[reg16]);
	}
	else if (dst_op.type == operand_type::MEMORY) {
		assert(false); // TODO: not implemented
	}
};

void exec_push(instruction instr) {};
void exec_pop(instruction instr) {};
void exec_xchg(instruction instr) {};
void exec_in(instruction instr) {};
void exec_out(instruction instr) {};
void exec_xlat(instruction instr) {};
void exec_lea(instruction instr) {};
void exec_lds(instruction instr) {};
void exec_les(instruction instr) {};
void exec_lahf(instruction instr) {};
void exec_sahf(instruction instr) {};
void exec_pushf(instruction instr) {};
void exec_popf(instruction instr) {};
void exec_add(instruction instr) {};
void exec_adc(instruction instr) {};
void exec_inc(instruction instr) {};
void exec_aaa(instruction instr) {};
void exec_daa(instruction instr) {};
void exec_sub(instruction instr) {};
void exec_sbb(instruction instr) {};
void exec_dec(instruction instr) {};
void exec_neg(instruction instr) {};
void exec_cmp(instruction instr) {};
void exec_aas(instruction instr) {};
void exec_das(instruction instr) {};
void exec_mul(instruction instr) {};
void exec_imul(instruction instr) {};
void exec_aam(instruction instr) {};
void exec_div(instruction instr) {};
void exec_idiv(instruction instr) {};
void exec_aad(instruction instr) {};
void exec_cbw(instruction instr) {};
void exec_cwd(instruction instr) {};
void exec_not(instruction instr) {};
void exec_shl(instruction instr) {};
void exec_shr(instruction instr) {};
void exec_sar(instruction instr) {};
void exec_rol(instruction instr) {};
void exec_ror(instruction instr) {};
void exec_rcl(instruction instr) {};
void exec_rcr(instruction instr) {};
void exec_and(instruction instr) {};
void exec_test(instruction instr) {}; 
void exec_or(instruction instr) {};
void exec_xor(instruction instr) {};
void exec_rep(instruction instr) {};
void exec_movs(instruction instr) {};
void exec_cmps(instruction instr) {};
void exec_scas(instruction instr) {};
void exec_lods(instruction instr) {};
void exec_stos(instruction instr) {};
void exec_call(instruction instr) {};
void exec_jmp(instruction instr) {};
void exec_ret(instruction instr) {};
void exec_retf(instruction instr) {};
void exec_je(instruction instr) {};
void exec_jl(instruction instr) {};
void exec_jle(instruction instr) {};
void exec_jb(instruction instr) {};
void exec_jbe(instruction instr) {};
void exec_jp(instruction instr) {};
void exec_jo(instruction instr) {};
void exec_js(instruction instr) {};
void exec_jne(instruction instr) {};
void exec_jnl(instruction instr) {};
void exec_jg(instruction instr) {};
void exec_jnb(instruction instr) {};
void exec_ja(instruction instr) {};
void exec_jnp(instruction instr) {};
void exec_jno(instruction instr) {};
void exec_jns(instruction instr) {};
void exec_loop(instruction instr) {};
void exec_loopz(instruction instr) {};
void exec_loopnz(instruction instr) {};
void exec_jcxz(instruction instr) {};
void exec_int(instruction instr) {};
void exec_int3(instruction instr) {}; 
void exec_into(instruction instr) {};
void exec_iret(instruction instr) {};
void exec_clc(instruction instr) {};
void exec_cmc(instruction instr) {};
void exec_stc(instruction instr) {};
void exec_cld(instruction instr) {};
void exec_std(instruction instr) {};
void exec_cli(instruction instr) {};
void exec_sti(instruction instr) {};
void exec_hlt(instruction instr) {};
void exec_wait(instruction instr) {};
void exec_esc(instruction instr) {};
void exec_lock(instruction instr) {};
void exec_segment(instruction instr) {};