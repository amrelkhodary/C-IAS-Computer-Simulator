//TODO: Fix bugs in arithemtic instructions especially when dealing with negative values
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "../headers/ias.h"

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
    m -> memory = (uint64_t*) calloc(IAS_MEMORY_WORD_COUNT, sizeof(uint64_t));
    if(!(ias && pc && ir && mar && mbr && ibr && ac && mq && m && m->memory)) {
        fprintf(stderr, "FATAL: Failed to start the IAS computer.\n");
        return NULL;
    }

    pc -> register_value = (address)0;
    pc -> MASK = ADDRESS_MASK;
    ir -> register_value = (opcode)0;
    mar -> register_value = (address)0;
    mar -> MASk = ADDRESS_MASK;
    mbr -> register_value = (word)0;
    mbr -> MASK = WORD_MASK;
    ibr -> register_value = (opcode)0;
    ibr -> MASK = HALF_WORD_MASK;
    ac -> register_value = (word)0;
    ac -> MASK = WORD_MASK;
    mq -> register_value = (word)0;
    mq -> MASK = WORD_MASK;
    ias -> ac = ac;
    ias -> mq = mq;
    ias -> pc = pc;
    ias -> ir = ir;
    ias -> mar = mar;
    ias -> mbr = mbr;
    ias -> ibr = ibr;
    ias -> m = m; 

    return ias;
}

//this functions runs the program that was loaded to memory
int run(IAS* ias) {
    int exec_return_val = -1;
    while(exec_return_val != UNRECOGNIZED_COMMAND) {
        fetch(ias);
        exec_return_val = execute(ias);
    }

    return SUCCESSFUL;
}

/*
    the purpose of the fetch cycle in IAS is to 
    1- put the next instruction in IR
    2- put the address the next instruction will operate on in MAR
*/
int fetch(IAS* ias) {
    /*
        0-7 8-19 20-27 28-39
    */
    //check if an instruction already exists in IBR
    if((ias -> ibr -> register_value & HALF_WORD_INSTRUCTION_MASK) >> 12 != NOP) {
        //load the instruction in IR
        ias -> ir -> register_value = (opcode) ((ias -> ibr -> register_value & HALF_WORD_INSTRUCTION_MASK) >> 12);

        //load the address in MAR
        ias -> mar -> register_value = (address) (ias -> ibr -> register_value & HALF_WORD_ADDRESS_MASK); 
        
        //update the program counter
        ias -> pc -> register_value = ias -> pc -> register_value + (address) 1;
    }
    else {
        //load the memory word containing the left and right instructions from memory
        ias -> mbr -> register_value = ias -> m -> memory[ias -> pc -> register_value];

        //decide whether the left instruction is necesssary
        if((ias -> mbr -> register_value & LEFT_INSTRUCTION_WORD_MASK) >> 32 != NOP) {
            //store right half word in IBR
            ias -> ibr -> register_value = (half_word) (ias -> mbr -> register_value & RIGHT_HALF_WORD_MASK);

            //load the instruction in the left half in IR
            ias -> ir -> register_value = (opcode) ((ias -> mbr -> register_value & LEFT_INSTRUCTION_WORD_MASK) >> 32);

            //load the address in the left half in MAR
            ias -> mar -> register_value = (address) ((ias -> mbr -> register_value & LEFT_ADDRESS_WORD_MASK) >> 20);
        }
        else {
            //load the instruction in the right half in IR
            ias -> ir -> register_value = (opcode) ((ias -> mbr -> register_value & RIGHT_INSTRUCTION_WORD_MASK) >> 12);

            //load the address in the right half in MAR
            ias -> mar -> register_value = (address) ((ias -> mbr -> register_value & RIGHT_ADDRESS_WORD_MASK));

            //update the program counter
            ias -> pc -> register_value = ias -> pc -> register_value + (address) 1;
        }
    }
    return SUCCESSFUL;
}

