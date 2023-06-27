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

#include "Instrumentation.h"

struct Pair {
    f64 x0, y0, x1, y1;
};

u64 ParseHaversinePairs(Buffer inputJSON, u64 maxPairCount, Pair* pairs) {
    TIME_FUNCTION;
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
    TIME_FUNCTION;
    f64 sum = 0.0;
    f64 coeff = 1.0 / (f64)count;
    for (int i = 0; i < count; i++) {
        Pair p = pairs[i];
        sum += coeff * ReferenceHaversine(p.x0, p.y0, p.x1, p.y1, 6372.8);
    }
    return sum;
}

int main(int argc, char* argv[]) {
    BEGIN_TIMING;

#ifdef _DEBUG
    FILE* fp = fopen("C:\\Users\\caleb\\source\\repos\\Performance-Aware-Programming\\haversine\\x64\\Debug\\haversine_0_1000000.json", "rb");
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

        if (input.data) {
            TIME_SCOPE("Read files");
            if (fread(input.data, input.count, 1, fp) != 1) {
                fprintf(stderr, "ERROR: Unable to read \"%s\".\n", argv[1]);
                FreeBuffer(&input);
            }
        }
        fclose(fp);

        u64 maxPairCount = input.count / 24;
        if (maxPairCount) {
            Buffer parsedPairs = AllocateBuffer(maxPairCount * sizeof(Pair));
            if (parsedPairs.count) {
                u64 pairCount = ParseHaversinePairs(input, 1000000, (Pair*)parsedPairs.data);
                f64 sum = SumHaversineDistances(pairCount, (Pair*)parsedPairs.data);

                fprintf(stdout, "Input size: %llu\n", input.count);
                fprintf(stdout, "Pair count: %llu\n", pairCount);
                fprintf(stdout, "Haversine sum: %f\n", sum);
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

    END_TIMING;

    print_measurements(EstimateCPUFreq(false));

    return 0;
}
