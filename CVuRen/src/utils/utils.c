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