//execute a program instruction
int execute(IAS* ias) {
    int RETURN_VALUE = 0;
    if(ias -> ir -> register_value == LOAD_MQ) {
        RETURN_VALUE = loadmq(ias);
    }
    else if(ias -> ir -> register_value == LOAD_MQ_MX) {
        RETURN_VALUE = loadmqmx(ias);
    }
    else if(ias -> ir -> register_value == STOR_MX) {
        RETURN_VALUE = stormx(ias);
    }
    else if(ias -> ir -> register_value == LOAD_MX) {
        RETURN_VALUE = loadmx(ias);
    }
    else if(ias -> ir -> register_value == LOAD_nMX) {
        RETURN_VALUE = loadnmx(ias);
    }
    else if(ias -> ir -> register_value == LOAD_naMX) {
        RETURN_VALUE = loadnamx(ias);
    }
    else if(ias -> ir -> register_value == JUMP_lMX) {
        RETURN_VALUE = jumplmx(ias);
    }
    else if(ias -> ir -> register_value == JUMP_rMX) {
        RETURN_VALUE = jumprmx(ias);
    }
    else if(ias -> ir -> register_value == CJUMP_lMX) {
        RETURN_VALUE = cjumplmx(ias);
    }
    else if(ias -> ir ->register_value == CJUMP_rMX) {
        RETURN_VALUE = cjumprmx(ias);
    }
    else if(ias -> ir -> register_value == ADD_MX) {
        RETURN_VALUE = addmx(ias);
    }
    else if(ias -> ir -> register_value == ADD_aMX) {
        RETURN_VALUE = addamx(ias);
    }
    else if(ias -> ir -> register_value == SUB_MX) {
        RETURN_VALUE = submx(ias);
    }
    else if(ias -> ir -> register_value == SUB_aMX) {
        RETURN_VALUE = subamx(ias);
    }
    else if(ias -> ir -> register_value == MUL_MX) {
        RETURN_VALUE = mulmx(ias);
    }
    else if(ias -> ir -> register_value == DIV_MX) {
        RETURN_VALUE = divmx(ias);
    }
    else if(ias -> ir -> register_value == LSH) {
        RETURN_VALUE = lsh(ias);
    }
    else if(ias -> ir -> register_value == RSH) {
        RETURN_VALUE = rsh(ias);
    }
    else if(ias -> ir -> register_value == STOR_lMX) {
        RETURN_VALUE = storlmx(ias);
    }
    else if(ias -> ir -> register_value == STOR_rMX) {
        RETURN_VALUE = storrmx(ias);
    }
    else {
        return UNRECOGNIZED_COMMAND;
    }
    return RETURN_VALUE;
}

