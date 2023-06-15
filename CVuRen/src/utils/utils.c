#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

void c_throw(const char* messege){
	fprintf(stderr, "\n|===== ERROR =====|\n%s\n", messege);
	abort();
}