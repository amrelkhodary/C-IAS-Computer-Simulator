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
    m -> memory = (uint64_t*) calloc(4096, sizeof(uint64_t));
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

//take instruction from the left half of memory location X
int jumplmx(IAS* ias) {

    return 0;
}

//take instruction from the right half of memory location X
int jumprmx(IAS* ias) {

    return 0;
}

//if AC is nonnegative, take instruction from left half of memory location X 
int cjumplmx(IAS* ias) {

    return 0;
}

//if AC is nonnegative, take instruction from right half of memory location X
int cjumprmx(IAS* ias) {
    return 0;
}

//add value in memory location X to AC
int addmx(IAS* ias) {
    ias -> mbr -> register_value = ias -> m -> memory[ias -> mar -> register_value];
    if((ias -> mbr -> register_value & NUMBER_VALUE_MASK) + (ias -> ac -> register_value & NUMBER_VALUE_MASK) > MAX_INTEGER) {
        return INTEGER_OVERFLOW;
    }

    bool isnegative_mbr = isNegative(ias -> mbr -> register_value);
    bool isnegative_ac = isNegative(ias -> ac -> register_value);
    bool isbigger_mbr = ((ias -> mbr -> register_value & NUMBER_VALUE_MASK) >= (ias -> ac -> register_value & NUMBER_VALUE_MASK)) ? true : false;

    ias -> ac -> register_value = (ias -> mbr -> register_value & NUMBER_VALUE_MASK) + (ias -> ac -> register_value & NUMBER_VALUE_MASK);

    if((isnegative_ac && isnegative_mbr) ||
       (isbigger_mbr && isnegative_mbr)  ||
       (!isbigger_mbr && isnegative_ac)) {
        //sum will be negative
        ias -> ac -> register_value = ias -> ac -> register_value | SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK;
    } else {
        //sum will be positive
        ias -> ac -> register_value = ias -> ac -> register_value & SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK;
    }

    return SUCCESSFUL;
}

//add absolute the value of a memory location X to AC
int addamx(IAS* ias) {
    ias -> mbr -> register_value = absoluteval(ias -> m -> memory[ias -> mar -> register_value]);
    if((ias -> mbr -> register_value & NUMBER_VALUE_MASK) + (ias -> ac -> register_value & NUMBER_VALUE_MASK) > MAX_INTEGER) {
        return INTEGER_OVERFLOW;
    }

    bool isnegative_ac = isNegative(ias -> ac -> register_value);
    bool isbigger_ac = ((ias -> ac -> register_value & NUMBER_VALUE_MASK) >= (ias -> mbr -> register_value & NUMBER_VALUE_MASK)) ? true : false;

    ias -> ac -> register_value = (ias -> ac -> register_value & NUMBER_VALUE_MASK) + (ias -> mbr -> register_value & NUMBER_VALUE_MASK);

    if(!isnegative_ac || (isnegative_ac && !isbigger_ac)) {
        //number will be positive
        ias -> ac -> register_value = ias -> ac -> register_value & SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK;
    } else {
        //number will be negative
        ias -> ac -> register_value = ias -> ac -> register_value | SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK;
    }

    return SUCCESSFUL;
}

//subtract value in memory location X from AC
int submx(IAS* ias) {

    return 0;
}

//add absolute the value of a memory location X to AC
int subamx(IAS* ias) {
    return 0;
}

//multiply value from memory location X by AC, store most significant bits in AC, least significat in MQ
int mulmx(IAS* ias) {
    //TODO: dumb implementation, should be replaced in the future
    ias -> mbr -> register_value = ias -> m -> memory[ias -> mar -> register_value];
    if((absoluteval(ias -> ac -> register_value) & NUMBER_VALUE_MASK) * (absoluteval(ias -> mbr -> register_value) & NUMBER_VALUE_MASK) > MAX_INTEGER) {
        return INTEGER_OVERFLOW;
    }

    bool isnegative_ac = isNegative(ias -> ac -> register_value);
    bool isnegative_mbr = isNegative(ias -> mbr -> register_value);

    ias -> ac -> register_value = ((absoluteval(ias -> ac -> register_value) & NUMBER_VALUE_MASK) * (absoluteval(ias -> mbr -> register_value) & NUMBER_VALUE_MASK) & NUMBER_VALUE_MASK);

    if((isnegative_ac && isnegative_mbr) || (!isnegative_ac && !isnegative_mbr)) {
        //positive
        ias -> ac -> register_value = ias -> ac -> register_value & SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK;
    } else {
       ias -> ac -> register_value = ias -> ac -> register_value | SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK;
    }

    return SUCCESSFUL;
}

//divide AC by value from memory location X, put the the quotient in MQ, the remainder in AC
int divmx(IAS* ias) {
    //TODO: Also dumb implementation, should also be replaced in the future
    ias -> mbr -> register_value = ias -> m -> memory[ias -> mar -> register_value];

    bool isnegative_ac = isNegative(ias -> ac -> register_value);
    bool isnegative_mbr = isNegative(ias -> mbr -> register_value);

    ias -> ac -> register_value = ((absoluteval(ias -> ac -> register_value) & NUMBER_VALUE_MASK) / (absoluteval(ias -> mbr -> register_value) & NUMBER_VALUE_MASK) & NUMBER_VALUE_MASK);
    ias -> mq -> register_value = ((absoluteval(ias -> ac -> register_value) & NUMBER_VALUE_MASK) % (absoluteval(ias -> mbr -> register_value) & NUMBER_VALUE_MASK) & NUMBER_VALUE_MASK);

    if((isnegative_ac && isnegative_mbr) || (!isnegative_ac && !isnegative_mbr)) {
        //positive
        ias -> ac -> register_value = ias -> ac -> register_value & SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK;
        ias -> mq -> register_value = ias -> mq -> register_value & SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK;
    } else {
       //negative
       ias -> ac -> register_value = ias -> ac -> register_value | SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK;
       ias -> mq -> register_value = ias -> mq -> register_value | SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK;
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