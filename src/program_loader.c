//TODO: Continue implementing parse
#include "../headers/program_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include "../headers/ias.h"
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

Data* data_arr = NULL;
int data_arr_index = 0;
half_word* instruction_arr = NULL;
int ins_arr_index = 0;
size_t data_initial_size = 10;
size_t instruction_initial_size = 50;
const size_t buffersize = 256;
const char* MEMORY_ALLOCATION_FAILED_ERROR_MESSAGE = "Memory allocation failed in program_loader.c\n";
const char* FILE_OPENING_FAILED_ERROR_MESSAGE = "Failed to open the program file, please check that the provided path is correct.\n";
const char* DATA_HEADER = "data:";
const char* PROGRAM_HEADER = "program:";
const char COMMENT_CHARACTER = '#';
//allocate memory for the data array which will store extracted program data form the inputted prgoram text
int allocateDataArr(Data* data_arr_ptr) {
    data_arr_ptr = (Data*) malloc(data_initial_size * sizeof(Data));
    if(!data_arr_ptr) {
        perror(MEMORY_ALLOCATION_FAILED_ERROR_MESSAGE);
        return errno; 
    }

    return SUCCESSFUL;
}

//allocate memory for the instruction array which will store extracted program instructions from the inputted program text
int allocateInsArr(Instruction* instruction_arr_ptr) {
    instruction_arr_ptr = (Instruction*) malloc(instruction_initial_size * sizeof(Instruction));
    if(!instruction_arr_ptr) {
        perror(MEMORY_ALLOCATION_FAILED_ERROR_MESSAGE);
        return errno;
    }

    return SUCCESSFUL;
}

//increase the size allocated for teh data array
int increaseDataArrSize(Data* data_arr_ptr) {
    data_arr_ptr = realloc(data_arr_ptr, data_initial_size * 2);
    if(!data_arr_ptr) {
        perror(MEMORY_ALLOCATION_FAILED_ERROR_MESSAGE);
        return errno;
    }

    data_initial_size*=2;
    return SUCCESSFUL;
}

//increase the size allocated for the instruction array
int increaseInsArrSize(Instruction* instruction_arr_ptr) {
    instruction_arr_ptr = realloc(instruction_arr_ptr, instruction_initial_size * 2);
    if(!instruction_arr_ptr) {
        perror(MEMORY_ALLOCATION_FAILED_ERROR_MESSAGE);
        return errno;
    }

    instruction_initial_size*=2;
    return SUCCESSFUL;
}

//check if a line is empty
bool isempty(char* buffer) {
    for(int i = 0; i<buffersize && buffer[i] != '\n'; i++) {
        if(isspace(buffer[i]) == 0) {
            return false;
        }
    }

    return true;
}

//check if a line is a data header
bool isdataheader(char* buffer, int line_number) {
    bool found_dataheader = false;
    for(int i = 0; i<buffersize && buffer[i] != '\n'; i++) {
        if(isspace(buffer[i]) != 0) {continue;}
        if(buffer[i] == COMMENT_CHARACTER) {break;}
        //d -> i, a -> i+1, t -> i+2, a -> i+3, : -> i+4
        if(i+4 < buffersize) {
            if(buffer[i] == 'd' && buffer[i+1] == 'a' && buffer[i+2] == 't' && buffer[i+3] == 'a' && buffer[i+4] == ':') { found_dataheader = true;}
        }

        if(found_dataheader && isspace(buffer[i]) == 0) {
            fprintf(stderr, "Syntax Error at Line %i: Found text other than \"data:\"\n", line_number);
            return false;
        }
    }

    return found_dataheader;
}

