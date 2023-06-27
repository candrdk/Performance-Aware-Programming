#pragma once

#include <stdint.h>

typedef uint32_t u32;
typedef uint64_t u64;

#ifndef MAX_MEASUREMENTS
	#define MAX_MEASUREMENTS 128
#endif

#define CONCAT_(prefix, suffix) prefix##suffix
#define CONCAT(prefix, suffix) CONCAT_(prefix, suffix)
#define UNIQUE_IDENTIFIER(prefix) CONCAT(prefix##_, __LINE__)

#define TIME_SCOPE(identifier) __timer UNIQUE_IDENTIFIER(timer) (__COUNTER__ + 1, identifier)
#define TIME_FUNCTION TIME_SCOPE(__FUNCTION__)
#define TIME_CALL(function) do { char f[] = #function; f[strcspn(f, "( \t")] = '\0'; TIME_SCOPE(f); function; } while(0)

#define BEGIN_TIMING __timer __begin_timer (0, __FUNCTION__)
#define END_TIMING __begin_timer.~__timer()

class __timer {
public:
	__timer(u32 counter, const char* name);
	~__timer();

private:
	u32 id;
};

void print_measurements(u64 cpuFrequency);