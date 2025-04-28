//TODO: implement arithmetic operations

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "../headers/ias.h"

int main(int argc, char** argv) {
    /*
        program format
        ias [filename] 
    */

    char WORKING_DIRECTORY[PATH_MAX];
    char FILEPATH[PATH_MAX]; 
    const char FORWARD_SLASH[1] = "/";
    FILE* SOURCE_CODE = NULL;
    Mode mode;
    if(!getcwd(WORKING_DIRECTORY, sizeof(WORKING_DIRECTORY))) {
        perror("FATAL: Could not get the current working directory.\n");
    }

    if(argc > 2) {
        fprintf(stderr, "FATAL: Invalid arguments passed to IAS Simulator.\n");
        return FATAL_INVALID_PROGRAM_ARGUMENTS;
    } else if(argc == 2) {
        //TODO: check if the file exists
        mode = MODE_READ_FROM_SOURCE_FILE;
        if(argv[1][0] == '/') {
            //user passed an absolute path
            strcpy(FILEPATH, argv[1]);
            SOURCE_CODE = fopen(FILEPATH, 'r');
            if(!SOURCE_CODE)  {
                perror("FATAL: Failed to read source code.\n");
                return FATAL_COULD_NOT_READ_SOURCE_CODE;
            }          
        } else {
            //user passed a relative path
            strcpy(FILEPATH, WORKING_DIRECTORY);
            strcat(FILEPATH, FORWARD_SLASH);
            strcat(FILEPATH, argv[1]);
            
            SOURCE_CODE = fopen(FILEPATH, 'r');
            if(!SOURCE_CODE) {
                perror("FATAL: Failed to read source code.\n");
                return FATAL_COULD_NOT_READ_SOURCE_CODE;
            }
        }   
    } else {
        //user did not pass a file to the program, start in interactive mode
        mode = MODE_INTERACTIVE;
    }

    IAS* ias = startIAS();
    return 0;
}

IAS* startIAS() {
    IAS* ias = (IAS*) malloc(sizeof(IAS));
    PC* pc = (PC*) malloc(sizeof(PC));
    IR* ir = (IR*) malloc(sizeof(IR));
    MAR* mar = (MAR*) malloc(sizeof(MAR));
    MBR* mbr = (MBR*) malloc(sizeof(MBR));
    IBR* ibr = (IBR*) malloc(sizeof(IBR));
    AC* ac = (AC*) malloc(sizeof(AC));
    MQ* mq = (MQ*) malloc(sizeof(MQ));
    Memory* m = (Memory*) malloc(sizeof(Memory));
    m -> memory = (uint64_t*) calloc(4096, sizeof(uint64_t));
    if(!(ias && pc && ir && mar && mbr && ibr && ac && mq && m && m->memory)) {
        fprintf(stderr, "FATAL: Failed to start the IAS computer.\n");
        return FATAL_FAILED_TO_START_IAS;
    }

    pc -> register_value = 0;
    pc -> MASK = ADDRESS_MASK;
    ir -> register_value = 0;
    mar -> register_value = 0;
    mar -> MASk = ADDRESS_MASK;
    mbr -> register_value = 0;
    mbr -> MASK = WORD_MASK;
    ibr -> register_value = 0;
    ibr -> MASK = HALF_WORD_MASK;
    ac -> register_value = 0;
    ac -> MASK = WORD_MASK;
    mq -> register_value = 0;
    mq -> MASK = WORD_MASK;
    ias -> pc = pc;
    ias -> ir = ir;
    ias -> mar = mar;
    ias -> mbr = mbr;
    ias -> ibr = ibr;
    ias -> m = m; 

    return ias;
}

//check if a number is positive or negative
bool isNegative(word number) {
    number = number & SIGN_BIT_MASK;
    if(number == 0) {
        return false;
    }

    return true;
}

