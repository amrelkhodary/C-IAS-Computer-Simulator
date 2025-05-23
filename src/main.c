#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <dirent.h>
#include <errno.h>
#include "../headers/ias.h"
#include "../headers/tests.h"
#include "../headers/program_loader.h"
#include "../headers/log.h"
//defining program constants
const opcode LOAD_MQ    = (opcode) 0b00001010; //transfer contents of MQ to AC 
const opcode LOAD_MQ_MX = (opcode) 0b00001001; //transfer contents of memory location X to MQ
const opcode STOR_MX    = (opcode) 0b00100001; //store value in AC in memory location X
const opcode LOAD_MX    = (opcode) 0b00000001; //load value from memory location X to AC
const opcode LOAD_nMX   = (opcode) 0b00000010; //load negative the value from memory location X to AC
const opcode LOAD_aMX   = (opcode) 0b00000011; //load absolution the value from memory location X to AC
const opcode LOAD_naMX  = (opcode) 0b00000100; //load negative the absolute value from memory location X to AC
const opcode JUMP_lMX   = (opcode) 0b00001101; //take instruction from the left half of memory word X
const opcode JUMP_rMX   = (opcode) 0b00001110; //take instruction from teh right half of memory word X
const opcode CJUMP_lMX  = (opcode) 0b00001111; //if AC is nonnegative, take instruction from left half of memory word X
const opcode CJUMP_rMX  = (opcode) 0b00010000; //if AC is nonnegative, take instruction from right half of memory word X
const opcode ADD_MX     = (opcode) 0b00000101; //add value from memory location X to AC
const opcode ADD_aMX    = (opcode) 0b00000111; //add absolute the value from memory location X to AC
const opcode SUB_MX     = (opcode) 0b00000110; //subtract the value from memory location X from AC
const opcode SUB_aMX    = (opcode) 0b00001000; //substract absolute the value from memory location X from AC
const opcode MUL_MX     = (opcode) 0b00001011; //multiply value from memory location X by MQ, store most significat bits in AC, lest significant in MQ
const opcode DIV_MX     = (opcode) 0b00001100; //divide AC by value from memory location X, put the quotient in MQ, the remainder in AC
const opcode LSH        = (opcode) 0b00010100; //left shift AC by one bit (i.e. multiply by 2)
const opcode RSH        = (opcode) 0b00010101; //right shift AC by one bit (i.e. divide by 2)
const opcode STOR_lMX   = (opcode) 0b00010010; //replace left address by 12 rightmost bits in AC
const opcode STOR_rMX   = (opcode) 0b00010011; //reaplce right address by 12 rightmost bits in AC  
const opcode NOP        = (opcode) 0b00000000; //original ias did NOT include a NOP instruction, this is just used for internal reasons inside the code

