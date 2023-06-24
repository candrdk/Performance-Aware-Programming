#pragma once

u64 GetOSTimerFreq() {
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return freq.QuadPart;
}

u64 ReadOSTimer() {
	LARGE_INTEGER value;
	QueryPerformanceCounter(&value);
	return value.QuadPart;
}

u64 ReadCPUTimer() {
	return __rdtsc();
}

u64 EstimateCPUFreq(bool printResults) {
	u64 OSFreq = GetOSTimerFreq();

	u64 CPUStart = ReadCPUTimer();
	u64 OSStart = ReadOSTimer();

	u64 OSEnd = 0;
	u64 OSElapsed = 0;
	u64 OSWaitTime = OSFreq * 0.1;

	while (OSElapsed < OSWaitTime) {
		OSEnd = ReadOSTimer();
		OSElapsed = OSEnd - OSStart;
	}

	u64 CPUEnd = ReadCPUTimer();
	u64 CPUElapsed = CPUEnd - CPUStart;

	u64 CPUFreq = 0;
	if (OSElapsed) {
		CPUFreq = OSFreq * CPUElapsed / OSElapsed;
	}

	if (printResults) {
		printf("      OS Freq: %llu (reported)\n", OSFreq);
		printf("     OS Timer: %llu -> %llu = %llu elapsed\n", OSStart, OSEnd, OSElapsed);
		printf("   OS Seconds: %.4f\n", (f64)OSElapsed / (f64)OSFreq);
		printf("    CPU Timer: %llu -> %llu = %llu elapsed\n", CPUStart, CPUEnd, CPUElapsed);
		printf("Est. CPU Freq: %llu (~%.2f ghz)\n", CPUFreq, CPUFreq / 1000000000.0);
	}

	return CPUFreq;
}