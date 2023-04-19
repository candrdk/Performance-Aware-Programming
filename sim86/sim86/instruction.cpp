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

void exec_mov(instruction instr) {
	instruction_operand to = instr.operands[0];
	instruction_operand from = instr.operands[1];

	if (to.type == operand_type::REGISTER) {
		register_index wide_index = to.Register.index;
		if (wide_index < 8 && to.Register.wide == false) wide_index = (register_index)(wide_index & 0b11);
		printf(" ; %s:0x%x->", registers.get_name(wide_index), registers.get_word(wide_index));

		if (from.type == operand_type::REGISTER) {
			if (to.Register.wide) {
				registers.get_word(to.Register.index) = registers.get_word(from.Register.index);
			}
			else {
				registers.get_byte(to.Register.index) = registers.get_byte(from.Register.index);
			}
		} 
		else if (from.type == operand_type::IMMEDIATE) {
			if (to.Register.wide) {
				registers.get_word(to.Register.index) = from.Immediate.value;
			} 
			else {
				registers.get_byte(to.Register.index) = from.Immediate.value;
			}
		}
		else if (from.type == operand_type::MEMORY) {
			assert(false); // TODO: not implemented
		}

		printf("0x%x", registers.get_word(wide_index));
	} 
	else if (to.type == operand_type::MEMORY) {
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