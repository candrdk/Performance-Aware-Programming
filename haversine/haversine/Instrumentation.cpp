#include "Instrumentation.h"

#include <intrin.h>
#include <assert.h>
#include <stdio.h>

struct __measurement {
    u32 scope;
    u32 count;
    const char* name;
    u64 begin;
    u64 end;
    u64 elapsed;
};

static __measurement __measurements[MAX_MEASUREMENTS];
static u32 __scope_depth = 0;

__timer::__timer(u32 counter, const char* name) : id{ counter } {
    assert(id < MAX_MEASUREMENTS);
    __measurement& m = __measurements[id];
    m.count++;
    m.scope = __scope_depth++;
    m.name = name;
    m.begin = __rdtsc();
}

__timer::~__timer() {
    __measurement& m = __measurements[id];
    m.end = __rdtsc();
    m.elapsed *= float(m.count - 1) / m.count;
    m.elapsed += (m.end - m.begin) / m.count;
    __scope_depth--;
}

void print_measurements(u64 cpuFrequency) {
    printf("\nTimings:\n");

    u32 prev_scope = 0;
    for (int i = 0; i < MAX_MEASUREMENTS; i++) {
        __measurement m = __measurements[i];
        if (!m.name) return;

        u64 parent_elapsed = m.elapsed;
        const char* parent_name = nullptr;
        for (int j = i; j >= 0; j--) {
            __measurement p = __measurements[j];
            if (p.scope < m.scope) {
                parent_elapsed = p.elapsed;
                parent_name = p.name;
                break;
            }
        }
        float percentage = 100.0f * (float)m.elapsed / parent_elapsed;
        for (int i = 0; i < m.scope; i++) printf("  ");
        printf("%s[%u]: %.2f seconds (%.2f%%)\n", m.name, m.count, m.elapsed / (double)cpuFrequency, percentage);
    }
}