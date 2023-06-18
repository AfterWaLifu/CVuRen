#include "dynamic_array.h"

#include <string.h>

#define mallok(type, number) (type*)malloc(sizeof(type) * number)

void dai_pushback(dynamic_array_int* dai, int item) {
	int* new_one = mallok(int, 1);
	*new_one = item;

	if (dai->length % 10 == 0) {
		int** new_array = mallok(int*, dai->length + 10);
		if (dai->array) {
			for (size_t i = 0; i < dai->length; ++i) {
				new_array[i] = dai->array[i];
			}
			free(dai->array);
		}
		dai->array = new_array;
	}
	dai->array[dai->length] = new_one;
	++(dai->length);
}

void dai_popback(dynamic_array_int* dai) {
	if (dai->length) {
		free(dai->array[dai->length - 1]);
		dai->array[dai->length - 1] = NULL;
		--(dai->length);
		if (dai->length && (dai->length % 10 == 0)) {
			int** new_array = mallok(int*, dai->length);
			for (size_t i = 0; i < dai->length; ++i) {
				new_array[i] = dai->array[i];
			}
			free(dai->array);
			dai->array = new_array;
		}
	}
}

void dai_insert(dynamic_array_int* dai, size_t position, int item) {

}

void dai_erase(dynamic_array_int* dai, size_t position) {

}

int dai_get(dynamic_array_int* dai, size_t position) {
	if (position >= dai->length) return INT_MAX;
	return *(dai->array[position]);
}

void dai_set(dynamic_array_int* dai, size_t position, int item) {
	if (position >= dai->length) return;
	*(dai->array[position]) = item;
}

void dau_pushback(dynamic_array_uint* dau, uint32_t item) {
	uint32_t* new_one = mallok(uint32_t, 1);
	*new_one = item;

	if (dau->length % 10 == 0) {
		uint32_t** new_array = mallok(uint32_t*, dau->length + 10);
		if (dau->array) {
			for (size_t i = 0; i < dau->length; ++i) {
				new_array[i] = dau->array[i];
			}
			free(dau->array);
		}
		dau->array = new_array;
	}
	dau->array[dau->length] = new_one;
	++(dau->length);
}

void dau_popback(dynamic_array_uint* dau) {
	if (dau->length) {
		free(dau->array[dau->length - 1]);
		dau->array[dau->length - 1] = NULL;
		--(dau->length);
		if (dau->length && (dau->length % 10 == 0)) {
			uint32_t** new_array = mallok(uint32_t*, dau->length);
			for (size_t i = 0; i < dau->length; ++i) {
				new_array[i] = dau->array[i];
			}
			free(dau->array);
			dau->array = new_array;
		}
	}
}

void dau_insert(dynamic_array_uint* dau, size_t position, uint32_t item) {

}

void dau_erase(dynamic_array_uint* dau, size_t position) {

}

uint32_t dau_get(dynamic_array_uint* dau, size_t position) {
	if (position >= dau->length) return UINT32_MAX;
	return *(dau->array[position]);
}

void dau_set(dynamic_array_uint* dau, size_t position, uint32_t item) {
	if (position >= dau->length) return;
	*(dau->array[position]) = item;
}

void das_pushback(dynamic_array_string* das, char* item) {
	if (das->length % 10 == 0) {
		char** new_array = mallok(char*, das->length + 10);
		if (das->array) {
			for (size_t i = 0; i < das->length; ++i) {
				new_array[i] = das->array[i];
			}
			free(das->array);
		}
		das->array = new_array;
	}
	das->array[das->length] = item;
	++(das->length);
}

void das_popback(dynamic_array_string* das) {
	if (das->length) {
		free(das->array[das->length - 1]);
		das->array[das->length - 1] = NULL;
		if (das->length && (das->length % 10 == 0)) {
			char** new_array = mallok(char*, das->length);
			for (size_t i = 0; i < das->length; ++i) {
				new_array[i] = das->array[i];
			}
			free(das->array);
			das->array = new_array;
		}
	}
}

void das_insert(dynamic_array_string* das, size_t position, char* item) {
}

void das_erase(dynamic_array_string* das, size_t position) {
}

char* das_get(dynamic_array_string* das, size_t position) {
	if (position >= das->length) return NULL;
	return das->array[position];
}

void das_set(dynamic_array_string* das, size_t position, char* item) {
	if (position >= das->length) return;
	free(das->array[position]);
	das->array[position] = item;
}
