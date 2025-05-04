#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "../headers/ias.h"
#include "../headers/tests.h"
int runtests() {
    IAS* ias = startIAS();
    setmem(ias, (address)0, (word)10);
    setmem(ias, (address)1, (word)20);
    setmem(ias, (address)2, (word)30);

    int testsval = test_isNegative(ias) | test_negative(ias) | test_loadmq(ias) | test_loadmqmx(ias) | test_stormx(ias)
                 | test_loadmx(ias) | test_loadamx(ias) | test_loadnamx(ias) | test_jumplmx(ias) | test_jumprmx(ias)
                 | test_cjumplmx(ias) | test_cjumprmx(ias) | test_arithmetic(ias) | test_storlmx(ias) | test_storrmx(ias);
    if(testsval == 0) {
       printf("tests succeeded\n");
        
    } else {
        printf("tests failed\n");
    }

    return testsval;
}

int test_loadmqmx(IAS* ias) {
    ias -> mar -> register_value = (address)0;
    loadmqmx(ias);
    if(ias -> mq -> register_value == (word)10) {
        return TEST_SUCCESSFUL;
    }

    printf("loadmqmx failed\n");
    return TEST_FAILED;
}

int test_loadmq(IAS* ias) {
    word current_mq_value = ias -> mq -> register_value;
    loadmq(ias);
    if(ias -> ac -> register_value == current_mq_value) {
        return TEST_SUCCESSFUL;
    }

    printf("loadmq failed\n");
    return TEST_FAILED;
}
int test_stormx(IAS* ias) {
    ias -> mar -> register_value = (address)20;
    int current_ac_value = ias -> ac -> register_value;
    stormx(ias);
    if(ias -> m -> memory[20] == current_ac_value)  {
        return TEST_SUCCESSFUL;
    }

    printf("stormx failed\n");
    return TEST_FAILED;
}
int test_loadmx(IAS* ias) {
    ias -> mar -> register_value = (word)2;
    loadmx(ias);
    if(ias -> ac -> register_value == ias -> m -> memory[ias -> mar -> register_value]) {
        return TEST_SUCCESSFUL;
    }

    printf("loadmx failed\n");
    return TEST_FAILED;
}
int test_isNegative(IAS* ias) {
    word positive_number = (word)0b0000000000000000000000000000000000000000000000000000000000000011;
    word negative_number = (word)0b0000000000000000000000001000000000000000000000000000000000000011; 
    if(!isNegative(positive_number) && isNegative(negative_number)) {
        return TEST_SUCCESSFUL;
    }

    printf("isNegative failed\n");
    return TEST_FAILED;
}

int test_negative(IAS* ias) {
    word positive_number = (word)0b0000000000000000000000000000000000000000000000000000000000000011;
    word negative_number = (word)0b0000000000000000000000001000000000000000000000000000000000000011; 

    if(isNegative(negative(positive_number)) && !isNegative(negative(negative_number))) {
        return TEST_SUCCESSFUL;
    }

    printf("negative failed\n");
    return TEST_FAILED;
}
int test_loadamx(IAS* ias) {
     word positive_number = (word)0b0000000000000000000000000000000000000000000000000000000000000011;
    word negative_number = (word)0b0000000000000000000000001000000000000000000000000000000000000011; 

    if(!isNegative(absoluteval(positive_number)) && !isNegative(absoluteval(negative_number))) {
        return TEST_SUCCESSFUL;
    }

    printf("loadamx failed\n");
    return TEST_FAILED;
}

int test_loadnamx(IAS* ias) {
    word positive_number = (word)0b0000000000000000000000000000000000000000000000000000000000000011;
    word negative_number = (word)0b0000000000000000000000001000000000000000000000000000000000000011; 

    if(isNegative(negative(absoluteval(positive_number))) && negative(isNegative(absoluteval(negative_number)))) {
        return TEST_SUCCESSFUL;
    }

    printf("loadnamx failed\n");
    return TEST_FAILED;
}

int test_jumplmx(IAS* ias) {
    word memory_instruction_word = (word) 0b0000000000000000000000000000010100000000000000000110000000000000;
    ias -> mar -> register_value = (address) 45;
    ias -> m -> memory[ias -> mar -> register_value] = memory_instruction_word;
    jumplmx(ias);

    if(ias -> pc -> register_value == (opcode) 0b00000101) {
        return TEST_SUCCESSFUL;
    }

    printf("jumplmx failed\n");
    return TEST_FAILED;
}

int test_jumprmx(IAS* ias) {
    word memory_instruction_word = (word) 0b0000000000000000000000000000010100000000000000000110000000000000;
    ias -> mar -> register_value = (address) 45;
    ias -> m -> memory[ias -> mar -> register_value] = memory_instruction_word;
    jumplmx(ias);

    if(ias -> pc -> register_value == (opcode) 0b00000110) {
        return TEST_SUCCESSFUL;
    }


    printf("jumprmx failed\n");
    return TEST_FAILED;
}

int test_cjumplmx(IAS* ias) {
    word memory_instruction_word = (word) 0b0000000000000000000000000000010100000000000000000110000000000000;
    ias -> mar -> register_value = (address) 45;
    ias -> m -> memory[ias -> mar -> register_value] = memory_instruction_word;
    ias -> ac -> register_value = (word)1; //random nonnegative value
    cjumplmx(ias);
    if(ias -> pc -> register_value != (opcode) 0b00000101) {

    printf("cjumplmx failed\n");
        return TEST_FAILED;
    }
    ias -> pc -> register_value = (address) 0;
    ias -> ac -> register_value = (word) negative(1);
    cjumplmx(ias);
    if(ias -> pc -> register_value != (opcode) 0b00000101) {

    printf("cjumplmx failed\n");
        return TEST_FAILED;
    }

    return TEST_SUCCESSFUL;
}