const address ADDRESS_MASK                    = (address)  0b0000111111111111; //mask used to extract 12 address bits out of uint16
const half_word HALF_WORD_MASK                = (half_word)0b00000000000011111111111111111111; //mask used to extract 20 bits out of uint32
const half_word HALF_WORD_INSTRUCTION_MASK    = (half_word)0b00000000000011111111000000000000; //mask used to extract the instruction out of a half word
const half_word HALF_WORD_ADDRESS_MASK        = (half_word)0b00000000000000000000111111111111; //mask used to extract the address out of a half word
const word WORD_MASK                          = (word)     0b0000000000000000000000001111111111111111111111111111111111111111; //mask used to extract 40 bits out of uint64
const word SIGN_BIT_MASK                      = (word)     0b0000000000000000000000001000000000000000000000000000000000000000; //mask used to extract sign bit out of uint64
const word NUMBER_VALUE_MASK                  = (word)     0b0000000000000000000000000111111111111111111111111111111111111111; //mask used to extract 39 bits (number value aside from sign bit) out of uint64
const word SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK = (word)     0b0000000000000000000000001000000000000000000000000000000000000000; //same value as SIGN_BIT_MASK, varaible declared to enhace program readability, mask used to switch sign bit from 0 to 1 (negative to positive)
const word SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK = (word)     0b1111111111111111111111110111111111111111111111111111111111111111; //mask used to switch the sign bit from 1 to 0 (negative to positive)
const word LEFT_ADDRESS_WORD_MASK             = (word)     0b0000000000000000000000000000000011111111111100000000000000000000;
const word RIGHT_ADDRESS_WORD_MASK            = (word)     0b0000000000000000000000000000000000000000000000000000111111111111; //mask used to extract the right address from a 40 bit word
const word LEFT_INSTRUCTION_WORD_MASK         = (word)     0b0000000000000000000000001111111100000000000000000000000000000000; //mask used to extract the left instruction from a 40 bit word
const word RIGHT_INSTRUCTION_WORD_MASK        = (word)     0b0000000000000000000000000000000000000000000011111111000000000000; //mask used to extract the right instruction from a 40 bit word
const word AC_ADDRESS_MASK                    = (word)     0b0000000000000000000000000000000000000000000000000000111111111111; //mask used to extract the 12 rightmost bits from AC
const word RIGHT_HALF_WORD_MASK               = (word)     0b0000000000000000000000000000000000000000000011111111111111111111; //mask used to extract the right half of a word
const word LEFT_HALF_WORD_MASK                = (word)     0b0000000000000000000000001111111111111111111100000000000000000000; //mask used to extract the left half of a word
const uint64_t MAX_INTEGER = (uint64_t) 549755813887; //the max number that could be represented by a 40-bit word that uses a sign bit and two's complement for negative numbers
const bool IGNORE_OVERFLOW = true;
const bool DONT_IGNORE_OVERFLOW = false;
const int IAS_MEMORY_WORD_COUNT = 1024;

int main(int argc, char** argv) {
    //exit code for the program
    int exit_code;
    //make sure the user provided only one argument and that that arguemnt is a valid file
    if(argc != 2) {
        fprintf(stderr, "Too many arguments passed to IAS, make sure you only pass the filename of the program you want to execute.\n");
        return FATAL_INVALID_PROGRAM_ARGUMENTS;
    }

    FILE* programfile = fopen(argv[1], "r");
    if(!programfile) {
        perror("Could not open program file");
        return errno;
    }
    fclose(programfile);

    //create an instace of the IAS computer
    IAS* ias = startIAS();

    //start the logging module
    exit_code = startLogging(ias);
    if(exit_code != SUCCESSFUL) {
        printf("\033[1;31mYour program was not executed successfully!\033[0m\nIf you think there were any issues or bugs with IAS, please send an email at \033[4;36mamrelkhodarybusiness@gmail.com\033[0m.\n");
        return exit_code;
    }

    //load the program and data into IAS memory
    exit_code = load_program(ias, argv[1]);
    if(exit_code != SUCCESSFUL) {
        printf("\033[1;31mYour program was not executed successfully!\033[0m\nIf you think there were any issues or bugs with IAS, please send an email at \033[4;36mamrelkhodarybusiness@gmail.com\033[0m.\n");
        return exit_code;
    }

    //run the IAS computer
    exit_code = run(ias);
    if(exit_code != SUCCESSFUL) {
        printf("\033[1;31mYour program was not executed successfully!\033[0m\nIf you think there were any issues or bugs with IAS, please send an email at \033[4;36mamrelkhodarybusiness@gmail.com\033[0m.\n");
        return exit_code;
    } 

    //free all the dynamically allocated memory used by the program
    freeLogs();
    freeProgramLoader();   
    freeIAS(ias);

    //print a message to the user instructing him where to find logs
    printf("\033[1;32mYour program was executed successfully!\033[0m \nRegister and memory dump logsfiles have been generated under \033[1m~/IAS_LOGS\033[0m.\nIf you think there were any issues or bugs with IAS, please send an email at \033[4;36mamrelkhodarybusiness@gmail.com\033[0m.\n");
    return 0;
}