#include "../headers/program_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include "../headers/ias.h"
#include <errno.h>

Data* data_arr = NULL;
half_word* instruction_arr = NULL;
size_t data_initial_size = 10;
size_t instruction_initial_size = 50;
const char* MEMORY_ALLOCATION_FAILED_ERROR_MESSAGE = "Memory allocation failed in program_loader.c\n";

int allocateDataArr(Data* data_arr_ptr) {
    data_arr_ptr = (Data*) malloc(data_initial_size * sizeof(Data));
    if(!data_arr_ptr) {
        perror(MEMORY_ALLOCATION_FAILED_ERROR_MESSAGE);
        return errno; 
    }

    return SUCCESSFUL;
}

int allocateInsArr(half_word* instruction_arr_ptr) {
    instruction_arr_ptr = (half_word*) malloc(instruction_initial_size * sizeof(half_word));
    if(!instruction_arr_ptr) {
        perror(MEMORY_ALLOCATION_FAILED_ERROR_MESSAGE);
        return errno;
    }

    return SUCCESSFUL;
}

int increaseDataArrSize(Data* data_arr_ptr) {
    data_arr_ptr = realloc(data_arr_ptr, data_initial_size * 2);
    if(!data_arr_ptr) {
        perror(MEMORY_ALLOCATION_FAILED_ERROR_MESSAGE);
        return errno;
    }

    data_initial_size*=2;
    return SUCCESSFUL;
}

int increaseInsArrSize(half_word* instruction_arr_ptr) {
    instruction_arr_ptr = realloc(instruction_arr_ptr, instruction_initial_size * 2);
    if(!instruction_arr_ptr) {
        perror(MEMORY_ALLOCATION_FAILED_ERROR_MESSAGE);
        return errno;
    }

    instruction_initial_size*=2;
    return SUCCESSFUL;
}