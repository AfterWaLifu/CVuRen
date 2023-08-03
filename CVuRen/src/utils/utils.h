#pragma once

#include <stdint.h>

void c_throw(const char* messege);

uint32_t u32_clamp(uint32_t v, uint32_t l, uint32_t h);

uint64_t getTimeInNanoseconds();