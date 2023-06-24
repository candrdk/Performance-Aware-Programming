#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>

#include <intrin.h>
#include <Windows.h>

typedef char u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#include "Buffer.h"
#include "ReferenceHaversine.h"
#include "JSON.h"
#include "Timers.h"

struct Pair {
    f64 x0, y0, x1, y1;
};

u64 ParseHaversinePairs(Buffer inputJSON, u64 maxPairCount, Pair* pairs) {
    u64 pairCount = 0;

    JSON_Element* json = ParseJSON(inputJSON);
    JSON_Element* pairsArray = LookupElement(json, StringBuffer("pairs"));

    if (pairs) {
        for(JSON_Element* element = pairsArray->firstSubElement;
            element && (pairCount < maxPairCount);
            element = element->next)
        {
            Pair* pair = pairs + pairCount++;
            *pair = {
                .x0 = ConvertElementToF64(element, StringBuffer("x0")),
                .y0 = ConvertElementToF64(element, StringBuffer("y0")),
                .x1 = ConvertElementToF64(element, StringBuffer("x1")),
                .y1 = ConvertElementToF64(element, StringBuffer("y1"))
            };
        }
    }

    FreeJSON(json);

    return pairCount;
}

f64 SumHaversineDistances(u64 count, Pair* pairs) {
    f64 sum = 0.0;
    f64 coeff = 1.0 / (f64)count;
    for (int i = 0; i < count; i++) {
        Pair p = pairs[i];
        sum += coeff * ReferenceHaversine(p.x0, p.y0, p.x1, p.y1, 6372.8);
    }
    return sum;
}

int main(int argc, char* argv[]) {
    u64 cpuFreq = EstimateCPUFreq(false);
    u64 start_time = ReadCPUTimer();

#ifdef _DEBUG
    FILE* fp = fopen("C:\\Users\\caleb\\dev\\repos\\Performance-Aware-Programming\\haversine\\x64\\Debug\\haversine_0_1000000.json", "rb");
#else
    if (argc != 2) {
        printf("Usage: haversine [haversine_input.json]");
        return 0;
    }

    FILE* fp = fopen(argv[1], "rb");
#endif
    Buffer input = {};

    if (fp) {
        fseek(fp, 0, SEEK_END);
        u64 size = ftell(fp);
        rewind(fp);

        input = AllocateBuffer(size);

        u64 read_start = ReadCPUTimer();
        if (input.data) {
            if (fread(input.data, input.count, 1, fp) != 1) {
                fprintf(stderr, "ERROR: Unable to read \"%s\".\n", argv[1]);
                FreeBuffer(&input);
            }
        }
        fclose(fp);
        u64 read_end = ReadCPUTimer();

        u64 maxPairCount = input.count / 24;
        if (maxPairCount) {
            Buffer parsedPairs = AllocateBuffer(maxPairCount * sizeof(Pair));
            if (parsedPairs.count) {
                u64 parse_start = ReadCPUTimer();
                u64 pairCount = ParseHaversinePairs(input, 1000000, (Pair*)parsedPairs.data);
                u64 parse_end = ReadCPUTimer();

                u64 sum_start = ReadCPUTimer();
                f64 sum = SumHaversineDistances(pairCount, (Pair*)parsedPairs.data);
                u64 sum_end = ReadCPUTimer();

                u64 end_time = ReadCPUTimer();

                fprintf(stdout, "Input size: %llu\n", input.count);
                fprintf(stdout, "Pair count: %llu\n", pairCount);
                fprintf(stdout, "Haversine sum: %f\n", sum);

                u64 misc_time = (read_start - start_time) + (parse_start - read_end);
                u64 read_time = read_end - read_start;
                u64 parse_time = parse_end - parse_start;
                u64 sum_time = sum_end - sum_start;
                f64 total_time = f64(end_time - start_time);
                fprintf(stdout, "\nTotal time: %f seconds\n",  total_time / (f64)cpuFreq);
                fprintf(stdout,   "Misc:  %llu (%.2f%%)\n",  misc_time, 100.0 * misc_time  / total_time);
                fprintf(stdout,   "Read:  %llu (%.2f%%)\n",  read_time, 100.0 * read_time  / total_time);
                fprintf(stdout,   "Parse: %llu (%.2f%%)\n", parse_time, 100.0 * parse_time / total_time);
                fprintf(stdout,   "Sum:   %llu (%.2f%%)\n",   sum_time, 100.0 * sum_time   / total_time);
            }
            FreeBuffer(&parsedPairs);
        }
        else {
            fprintf(stderr, "ERROR: Malformed input JSON\n");
        }

        FreeBuffer(&input);
    }
    else {
        fprintf(stderr, "ERROR: Unable to open \"%s\".\n", argv[1]);
    }

    return 0;
}