//get the negative of a number stored in a word
word negative(word number) {
    /* the IAS computer used 2's complement to represent signed numbers, 
       which works by:
       1- inverting all the bits of the numbers (which is called 1's complement)
       2- adding 1 to the result

        IAS number word format: 1 sign bit + 39 bits
    */

    number = number & WORD_MASK;
    
    number = ~number; //invert the bits
    number = number & WORD_MASK; //reapply mask since bit inversion would turn leftmost bits to 1
    number++;
    
    //update sign bit
    if(!isNegative(number)) {
        number = number | SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK;
    } else {
        number = number & SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK;
    }
    return number;
}



//get the absolute value of a number
word absoluteval(word number) {
    if(isNegative(number)) {
        return negative(number);
    }

    return number;
}

//transfer contents of MQ to AC
int loadmq(IAS* ias) {
    ias -> ac -> register_value = ias -> mq -> register_value;
    return SUCCESSFUL;
}

//transfer contents of memory location X to MQ
int loadmqmx(IAS* ias) {
    ias -> mbr -> register_value = ias -> m -> memory[ias -> mar -> register_value];
    ias -> mq -> register_value = ias -> mbr -> register_value;
    return SUCCESSFUL;
}

//store value in AC in memory location X
int stormx(IAS* ias) {
    ias -> mbr -> register_value = ias -> ac -> register_value;
    ias -> m -> memory[ias -> mar -> register_value] = ias -> mbr -> register_value;
    return SUCCESSFUL; 
}

//load value from memory location X to AC
int loadmx(IAS* ias) {
    ias -> mbr -> register_value = ias -> m -> memory[ias -> mar -> register_value];
    ias -> ac -> register_value = ias -> mbr -> register_value;
    return SUCCESSFUL;
}

//load negative value from memory location X to AC
int loadnmx(IAS* ias) {
    ias -> mbr -> register_value = negative(ias -> m -> memory[ias -> mar -> register_value]);
    ias -> ac -> register_value = ias -> mbr -> register_value;
    return SUCCESSFUL;
}

//load absolute value from memory location X to AC
int loadamx(IAS* ias) {
    ias -> mbr -> register_value = absoluteval(ias -> m -> memory[ias -> mar -> register_value]);
    ias -> ac -> register_value = ias -> mbr -> register_value;
    return SUCCESSFUL;
}

//load negative the absolute value from memory location X to AC
int loadnamx(IAS* ias) {
    ias -> mbr -> register_value = negative(absoluteval(ias -> m -> memory[ias -> mar -> register_value]));
    ias -> ac ->register_value = ias -> mbr -> register_value;
    return SUCCESSFUL;
}

//take instruction from the left half of memory location X
int jumplmx(IAS* ias) {
    ias -> mbr -> register_value = ias -> m -> memory[ias -> mar -> register_value];
    ias -> pc -> register_value = ias -> mbr -> register_value & LEFT_ADDRESS_WORD_MASK;
    return SUCCESSFUL;
}

//take instruction from the right half of memory location X
int jumprmx(IAS* ias) {
    ias -> mbr -> register_value = ias -> m -> memory[ias -> mar -> register_value];
    ias -> pc -> register_value = ias -> mbr -> register_value & RIGHT_ADDRESS_WORD_MASK;
    return SUCCESSFUL;
}

//if AC is nonnegative, take instruction from left half of memory location X 
int cjumplmx(IAS* ias) {
    if(isNegative(ias -> ac -> register_value)) {
        return SUCCESSFUL;
    }

    ias -> mbr -> register_value = ias -> m -> memory[ias -> mar -> register_value];
    ias -> pc -> register_value = ias -> mbr -> register_value & LEFT_ADDRESS_WORD_MASK;
    return SUCCESSFUL;
}

//if AC is nonnegative, take instruction from right half of memory location X
int cjumprmx(IAS* ias) {
    if(isNegative(ias -> ac -> register_value)) {
        return SUCCESSFUL;
    }

    ias -> mbr -> register_value = ias -> m -> memory[ias -> mar -> register_value];
    ias -> pc -> register_value = ias -> mbr -> register_value & RIGHT_ADDRESS_WORD_MASK;
    return SUCCESSFUL; 
}
