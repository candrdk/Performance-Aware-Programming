#pragma once

struct Memory {
	u32 m_size;
	u8* m_memory;

	u32 physical_address(u16 segment, u16 offset);
	u32 physical_address(EffectiveAddress address);

	u8& operator()(u16 segment, u16 offset);
	u8& operator()(u32 address);

	bool allocate(u32 size);
	size_t load_from_file(const char* path, u16 segment, u16 offset);
};

extern Memory memory;