//check if a line is a program header
bool isprogramheader(char* buffer, int line_number) {
    bool found_progheader = false;
    for(int i = 0; i<buffersize && buffer[i] != '\n'; i++) {
        if(isspace(buffer[i]) != 0) {continue;}
        if(buffer[i] == COMMENT_CHARACTER) {break;}
        //p -> i, r -> i+1, o -> i+2, g -> i+3, r -> i+4, a -> i+5, m -> i+6, : -> i+7
        if(i+4 < buffersize) {
            if(buffer[i] == 'p' && buffer[i+1] == 'r' && buffer[i+2] == 'o' && buffer[i+3] == 'g' 
               && buffer[i+4] == 'r' && buffer[i+5] == 'a' && buffer[i+6] == 'm' && buffer[i+7] == ':') { found_progheader = true;}
        }

        if(found_progheader && isspace(buffer[i]) == 0) {
            fprintf(stderr, "Syntax Error at Line %i: Found text other than \"program:\"\n", line_number);
            return false;
        }
    }

    return found_progheader;
}

//check if a line is a data statement
bool isdatastatement(char* buffer) {
    //remove the spaces from the line
    char nowhitespace_buffer[buffersize];
    int nowhitespace_buffer_index = 0;
    for(int i = 0; i<buffersize && buffer[i] != '\n'; i++) {
        if(isspace(buffer[i]) == 0) {
            nowhitespace_buffer[nowhitespace_buffer_index++] = buffer[i];        
        }
    }

    //now text should be "number,number"
    for(int i = 0; i<buffersize; i++) {
        if(!(nowhitespace_buffer[i] == ',' || isdigit(nowhitespace_buffer[i]))) {
            return false;
        }
    }

    return true;
}


//check if a line is a program statement
bool isprogstatement(char* buffer) {
    //remove the spaces from the line (except the one between the opcode and the address)
    char nowhitespace_buffer[buffersize];
    int nowhitespace_buffer_index = 0;
    for(int i = 0; i<buffersize && buffer[i] != '\n'; i++) {
        if(isspace(buffer[i]) == 0) {
            nowhitespace_buffer[nowhitespace_buffer_index++] = buffer[i];        
        } else {
            if(i>=1 && isupper(buffer[i-1]) && isdigit(buffer[i+1])) {
                nowhitespace_buffer[nowhitespace_buffer_index++] = buffer[i];        
            }
        }
    }

    //now text should be "alpha number"
    bool beforespace = true;
    for(int i = 0; i<buffersize; i++) {
        if(nowhitespace_buffer[i] == ' ') {beforespace = false;}
        if(!((beforespace && isupper(nowhitespace_buffer[i])) || (nowhitespace_buffer[i] == ' ') || (!beforespace && isdigit(nowhitespace_buffer[i])))) {
            return false;
        }
    }

    return true;
}

//extract address and value from a valid data statement
int extractData(char* datastring) {
    address adr = (address) 0; //initial value
    word val = (word) 0; //initial value

    //loop through the string to extract values
    bool foundComma = false;
    int position = 0;
    for(int i = strlen(datastring)-1; i>=0; i++) {
        if(datastring[i] == ',') {foundComma = true; position = 0; continue;}

        if(foundComma) {
            //add to address
            adr += ((address)datastring[i]) * ((address)pow(10, position++));
        } else {
            //add to value
            val += ((word)datastring[i]) * ((word)pow(10, position++));
        }
    }
    //create a new data struct and add it to the data array
    Data ndata; 
    ndata.adr = adr; ndata.val = val;
    if(data_arr_index == data_initial_size) {
        increaseDataArrSize(data_arr);
    }
    data_arr[data_arr_index++];
    return SUCCESSFUL;
}

//extract opcode and address from a validinstruction statement
int extractInstruction(char* inststring) {
    opcode op = (opcode) 0; //initial value
    address adr = (address) 0; //initial value

    //loop through the string to extract values
    bool foundSpace = false;
    int position = 0;
    for(int i = strlen(inststring)-1; i>=0; i++) {
        if(inststring[i] == ' ') {foundSpace = true; position = 0; continue;}

        if(foundSpace) {
            //add to address
            adr += ((address)inststring[i]) * ((address)pow(10, position++));
        } else {
            //add to opcode
            op += ((word)inststring[i]) * ((word)pow(10, position++));
        }
    }
    //create a new data struct and add it to the data array
    Instruction ninst; 
    ninst.op = op; ninst.adr = adr;
    if(ins_arr_index == instruction_initial_size) {
        increaseInsArrSize(instruction_arr);
    }
    instruction_arr[ins_arr_index++];
    return SUCCESSFUL;
}

