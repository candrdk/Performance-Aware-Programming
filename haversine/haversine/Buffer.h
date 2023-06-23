#pragma once

struct Buffer {
    size_t count;
    u8* data;

    bool InBounds(u64 i) { return i < count; }

    u8& operator[](int i) { return data[i]; }
    const u8& operator[](int i) const { return data[i]; }

    bool operator==(const Buffer& other) {
        if (count != other.count) return false;

        for (int i = 0; i < count; i++) {
            if (data[i] != other.data[i]) { return false; }
        }

        return true;
    }
};

Buffer AllocateBuffer(size_t size) {
    Buffer result = {};
    result.data = (u8*)malloc(size);
    if (result.data) {
        result.count = size;
    }
    else {
        fprintf(stderr, "ERROR: Unable to allocate %llu bytes.\n", size);
    }

    return result;
}

void FreeBuffer(Buffer* buffer) {
    if (buffer->data) {
        free(buffer->data);
    }
    *buffer = {};
}

template <size_t length>
Buffer StringBuffer(const char(&string)[length]) {
    return { .count = length - 1, .data = (u8*)string };
}