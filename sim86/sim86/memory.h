#pragma once
#include <stdlib.h>

struct Memory {
	u32 m_size;
	u8* m_memory;

	u32 physical_address(u16 segment, u16 offset) {
		return ((u32)segment << 4) + (u32)offset;
	}

	u8& operator()(u16 segment, u16 offset) {
		return m_memory[physical_address(segment, offset)];
	}

	bool allocate(u32 size) {
		if (!m_memory) {
			free(m_memory);
		}
		m_size = size;
		m_memory = (u8*)malloc(m_size);
		return m_memory;
	}
};