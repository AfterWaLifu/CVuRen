#include "dynamic_array.h"

#include <string.h>

#define mallok(type, i) (type)malloc(sizeof(type) * i)

void dai_pushback(dynamic_array_int* dai, int item) {
	int* new_one = mallok(int*, 1);
	*new_one = item;

	if (dai->length % 10 == 0) {
		int** new_array = mallok(int**, dai->length + 10);
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
			int** new_array = mallok(int**, dai->length);
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

void das_pushback(dynamic_array_string* dai, char* item) {
	if (dai->length % 10 == 0) {
		char** new_array = mallok(char**, dai->length + 10);
		if (dai->array) {
			for (size_t i = 0; i < dai->length; ++i) {
				new_array[i] = dai->array[i];
			}
			free(dai->array);
		}
		dai->array = new_array;
	}
	dai->array[dai->length] = item;
	++(dai->length);
}

void das_popback(dynamic_array_string* dai) {
	if (dai->length) {
		free(dai->array[dai->length - 1]);
		dai->array[dai->length - 1] = NULL;
		if (dai->length && (dai->length % 10 == 0)) {
			char** new_array = mallok(char**, dai->length);
			for (size_t i = 0; i < dai->length; ++i) {
				new_array[i] = dai->array[i];
			}
			free(dai->array);
			dai->array = new_array;
		}
	}
}

void das_insert(dynamic_array_string* dai, size_t position, char* item) {
}

void das_erase(dynamic_array_string* dai, size_t position) {
}

char* das_get(dynamic_array_string* dai, size_t position) {
	if (position >= dai->length) return NULL;
	return dai->array[position];
}

void das_set(dynamic_array_string* dai, size_t position, char* item) {
	if (position >= dai->length) return;
	free(dai->array[position]);
	dai->array[position] = item;
}
