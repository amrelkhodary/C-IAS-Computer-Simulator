#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../headers/ias.h"
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "../headers/program_loader.h"

Data* data_arr = NULL;
int data_arr_index = 0;
Instruction* instruction_arr = NULL;
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
int allocateDataArr() {
    data_arr = (Data*) malloc(data_initial_size * sizeof(Data));
    if(!data_arr) {
        perror(MEMORY_ALLOCATION_FAILED_ERROR_MESSAGE);
        return errno; 
    }

    return SUCCESSFUL;
}

//allocate memory for the instruction array which will store extracted program instructions from the inputted program text
int allocateInsArr() {
    instruction_arr = (Instruction*) malloc(instruction_initial_size * sizeof(Instruction));
    if(!instruction_arr) {
        perror(MEMORY_ALLOCATION_FAILED_ERROR_MESSAGE);
        return errno;
    }

    return SUCCESSFUL;
}

//increase the size allocated for teh data array
int increaseDataArrSize() {
    data_arr = realloc(data_arr, data_initial_size * 2 * sizeof(Data));
    if(!data_arr) {
        perror(MEMORY_ALLOCATION_FAILED_ERROR_MESSAGE);
        return errno;
    }

    data_initial_size*=2;
    return SUCCESSFUL;
}

//increase the size allocated for the instruction array
int increaseInsArrSize() {
    instruction_arr = realloc(instruction_arr, instruction_initial_size * 2 * sizeof(Instruction));
    if(!instruction_arr) {
        perror(MEMORY_ALLOCATION_FAILED_ERROR_MESSAGE);
        return errno;
    }

    instruction_initial_size*=2;
    return SUCCESSFUL;
}

