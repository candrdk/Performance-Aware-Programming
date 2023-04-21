#include "sim86.h"
#include "sim86_instruction_table.h"

struct decode_context {
	u32 flags;
	reg16_t default_segment;
};

u32 parse_data_value(u8** ptr, bool exists, bool wide, bool sign_extended) {
	u32 result = 0;

	if (exists) {
		if (wide) {
			u8 lo = (*ptr)[0];
			u8 hi = (*ptr)[1];
			result = (hi << 8) | lo;
			*ptr += 2;
		}
		else {
			result = **ptr;
			if (sign_extended) {
				result = (i32)*(i8*)&result;
			}
			*ptr += 1;
		}
	}

	return result;
}

instruction try_decode(decode_context* context, instruction_encoding* encoding, u8* ptr) {
	//TODO: proper addressing
	u32 starting_address = (u32)(uintptr_t)ptr;
	instruction result = {};
	bool valid = true;

	bool has[Bits_Count] = {};
	u32 bits[Bits_Count] = {};

	u32 bits_pending_count = 0;
	u32 bits_pending = 0;
	for (u32 i = 0; valid && (i < sizeof(encoding->bits) / sizeof(encoding->bits[0])); i++) {
		instruction_bits test_bits = encoding->bits[i];

		if (test_bits.usage == Bits_End) break;

		// Does this part of the encoding involve reading bits from the bitstream?
		// Otherwise, if count == 0, value holds the bits of an implicit value.
		u32 read_bits = test_bits.value;
		if (test_bits.count != 0) {
			// If we have no left-over bits, read a new byte
			if (bits_pending_count == 0) {
				bits_pending_count = 8;
				bits_pending = *ptr++;
			}

			assert(test_bits.count <= bits_pending_count);

			bits_pending_count -= test_bits.count;
			read_bits = bits_pending;
			read_bits >>= bits_pending_count;			// shift read bits down
			read_bits &= ~(0xff << test_bits.count);	// mask off the lowest count bits
		}

		// If we are testing for a literal, check that it matches.
		// Otherwise, store the read bits and mark them as seen.
		if (test_bits.usage == Bits_Literal) {
			valid = valid && (read_bits == test_bits.value);
		}
		else {
			bits[test_bits.usage] |= (read_bits << test_bits.shift);
			has[test_bits.usage] = true;
		}
	}

	if (valid) {
		u32 MOD = bits[Bits_MOD];
		u32 RM = bits[Bits_RM];
		bool W = bits[Bits_W];
		bool S = bits[Bits_S];
		bool D = bits[Bits_D];

		bool has_direct_address = (MOD == 0b00) && (RM == 0b110);
		has[Bits_Disp] = has[Bits_Disp] || (MOD == 0b10) || (MOD == 0b01) || has_direct_address;

		bool wide_displacement = bits[Bits_DispAlwaysW] || (MOD == 0b10) || has_direct_address;
		bool wide_data = bits[Bits_WMakesDataW] && !S && W;

		bits[Bits_Disp] |= parse_data_value(&ptr, has[Bits_Disp], wide_displacement, !wide_displacement);
		bits[Bits_Data] |= parse_data_value(&ptr, has[Bits_Data], wide_data, S);

		result = {
			.address = starting_address,
			.size = (u32)(uintptr_t)ptr - starting_address,
			.op = encoding->op,
			.flags = context->flags,
			.segment_override = context->default_segment
		};
		
		if (W) {
			result.flags |= Inst_Wide;
		}
		if (bits[Bits_Far]) {
			result.flags |= Inst_Far;
		}
		
		u32 temp_disp = bits[Bits_Disp];
		i16 displacement = (i16)temp_disp;

		instruction_operand* reg_operand = &result.operands[D ? 0 : 1];
		instruction_operand* mod_operand = &result.operands[D ? 1 : 0];

		if (has[Bits_SR]) {
			reg_operand->type = operand_type::REGISTER;
			reg_operand->Register = { true, reg16_t::ES + (bits[Bits_SR] & 0b11)};
		}

		if (has[Bits_REG]) {
			reg_operand->type = operand_type::REGISTER;
			reg_operand->Register = { W, bits[Bits_REG]};
		}

		if (has[Bits_MOD]) {
			if (MOD == 0b11) {
				mod_operand->type = operand_type::REGISTER;
				mod_operand->Register = { W || bits[Bits_RMRegAlwaysW], bits[Bits_RM] };
			}
			else {
				mod_operand->type = operand_type::MEMORY;
				if ((MOD == 0b00) && (RM == 0b110)) {
					mod_operand->Address = { reg16_t::NONE, reg16_t::NONE, displacement };
				}
				else {
					mod_operand->Address = { rm_table[RM][0], rm_table[RM][1], displacement };
				}
			}
		}

		if (has[Bits_Data] && has[Bits_Disp] && !has[Bits_MOD]) {
			result.operands[0].type = operand_type::MEMORY;
			result.operands[0].Address = {
				.displacement = displacement,
				.segment = reg16_t(bits[Bits_Data]),
				.explicit_segment = true
			};
		}
		else {
			instruction_operand* last_operand = &result.operands[0];
			if (last_operand->type != operand_type::NONE) {
				last_operand = &result.operands[1];
			}

			if (bits[Bits_RelJMPDisp]) {
				last_operand->type = operand_type::IMMEDIATE;
				last_operand->Immediate = { displacement, true };
			}
			else if (has[Bits_Data]) {
				last_operand->type = operand_type::IMMEDIATE;
				last_operand->Immediate = { (i32)bits[Bits_Data] };
			}
			else if (has[Bits_V]) {		// V = 0		Shift/rotate count is one
				if (bits[Bits_V]) {		// V = 1		Shift/rotate count is specified in CL register.
					last_operand->type = operand_type::REGISTER;
					last_operand->Register = { false, reg16_t::CX };
				}
				else {
					last_operand->type = operand_type::IMMEDIATE;
					last_operand->Immediate = { 1 };
				}
			}
		}
	}

	return result;
}

instruction decode_instruction(u8* ptr) {
	//TODO: fix this to calculate proper absolute address.
	decode_context context = {};
	instruction result = {};

	u32 start_address = ((uintptr_t)ptr & 0xFFFFFFFF);

	u32 bytes_read = 0;
	while (bytes_read < 15) {	// intel specified maximum possible length of instruction
		result = {};
		for (u32 i = 0; i < (sizeof(instruction_table) / sizeof(instruction_table[0])); i++) {
			instruction_encoding inst = instruction_table[i];
			result = try_decode(&context, &inst, ptr);

			if (result.op) {
				ptr += result.size;
				bytes_read += result.size;
				break;
			}
		}

		if (result.op == Op_lock) {
			context.flags |= Inst_Lock;
		}
		else if (result.op == Op_rep) {
			context.flags |= Inst_Rep;
		}
		else if (result.op == Op_segment) {
			context.flags |= Inst_Segment;
			context.default_segment = reg16_t(result.operands[1].Register.reg16);
		}
		else {
			break;
		}
	}

	if (bytes_read <= 15) {
		result.address = start_address;
		result.size = bytes_read;
	}
	else {
		result = {};
	}

	return result;
}