int test_cjumprmx(IAS* ias) {
     word memory_instruction_word = (word) 0b0000000000000000000000000000010100000000000000000110000000000000;
    ias -> mar -> register_value = (address) 45;
    ias -> m -> memory[ias -> mar -> register_value] = memory_instruction_word;
    ias -> ac -> register_value = (word)1; //random nonnegative value
    cjumplmx(ias);
    if(ias -> pc -> register_value != (opcode) 0b00000110) {
    printf("cjumprmx failed\n");
        return TEST_FAILED;
    }
    ias -> pc -> register_value = (address) 0;
    ias -> ac -> register_value = (word) negative(1);
    cjumplmx(ias);
    if(ias -> pc -> register_value != (opcode) 0b00000110) {

    printf("cjumprmx failed\n");
        return TEST_FAILED;
    }

    return TEST_SUCCESSFUL;     
}

int test_arithmetic(IAS* ias) {
    ias -> mar -> register_value = (address) 100;
    ias -> m -> memory[ias -> mar -> register_value] = (word) 10;

    //test addmx
    ias -> ac -> register_value = 0;
    addmx(ias); //ac = 10
    if(ias -> ac -> register_value != (word) 10) {
        printf("1: addmx failed, expected 10, found %li\n", ias -> ac -> register_value);
        return TEST_FAILED;
    }
    ias -> m -> memory[ias -> mar -> register_value] = negative((word) 10);
    ias -> ac -> register_value = 0;
    addmx(ias); //ac = -10
    addmx(ias); //ac = -20
    if(ias -> ac -> register_value != negative((word) 20)) {
        printf("2: addmx failed, expected -20, found -%li\n", absoluteval(ias -> ac -> register_value));
        return TEST_FAILED;
    }
    //test submx
    ias -> ac -> register_value = 0;
    ias -> m -> memory[ias -> mar -> register_value] = 10;
    submx(ias); //ac = -10
    if(ias -> ac -> register_value != negative((word) 10)) {
       printf("3: submx failed, expected -10, found -%li\n", absoluteval(ias -> ac -> register_value));
       return TEST_FAILED; 
    }
    ias -> ac -> register_value = 0;
    ias -> m -> memory[ias -> mar -> register_value] = negative((word) 10);
    submx(ias); //ac = 10
    if(ias -> ac -> register_value != (word) 10) {
        printf("4: submx failed, expected 10, found -%li\n", absoluteval(ias -> ac -> register_value));
        return TEST_FAILED;
    }
    //test mulmx
    ias -> ac -> register_value = 0;
    ias -> m -> memory[ias -> mar -> register_value] = (word) 10;
    addmx(ias); //ac = 10
    mulmx(ias); //ac = 100
    if(ias -> ac -> register_value != (word) 100) {
        printf("5: mulmx failed, expected 100, found %li\n", ias -> ac -> register_value);
        return TEST_FAILED;
    }
    ias -> ac -> register_value = 0;
    ias -> m -> memory[ias -> mar -> register_value] = negative((word)10);
    addmx(ias); //ac = -10
    mulmx(ias); //ac = 100
    mulmx(ias); //ac -1000
    if(ias -> ac -> register_value != negative((word) 1000)) {
        printf("6: mulmx failed, expected -1000, found -%li\n", absoluteval(ias -> ac -> register_value));
        return TEST_FAILED;
    }
    //test divmx
    ias -> ac -> register_value = 4;
    ias -> m -> memory[ias -> mar -> register_value] = (word) 2;
    divmx(ias); //ac = 2
    if(ias -> ac -> register_value != (word) 2) {
        printf("7: divmx failed, expected 2, found %li\n", ias -> ac -> register_value);
        return TEST_FAILED;
    }
    ias -> ac -> register_value = 4;
    ias -> m -> memory[ias -> mar -> register_value] = negative((word) 2);
    divmx(ias); //ac = -2
    if(ias -> ac -> register_value != negative((word) 2))  {
        printf("8: divmx failed, expected -2, found -%li\n", ias -> ac -> register_value);
    }
    return TEST_SUCCESSFUL;
}

int test_storlmx(IAS* ias) {
   ias -> ac -> register_value = (word) 0b0000000000000000000000000000000000000000000000000000000000000100;
   ias -> mar -> register_value = (address) 30;
   ias -> m -> memory[ias -> mar -> register_value] = (word) 0;
   storlmx(ias);
//
printf("memory: %li, ac: %li\n", ias -> m -> memory[ias -> mar -> register_value], ias -> ac -> register_value);
   if(ias -> m -> memory[ias -> mar -> register_value] == (ias -> ac -> register_value)) {
        return TEST_SUCCESSFUL;
   }

    printf("storlmx failed\n");
   return TEST_FAILED;
}

int test_storrmx(IAS* ias) {
   ias -> ac -> register_value = (word) 0b0000000000000000000000000000000000000000000000000000000000000100;
   ias -> mar -> register_value = (address) 30;
   ias -> m -> memory[ias -> mar -> register_value] = (word) 0;
   storrmx(ias);
//
   if(ias -> m -> memory[ias -> mar -> register_value] == (ias -> ac ->register_value)) {
        return TEST_SUCCESSFUL;
   }

    printf("storrmx failed\n");
   return TEST_FAILED;
}
