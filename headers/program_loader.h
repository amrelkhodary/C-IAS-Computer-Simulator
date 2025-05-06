typedef struct Data {
    address adr;
    word val;
} Data;

extern Data* data_arr; 
extern half_word* instruction_arr;
extern size_t data_intial_size;
extern size_t instruction_intial_size;
extern const char* MEMORY_ALLOCATION_FAILED_ERROR_MESSAGE;

int parse(FILE* program);
int allocateDataArr(Data* data_arr_ptr);
int allocateInsArr(half_word* instruction_arr_ptr);
int increaseDataArrSize(Data* data_arr_ptr); 
int increaseInsArrSize(half_word* instruction_arr_ptr);