//parse the inputted program text, validate it, and if valid, extract from it program data and instrcutions and store them in the data, program arrays respectively
int parse(char* program_filepath) {
    /*
        PROGRAM FORMAT:-

        data:
        address(numeric), value(numeric)
        program:
        opcode(alphabetic) address(numeric)
        ----------------------------------------
        WHAT TO IGNORE:-

        ignore comments started by the symbol #
        ingore whitespaces except those that differentiate between tokens
        ----------------------------------------
        EXAMPLE:-
        
        #this program adds 10 and 20
        data: 
            900, 10
            901, 20
        program:
            ADDMX 900
            ADDMX 901
        ----------------------------------------
    */

    //open the file
    FILE* program_fileptr = fopen(program_filepath, "r");
    if(!program_filepath) {
        perror(FILE_OPENING_FAILED_ERROR_MESSAGE);
        return errno;
    }

    //create the data and instruction arrays
    allocateDataArr(data_arr);
    allocateInsArr(instruction_arr);

    char buffer[buffersize];
    bool found_data_header = false;
    bool found_program_header = false;
    int line_number = 1;
    while(fgets(buffer, buffersize, program_fileptr) != EOF) {
        //skip the line if its empty
        if(isempty(buffer)) {continue;}
        
        /*
            A line is considered valid if it is 
            1- a data header (before a program header or any other statement is written) 
            2- a program header (after a data header)
            3- a data statement (after a data header and before a program header)
            4- a program statement (after a program and data header)
        */
        if(isdataheader(buffer, line_number) && !found_program_header) {
            found_data_header = true;
        }
        else if(isprogramheader(buffer, line_number) && found_data_header) {
            found_program_header = true;
        }
        else if(found_data_header && !found_program_header && isdatastatement(buffer)) {
            //TODO: extract data and store it in the data array
            extractData(buffer);
        }
        else if(found_data_header && found_program_header && isprogstatement(buffer)) {
            //TODO: extract opcode and adreess and store it in the instruction array
            extractInstruction(buffer);
        }
        else {
            fprintf(stderr, "Syntax error in your program file.\n");
            return SYNTAX_ERROR;
        }
        line_number++;
    }

    return SUCCESSFUL;
}

//load data from the data array into IAS memory
int loadData(IAS* ias, Data* data_arr) {
    for(int i = 0; i<=data_arr_index; i++)  {
        setmem(ias, data_arr[i].adr, data_arr[i].val);
    }   

    return SUCCESSFUL;
}

//load instructions form the instructions array into the IAS memory
int loadInstructions(IAS* ias, Instruction* instruction_arr) {
    /*
        this function loads instructions only in the right side of a memory word, the left side will include a NOP instruction
        , this design, though it doesn't make full use of IAS's resources, makes it simpler to write programs.
    */

    word temp_instword = (word) 0;
    word temp_opcode = (word) 0;
    word temp_address = (word) 0;
    address ins_address_counter = (address) 0;
    for(int i = 0; i<=ins_arr_index; i++) {
        temp_opcode = (word) instruction_arr[i].op;
        temp_address = (word) instruction_arr[i].adr;
        temp_opcode = temp_opcode << 12;

        temp_instword = temp_instword | temp_opcode;
        temp_instword = temp_instword | temp_address;
        setmem(ias, ins_address_counter++, temp_instword);
    }
    return SUCCESSFUL;
}

int load_program(IAS* ias, char* program_filepath) {
    //parse the program file
    int returnval = parse(program_filepath);
    if(returnval != SUCCESSFUL) {
        fprintf(stderr, "ERROR: Failed to parse you program file.\n");
        return returnval;
    }

    //load data and instructions 
    returnval = loadData(ias, data_arr);
    if(returnval != SUCCESSFUL) {
        fprintf(stderr, "ERROR: Failed to load data into IAS.\n");
        return returnval;
    }

    returnval = loadInstructions(ias, instruction_arr);
    if(returnval != SUCCESSFUL) {
        fprintf(stderr, "ERROR: Failed to load instructions into IAS.\n");
        return returnval;
    }

    return SUCCESSFUL;
}