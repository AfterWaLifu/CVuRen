#pragma once

#include <stdlib.h>

/* dynamic array structure for int's */
typedef struct {
	int** array;
	size_t length;
} dynamic_array_int;

/* dynamic array structure for cstrings */
typedef struct {
	char** array;
	size_t length;
} dynamic_array_string;

/* implementation for int's */
void dai_pushback(dynamic_array_int* dai, int item);
void dai_popback(dynamic_array_int* dai);
void dai_insert(dynamic_array_int* dai, size_t position, int item);
void dai_erase(dynamic_array_int* dai, size_t position);
int dai_get(dynamic_array_int* dai, size_t position);
void dai_set(dynamic_array_int* dai, size_t position, int item);

/* implementation for cstrings */
void das_pushback(dynamic_array_string* dai, char* item);
void das_popback(dynamic_array_string* dai);
void das_insert(dynamic_array_string* dai, size_t position, char* item);
void das_erase(dynamic_array_string* dai, size_t position);
char* das_get(dynamic_array_string* dai, size_t position);
void das_set(dynamic_array_string* dai, size_t position, char* item);