//check if a line is empty
bool isempty(char* buffer) {
    for(int i = 0; i<buffersize && buffer[i] != '\n'; i++) {
        if(buffer[i] == COMMENT_CHARACTER) {break;}
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
            if(buffer[i] == 'd' && buffer[i+1] == 'a' && buffer[i+2] == 't' && buffer[i+3] == 'a' && buffer[i+4] == ':') { found_dataheader = true; i+=5; continue;}
        }

        if(found_dataheader && isspace(buffer[i]) == 0 && buffer[i] != '\0') {
            fprintf(stderr, "Syntax Error at Line %i: Found text other than \"data:\". Character found: \"%c\"\n", line_number, buffer[i]);
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
        if(i+7 < buffersize) {
            if(buffer[i] == 'p' && buffer[i+1] == 'r' && buffer[i+2] == 'o' && buffer[i+3] == 'g' 
               && buffer[i+4] == 'r' && buffer[i+5] == 'a' && buffer[i+6] == 'm' && buffer[i+7] == ':') { found_progheader = true; i+=8; continue;}
        }

        if(found_progheader && isspace(buffer[i]) == 0 && buffer[i] != 0) {
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
    memset(nowhitespace_buffer, 0, buffersize);
    int nowhitespace_buffer_index = 0;
    for(int i = 0; i<buffersize && buffer[i]; i++) {
        if(buffer[i] == COMMENT_CHARACTER) {break;}
        if(isspace(buffer[i]) == 0) {
            nowhitespace_buffer[nowhitespace_buffer_index++] = buffer[i];        
        }
    }

    //now text should be "number,number"
    for(int i = 0; i<buffersize; i++) {
        if(!(nowhitespace_buffer[i] == ',' || isdigit(nowhitespace_buffer[i]) || nowhitespace_buffer[i] == '\0')) {
            return false;
        }
    }

    return true;
}


//check if a line is a program statement
bool isprogstatement(char* buffer) {
    //remove the spaces from the line (except the one between the opcode and the address)
    char nowhitespace_buffer[buffersize];
    memset(nowhitespace_buffer, 0, buffersize);
    int nowhitespace_buffer_index = 0;
    for(int i = 0; i<buffersize && buffer[i] != '\n'; i++) {
        if(buffer[i] == COMMENT_CHARACTER) {break;}
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
        if(!((beforespace && isupper(nowhitespace_buffer[i])) || (nowhitespace_buffer[i] == ' ') || (!beforespace && isdigit(nowhitespace_buffer[i])) || nowhitespace_buffer[i] == '\0')) {
            return false;
        }
    }

    return true;
}

//extract address and value from a valid data statement
int extractData(char* datastring) {
    //extract address and value substrings
    char* substr_address = NULL;
    char* substr_value = NULL;
    for(int i = 0; i<buffersize; i++) {
        if(datastring[i] == COMMENT_CHARACTER) {datastring[i] = '\0'; break;}
        if(datastring[i] == ',') {
            datastring[i] = '\0';

            substr_address = datastring;
            substr_value = datastring+i+1;
            break;
        }
    }

    Data ndata; 
    //extract address from the address substring
    ndata.adr = extractNumber(ignoreSpaces(substr_address));

    //extract data from the data substring
    ndata.val = extractNumber_word(ignoreSpaces(substr_value));

    //create a new data struct and add it to the data array
    if(data_arr_index == data_initial_size) {
        increaseDataArrSize(data_arr);
    }
    data_arr[data_arr_index++] = ndata;
    return SUCCESSFUL;
}

//function to delete leading or following spaces from a string 
char* ignoreSpaces(char* str) {
    bool is_following = false;
    for(int i = 0; str[i] != '\0'; i++) {
        if(isspace(str[i])) {
            if(is_following) {
                str[i] = '\0';
                break;
            } else {
                str++;
            }
        }
        else {
            is_following = true;
        }
    }
    
    return str;
}

address extractNumber(char* str) {
    address number = (address) 0;
    address position = 0;

    for(int i = strlen(str) - 1; i>=0; i--) {
        if(isdigit(str[i])) {
            number += ((address) (str[i] - '0')) * pow(10, position++);
        }
    }

    return number;
}

word extractNumber_word(char* str) {
    //determine the sign
    bool isNegative = (str[0] == '-') ? true : false;

    //extract the value of the number
    word number = (word) 0;
    word position = (word) 0;
    for(int i = strlen(str) - 1; i>=0; i--) {
        if(isdigit(str[i])) {
            number += ((word) (str[i] - '0')) * pow(10, position++);
        }
    }

    //if negative, convert the number to two's complement, otherwise, return it as is
    if(isNegative) {
        return negative(number);
    }
    
    return number;
}

//extract opcode and address from a valid instruction statement
int extractInstruction(char* inststring) {
    /*
        format of a valid instruction statement
        [whitespace]<OPCODE(uppercase)>[whitespace] <address(number)>[whitespace or comment]
    */
    Instruction ninst;
    char* address_substr = NULL;
    for(int i = 0; i<buffersize; i++) {
        //skip if there is a comment
        if(inststring[i] == COMMENT_CHARACTER) {inststring[i] = '\0'; break;}

        //skip all whitespace except the one space that is between the opcode and the address
        if(isspace(inststring[i]) && !(i>=1 && isupper(inststring[i-1]) && isdigit(inststring[i+1]))) {continue;}

        //detect the space between the opcode and the address
        if(isspace(inststring[i])) {
            //initilize the address substr
            address_substr = inststring + i + 1;
            //turn the space into a null terminator character so that strcmp can be used to match against known opcodes
            inststring[i] = '\0';
            continue;
        }
    }

    //extract opcode
    if(strcmp(ignoreSpaces(inststring), "LOADMQ") == 0) {
        ninst.op = LOAD_MQ; 
    }
    else if(strcmp(ignoreSpaces(inststring), "LOADMQMX") == 0) {
        ninst.op = LOAD_MQ_MX;
    }
    else if(strcmp(ignoreSpaces(inststring), "STOR_MX") == 0) {
        ninst.op = STOR_MX;
    }
    else if(strcmp(ignoreSpaces(inststring), "LOAD_MX") == 0) {
        ninst.op = LOAD_MX;
    }
    else if(strcmp(ignoreSpaces(inststring), "LOAD_nMX") == 0) {
        ninst.op = LOAD_nMX;
    }
    else if(strcmp(ignoreSpaces(inststring), "LOAD_aMX") == 0) {
        ninst.op = LOAD_aMX;
    }
    else if(strcmp(ignoreSpaces(inststring), "LOAD_naMX") == 0) {
        ninst.op = LOAD_naMX;
    }
    else if(strcmp(ignoreSpaces(inststring), "JUMP") == 0) {
        ninst.op = JUMP_lMX;
    }
    else if(strcmp(ignoreSpaces(inststring), "CJUMP") == 0) {
        ninst.op = CJUMP_lMX;
    }
    else if(strcmp(ignoreSpaces(inststring), "ADDMX") == 0) {
        ninst.op = ADD_MX;
    }
    else if(strcmp(ignoreSpaces(inststring), "ADDAMX") == 0) {
        ninst.op = ADD_aMX;
    }
    else if(strcmp(ignoreSpaces(inststring), "SUBMX") == 0) {
        ninst.op = SUB_MX;
    }
    else if(strcmp(ignoreSpaces(inststring), "SUBAMX") == 0) {
        ninst.op = SUB_aMX;
    }
    else if(strcmp(ignoreSpaces(inststring), "MULMX") == 0) {
        ninst.op = MUL_MX;
    }
    else if(strcmp(ignoreSpaces(inststring), "DIVMX") == 0) {
        ninst.op = DIV_MX;
    }
    else if(strcmp(ignoreSpaces(inststring), "LSH") == 0) {
        ninst.op = LSH;
    }
    else if(strcmp(ignoreSpaces(inststring), "RSH") == 0) {
        ninst.op = RSH;
    }
    else if(strcmp(ignoreSpaces(inststring), "STORAMX") == 0) {
        ninst.op = STOR_lMX;
    } else {
        fprintf(stderr, "ERROR: Invalid command %s found in program file.\n", ignoreSpaces(inststring) );
        return UNRECOGNIZED_COMMAND;
    }

    //extract address
    if(address_substr) {ninst.adr = extractNumber(ignoreSpaces(address_substr));} 
    else {fprintf(stderr, "Could not extract address from instruction.\n"); return SYNTAX_ERROR;}
    //add instruction to instruction array
    if(ins_arr_index == instruction_initial_size) {
        increaseInsArrSize(instruction_arr);
    }
    instruction_arr[ins_arr_index++] = ninst;

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
    if(!program_fileptr) {
        perror(FILE_OPENING_FAILED_ERROR_MESSAGE);
        return errno;
    }

    //create the data and instruction arrays
    allocateDataArr(data_arr);
    allocateInsArr(instruction_arr);

    char buffer[buffersize];
    //initialize values inside buffer to 0
    for(int i = 0; i<buffersize; i++) {
        buffer[i] = '\0';
    }
    bool found_data_header = false;
    bool found_program_header = false;
    int line_number = 1;
    while(fgets(buffer, buffersize, program_fileptr)) {
        //skip the line if its empty
        if(isempty(buffer)) {line_number++; memset(buffer, 0, buffersize); continue;}
        
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
            fprintf(stderr, "Syntax error in your program file at line %i.\n", line_number);
            memset(buffer, 0, buffersize);
            return SYNTAX_ERROR;
        }

        memset(buffer, 0, buffersize);
        line_number++;
    }

    fclose(program_fileptr);
    return SUCCESSFUL;
}

//load data from the data array into IAS memory
int loadData(IAS* ias, Data* data_arr) {
    for(int i = 0; i<data_arr_index; i++)  {
        setmem(ias, data_arr[i].adr, data_arr[i].val);
    }   

    return SUCCESSFUL;
}

//load instructions form the instructions array into the IAS memory
int loadInstructions(IAS* ias, Instruction* instruction_arr) {
    /*
        this function loads instructions only in the right side of a memory word, the left side will include a NOP instruction
        , this design, though it doesn't make full use of IAS's resources, makes it simpler to write programs.

        0-7 8-19 20-27 28-39
                       (28 - 31)(32 - 39)
    */

    word temp_instword = (word) 0;
    word temp_opcode = (word) 0;
    word temp_address = (word) 0;
    address ins_address_counter = (address) 0;

    for(int i = 0; i<ins_arr_index; i++) {
        temp_opcode = (word) instruction_arr[i].op;
        temp_address = (word) instruction_arr[i].adr;
        temp_opcode = temp_opcode << 12;

        temp_instword = temp_instword | temp_opcode;
        temp_instword = temp_instword | temp_address;
        setmem(ias, ins_address_counter++, temp_instword);
        temp_instword = (word) 0;
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

    free(data_arr);
    free(instruction_arr);
    return SUCCESSFUL;
}

void debug_printData() {
    printf("data: \n");
    for(int i = 0; i<data_arr_index; i++) {
        printf("address: %i, value: %li\n", data_arr[i].adr, data_arr[i].val);
    }   
}

void debug_printInstructions() {
    printf("instructions: \n");
    for(int i = 0; i<ins_arr_index; i++) {
        printf("opcode: %i, address: %i\n", instruction_arr[i].op, instruction_arr[i].adr);
    }
}