//check if a number is positive or negative
bool isNegative(word number) {
    number = number & SIGN_BIT_MASK;
    if(number == (word)0) {
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
    bool is_negative = isNegative(number);

    number = (word) ~number; //invert the bits
    number = (word) (number + (word) 1); //add 1
    number = (word) number & WORD_MASK; //reapply word mask

    //update sign bit
    if(!is_negative) {
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

int setmem(IAS* ias, address adr, word value) {
    ias -> m -> memory[adr] = value;
    return SUCCESSFUL;
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

/*
    should set up the registers in such that the next instruction to be exectued
    is the left instruction in a memory word x
    in the case of jumplmx, all that needs to be done is to modify the value of the pc register and to clear the IBR
*/
int jumplmx(IAS* ias) {
    //modify the value of the PC register to point to the memory word x
    ias -> pc -> register_value = ias -> mar -> register_value;

    //clear IBR
    ias -> ibr -> register_value = (half_word) 0;

    return SUCCESSFUL;
}

/*
    should set up the registers in such a way that the next instruction to be executed is
    the right instruction of a memory word x
    in the case of jumprmx, what needs to be done is for the value of PC and IBR to be modified
*/
int jumprmx(IAS* ias) {
    //modify the value of the PC register to point to the memory word x
    ias -> pc -> register_value = ias -> mar -> register_value;

    //modify the value of the IBR register to contain the right instruction of in the memory word
    ias -> ibr -> register_value = (ias -> m -> memory[ias -> mar -> register_value] & RIGHT_HALF_WORD_MASK);

    return 0;
}

//if AC is nonnegative, take instruction from left half of memory location X 
int cjumplmx(IAS* ias) {
    if(ias -> ac -> register_value > (word) 0) {
        jumplmx(ias);
    }
    return 0;
}

//if AC is nonnegative, take instruction from right half of memory location X
int cjumprmx(IAS* ias) {
    if(ias -> ac -> register_value > (word) 0) {
        jumprmx(ias);
    }
    return 0;
}

//add value in memory location X to AC
int addmx(IAS* ias) {
    //store the number to add to AC in MBR
    ias -> mbr -> register_value = ias -> m -> memory[ias -> mar -> register_value];

    //keep track of the sizes and negativity of the values in AC and MBR to determine sign bit 
    bool mbr_isnegative = isNegative(ias -> mbr -> register_value);
    bool ac_isnegative = isNegative(ias -> ac -> register_value);
    bool ac_isbigger = (absoluteval(ias -> ac -> register_value) >= absoluteval(ias -> mbr -> register_value)) ? true : false;

    //add the two numbers (we don't have to do any conversions since two's complement representation handles addition)
    ias -> ac -> register_value = (ias -> ac -> register_value & NUMBER_VALUE_MASK) + (ias -> mbr -> register_value & NUMBER_VALUE_MASK);

    //determine sign bit
    if((ac_isbigger && !ac_isnegative) || (!ac_isbigger && !mbr_isnegative)) {
        //result will be positive
        ias -> ac -> register_value = ias -> ac -> register_value & SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK;
    } else {
        //result will be negative
        ias -> ac -> register_value = ias -> ac -> register_value | SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK;
    }
    return SUCCESSFUL;
}

//add absolute the value of a memory location X to AC
int addamx(IAS* ias) {
    //store the number to add to AC in MBR
    ias -> mbr -> register_value = absoluteval(ias -> m -> memory[ias -> mar -> register_value]);

    //keep track of the sizes and negativity of the values in AC and MBR to determine sign bit later
    bool mbr_isnegative = isNegative(ias -> mbr -> register_value);
    bool ac_isnegative = isNegative(ias -> ac -> register_value);
    bool ac_isbigger = (absoluteval(ias -> ac -> register_value) >= absoluteval(ias -> mbr -> register_value)) ? true : false;

    //add the two numbers (we don't have to do any conversions since two's complement representation handles addition)
    ias -> ac -> register_value = (ias -> ac -> register_value & NUMBER_VALUE_MASK) + (ias -> mbr -> register_value & NUMBER_VALUE_MASK);

    //determine sign bit
    if((ac_isbigger && !ac_isnegative) || (!ac_isbigger && !mbr_isnegative)) {
        //result will be positive
        ias -> ac -> register_value = ias -> ac -> register_value & SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK;
    } else {
        //result will be negative
        ias -> ac -> register_value = ias -> ac -> register_value | SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK;
    }
    return SUCCESSFUL;
}

//subtract value in memory location X from AC
int submx(IAS* ias) {
    /*
        how to subtract numbers in two's complement
        A - B
       =A + (-B)
    */

    //get the number to subtract from AC from memory and store it in MBR
    ias -> mbr -> register_value = ias -> m -> memory[ias -> mar -> register_value];

    //remember whether the value was negative or positive to maintain sign bit
    bool mbr_isnegative = isNegative(ias -> mbr -> register_value);
    //convert the number to negative using two's complement
    ias -> mbr -> register_value = negative(ias -> mbr -> register_value);

    //keep track of who's bigger and who's negative to maintain sign bit
    bool ac_isbigger = (absoluteval(ias -> ac -> register_value) >= absoluteval(ias -> mbr -> register_value)) ? true : false;
    bool ac_isnegative = isNegative(ias -> ac -> register_value);
    mbr_isnegative = !mbr_isnegative;

    //add the values in mbr and ac
    ias -> ac -> register_value = (ias -> ac -> register_value & NUMBER_VALUE_MASK) + (ias -> mbr -> register_value & NUMBER_VALUE_MASK);

    if((ac_isbigger && !ac_isnegative) || (!ac_isbigger && !mbr_isnegative)) {
        //result will be positive
        ias -> ac -> register_value = ias -> ac -> register_value & SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK;
    } else {
        //result will be negative
        ias -> ac -> register_value = ias -> ac -> register_value | SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK;
    }

    return SUCCESSFUL;
}

//add absolute the value of a memory location X to AC
int subamx(IAS* ias) {
    /*
        how to subtract numbers in two's complement
        A - B
       =A + (-B)
    */

    //get the number to subtract from AC from memory and store it in MBR
    ias -> mbr -> register_value = absoluteval(ias -> m -> memory[ias -> mar -> register_value]);

    //remember whether the value was negative or positive to maintain sign bit
    bool mbr_isnegative = isNegative(ias -> mbr -> register_value);
    //convert the number to negative using two's complement
    ias -> mbr -> register_value = negative(ias -> mbr -> register_value);

    //keep track of who's bigger and who's negative to maintain sign bit
    bool ac_isbigger = (absoluteval(ias -> ac -> register_value) >= absoluteval(ias -> mbr -> register_value)) ? true : false;
    bool ac_isnegative = isNegative(ias -> ac -> register_value);
    mbr_isnegative = !mbr_isnegative;

    //add the values in mbr and ac
    ias -> ac -> register_value = (ias -> ac -> register_value & NUMBER_VALUE_MASK) + (ias -> mbr -> register_value & NUMBER_VALUE_MASK);

    if((ac_isbigger && !ac_isnegative) || (!ac_isbigger && !mbr_isnegative)) {
        //result will be positive
        ias -> ac -> register_value = ias -> ac -> register_value & SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK;
    } else {
        //result will be negative
        ias -> ac -> register_value = ias -> ac -> register_value | SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK;
    }

    return SUCCESSFUL;
}

//multiply value from memory location X by AC, store most significant bits in AC, least significat in MQ
int mulmx(IAS* ias) {
    /*
        WARNING: This is the not the way the IAS computer actually implemented multiplication
    */

    //get value to be multiplied by AC from memory and store it in MBR
    ias -> mbr -> register_value = ias -> m -> memory[ias -> mar -> register_value];

    //keep track of the negativity of the values in AC and MBR to maintain the sign bit later
    bool isnegative_ac = isNegative(ias -> ac -> register_value);
    bool isnegative_mbr = isNegative(ias -> mbr -> register_value);

    //make sure the multiplication would NOT result in overflow
    if((absoluteval(ias -> ac -> register_value) & NUMBER_VALUE_MASK) * (absoluteval(ias -> mbr -> register_value) & NUMBER_VALUE_MASK) > MAX_INTEGER) {
        return INTEGER_OVERFLOW;
    }

    //multiply the values 
    ias -> ac -> register_value = (absoluteval(ias -> ac -> register_value) & NUMBER_VALUE_MASK) * (absoluteval(ias -> mbr -> register_value) & NUMBER_VALUE_MASK);

    //determine sign bit
    if((isnegative_ac && isnegative_mbr) || (!isnegative_ac && !isnegative_mbr)) {
        //result will be positive
        ias -> ac -> register_value = ias -> ac -> register_value & SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK;
    } else {
        //result will be negative
        ias -> ac -> register_value = negative(ias -> ac -> register_value);
        ias -> ac -> register_value = ias -> ac -> register_value | SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK;
    }

    return SUCCESSFUL;
}

//divide AC by value from memory location X, put the the quotient in MQ, the remainder in AC
int divmx(IAS* ias) {
     //get value to be multiplied by AC from memory and store it in MBR
    ias -> mbr -> register_value = ias -> m -> memory[ias -> mar -> register_value];

    //keep track of the negativity of the values in AC and MBR to maintain the sign bit later
    bool isnegative_ac = isNegative(ias -> ac -> register_value);
    bool isnegative_mbr = isNegative(ias -> mbr -> register_value);

    //divide, store result in AC, remainder in MQ
    ias -> mq -> register_value = (absoluteval(ias -> mq -> register_value) & NUMBER_VALUE_MASK) % (absoluteval(ias -> mbr -> register_value) & NUMBER_VALUE_MASK);
    ias -> ac -> register_value = (absoluteval(ias -> ac -> register_value) & NUMBER_VALUE_MASK) / (absoluteval(ias -> mbr -> register_value) & NUMBER_VALUE_MASK);

    //determine sign bit
    if((isnegative_ac && isnegative_mbr) || (!isnegative_ac && !isnegative_mbr)) {
        //result will be positive
        ias -> ac -> register_value = ias -> ac -> register_value & SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK;
    } else {
        //result will be negative
        ias -> ac -> register_value = negative(ias -> ac -> register_value);
        ias -> ac -> register_value = ias -> ac -> register_value | SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK;
    }

    return SUCCESSFUL;
}

//right shift AC by one bit (i.e. divide by 2)
int rsh(IAS* ias) {
    if((ias -> ac -> register_value & NUMBER_VALUE_MASK) >> 1 > MAX_INTEGER) {
        return INTEGER_OVERFLOW;
    }

    bool isnegative = isNegative(ias -> ac -> register_value);

    ias -> ac -> register_value = (ias -> ac -> register_value & NUMBER_VALUE_MASK) >> 1;

    if(isnegative) {
        ias -> ac -> register_value = ias -> ac -> register_value | SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK;
    } else {
        ias -> ac -> register_value = ias -> ac -> register_value & SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK;
    }

    return SUCCESSFUL;
}

//left shift AC by one bit (i.e. multiply by 2) 
int lsh(IAS* ias) {
    if((ias -> ac -> register_value & NUMBER_VALUE_MASK) >> 1 > MAX_INTEGER) {
        return INTEGER_OVERFLOW;
    }

    bool isnegative = isNegative(ias -> ac -> register_value);

    ias -> ac -> register_value = (ias -> ac -> register_value & NUMBER_VALUE_MASK) << 1;

    if(isnegative) {
        ias -> ac -> register_value = ias -> ac -> register_value | SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK;
    } else {
        ias -> ac -> register_value = ias -> ac -> register_value & SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK;
    }

    return SUCCESSFUL;
}

//replace left address by 12 rightmost bits in AC
int storlmx(IAS* ias) {
    ias -> ac -> register_value = ias -> ac -> register_value & AC_ADDRESS_MASK;
    ias -> ac -> register_value = ias -> ac -> register_value << 20;
    ias -> mbr -> register_value = ias -> m -> memory[ias -> mar -> register_value];
    ias -> mq -> register_value = ias -> mbr -> register_value;
    ias -> mq -> register_value = ias -> mq -> register_value & (~LEFT_ADDRESS_WORD_MASK);
    ias -> ac -> register_value = ias -> ac -> register_value | ias -> mq -> register_value;
    ias -> m -> memory[ias -> mar -> register_value] = ias -> ac -> register_value;
    return SUCCESSFUL; 
}

//replace right address by 12 rightmost bits in AC
int storrmx(IAS* ias) {
    ias -> ac -> register_value = ias -> ac -> register_value & AC_ADDRESS_MASK;
    ias -> mbr -> register_value = ias -> m -> memory[ias -> mar -> register_value];
    ias -> mq -> register_value = ias -> mbr -> register_value;
    ias -> mq -> register_value = ias -> mq -> register_value & (~RIGHT_ADDRESS_WORD_MASK);
    ias -> ac -> register_value = ias -> ac -> register_value | ias -> mq -> register_value;
    ias -> m -> memory[ias -> mar -> register_value] = ias -> ac -> register_value;
    return SUCCESSFUL; 
}

/*
     0-7 8-19 20-27 28-39
*/