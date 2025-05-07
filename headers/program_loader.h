typedef struct Data {
    address adr;
    word val;
} Data;

typedef struct Instruction {
    opcode op;
    address adr;
} Instruction;

extern Data* data_arr; 
extern Instruction* instruction_arr;
extern int data_arr_index;
extern int ins_arr_index;
extern const size_t data_intial_size;
extern const size_t instruction_intial_size;
extern const size_t buffersize;
extern const char* MEMORY_ALLOCATION_FAILED_ERROR_MESSAGE;
extern const char* FILE_OPENING_FAILED_ERROR_MESSAGE;
extern const char* DATA_HEADER;
extern const char* PROGRAM_HEADER;
extern const char COMMENT_CHARACTER;

bool isdataheader(char* buffer, int line_number); 
bool isprogramheader(char* buffer, int line_number); 
bool isdatastatement(char* buffer); 
bool isprogstatement(char* buffer); 
int extractData(char* datastring);
int extractInstruction(char* inststring);
int parse(char* program_filepath);
int allocateDataArr(Data* data_arr_ptr);
int allocateInsArr(half_word* instruction_arr_ptr);
int increaseDataArrSize(Data* data_arr_ptr); 
int increaseInsArrSize(half_word* instruction_arr_ptr);
int loadData(IAS* ias, Data* data_arr);
int loadInstructions(IAS* ias, Instruction* instruction_arr);