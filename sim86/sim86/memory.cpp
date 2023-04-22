#include "sim86.h"
#include "stdlib.h"
#include "stdio.h"

#include "instruction.h"
#include "memory.h"

Memory memory;

u32 Memory::physical_address(u16 segment, u16 offset) {
	return ((u32)segment << 4) + (u32)offset;
}

u32 Memory::physical_address(EffectiveAddress address) {
	reg16_t segment = reg16_t::DS;
	if (address.register1 == reg16_t::BP) segment = reg16_t::SS;
	if (address.explicit_segment) segment = address.segment;

	u16 offset = 0;
	if (address.register1 != reg16_t::NONE) offset += registers[address.register1];
	if (address.register2 != reg16_t::NONE) offset += registers[address.register2];
	offset += address.displacement;

	return physical_address(registers[segment], offset);
}

u8& Memory::operator()(u16 segment, u16 offset) {
	return m_memory[physical_address(segment, offset)];
}

u8& Memory::operator()(u32 address) {
	return m_memory[address];
}

bool Memory::allocate(u32 size) {
	if (!m_memory) {
		free(m_memory);
	}
	m_size = size;
	m_memory = (u8*)malloc(m_size);
	return m_memory;
}

size_t Memory::load_from_file(const char* path, u16 segment, u16 offset) {
	FILE* file;
	fopen_s(&file, path, "rb");

	if (file) {
		u32 address = physical_address(segment, offset);
		size_t read = fread_s(m_memory + address, m_size - address, 1, 0x10000, file);
		fclose(file);
		return read;
	}
	else {
		fprintf(stderr, "ERROR: Unable to open %s.\n", path);
		return -1;
	}
}

size_t Memory::dump_to_file(const char* file_name) {
	FILE* file;
	fopen_s(&file, file_name, "wb");

	if (file) {
		size_t written = fwrite(m_memory, 1, m_size, file);
		fclose(file);
		return written;
	}
	else {
		fprintf(stderr, "ERROR: Unable to open %s.\n", file_name);
	}
}