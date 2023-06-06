#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

void c_throw(const char* messege){
	fprintf(stderr, messege);
	abort();
}