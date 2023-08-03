#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

void c_throw(const char* messege){
	fprintf(stderr, "\n|===== ERROR =====|\n%s\n", messege);
	abort();
}

uint32_t u32_clamp(uint32_t v, uint32_t l, uint32_t h) {
	if (v < l) return l;
	if (v > h) return h;
	return v;
}

#include <windows.h>
uint64_t getTimeInNanoseconds() {
    const long long ns_in_us = 1000;
    const long long ns_in_ms = 1000 * ns_in_us;
    const long long ns_in_s = 1000 * ns_in_ms;

    LARGE_INTEGER freq;
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    QueryPerformanceFrequency(&freq);

    if (freq.LowPart == 0 && freq.HighPart == 0) {
        c_throw("can't get cpu frequancy");
    }

    if (count.QuadPart < MAXLONGLONG / ns_in_s) {
        if (freq.QuadPart != 0) {
            return count.QuadPart * ns_in_s / freq.QuadPart;
        }
        else return count.QuadPart * ns_in_s;
    }
    else {
        if (freq.QuadPart > ns_in_s) {
            return count.QuadPart / (freq.QuadPart / ns_in_s);
        }
        else return count.QuadPart / freq.QuadPart;
    }
}
