#ifndef H_IAS
    #define H_IAS
    //types
    typedef enum {
        SUCCESSFUL,
        FATAL_INVALID_PROGRAM_ARGUMENTS,
        FATAL_COULD_NOT_GET_WORKING_DIRECTORY,
        FATAL_COULD_NOT_READ_SOURCE_CODE,
        FATAL_FAILED_TO_START_IAS,
        INTEGER_OVERFLOW,
        UNRECOGNIZED_COMMAND,
        
    } Error;

    typedef enum {
        MODE_READ_FROM_SOURCE_FILE,
        MODE_INTERACTIVE,
    } Mode;

    typedef uint8_t opcode;
    typedef uint16_t address; 
    typedef uint32_t half_word;
    typedef uint64_t word;

    typedef struct PC {
        uint16_t register_value;
        uint16_t MASK;
    } PC;

    typedef struct IR {
        uint8_t register_value;
    } IR;

    typedef struct MAR {
        uint16_t register_value;
        uint16_t MASk;
    } MAR;

    typedef struct MBR {
        uint64_t register_value;
        uint64_t MASK;
    } MBR;

    typedef struct IBR {
        uint32_t register_value;
        uint32_t MASK;
    } IBR;

    typedef struct AC {
        uint64_t register_value;
        uint64_t MASK;
    } AC;

    typedef struct MQ {
        uint64_t register_value;
        uint64_t MASK;
    } MQ;
    typedef struct Memory {
        uint64_t* memory;
    } Memory;

    typedef struct IAS {
        PC* pc;
        IR* ir;
        MAR* mar;
        MBR* mbr;
        IBR* ibr;
        AC* ac;
        MQ* mq;
        Memory* m;
    } IAS;

    //constants
    extern const opcode LOAD_MQ; //transfer contents of MQ to AC 
    extern const opcode LOAD_MQ_MX; //transfer contents of memory location X to MQ
    extern const opcode STOR_MX; //store value in AC in memory location X
    extern const opcode LOAD_MX; //load value from memory location X to AC
    extern const opcode LOAD_nMX; //load negative the value from memory location X to AC
    extern const opcode LOAD_aMX; //load absolution the value from memory location X to AC
    extern const opcode LOAD_naMX; //load negative the absolute value from memory location X to AC
    extern const opcode JUMP_lMX; //take instruction from the left half of memory word X
    extern const opcode JUMP_rMX; //take instruction from teh right half of memory word X
    extern const opcode CJUMP_lMX; //if AC is nonnegative, take instruction from left half of memory word X
    extern const opcode CJUMP_rMX; //if AC is nonnegative, take instruction from right half of memory word X
    extern const opcode ADD_MX; //add value from memory location X to AC
    extern const opcode ADD_aMX; //add absolute the value from memory location X to AC
    extern const opcode SUB_MX; //subtract the value from memory location X from AC
    extern const opcode SUB_aMX; //substract absolute the value from memory location X from AC
    extern const opcode MUL_MX; //multiply value from memory location X by MQ, store most significat bits in AC, lest significant in MQ
    extern const opcode DIV_MX; //divide AC by value from memory location X, put the quotient in MQ, the remainder in AC
    extern const opcode LSH; //left shift AC by one bit (i.e. multiply by 2)
    extern const opcode RSH; //right shift AC by one bit (i.e. divide by 2)
    extern const opcode STOR_lMX; //replace left address by 12 rightmost bits in AC
    extern const opcode STOR_rMX; //reaplce right address by 12 rightmost bits in AC  
    extern const opcode NOP;

    extern const address ADDRESS_MASK; //mask used to extract 12 address bits out of uint16
    extern const half_word HALF_WORD_MASK; //mask used to extract 20 bits out of uint32
    extern const half_word HALF_WORD_INSTRUCTION_MASK; //mask used to extract the instruction out of a half word
    extern const half_word HALF_WORD_ADDRESS_MASK; //mask used to extract the address out of a half word
    extern const word WORD_MASK; //mask used to extract 40 bits out of uint64
    extern const word SIGN_BIT_MASK; //mask used to extract sign bit out of uint64
    extern const word NUMBER_VALUE_MASK; //mask used to extract 39 bits (number value aside from sign bit) out of uint64
    extern const word SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK ; //same value as SIGN_BIT_MASK, varaible declared to enhace program readability, mask used to switch sign bit from 0 to 1 (negative to positive)
    extern const word SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK ; //mask used to switch the sign bit from 1 to 0 (negative to positive)
    extern const word LEFT_ADDRESS_WORD_MASK;
    extern const word RIGHT_ADDRESS_WORD_MASK; //mask used to extract the right address from a 40 bit word
    extern const word LEFT_INSTRUCTION_WORD_MASK; //mask used to extract the left instruction from a 40 bit word
    extern const word RIGHT_INSTRUCTION_WORD_MASK; //mask used to extract the right instruction from a 40 bit word
    extern const word AC_ADDRESS_MASK; //mask used to extract the 12 rightmost bits from AC
    extern const word RIGHT_HALF_WORD_MASK;
    extern const word LEFT_HALF_WORD_MASK;
    extern const uint64_t MAX_INTEGER; //the max number that could be represented by a 40-bit word that uses a sign bit and two's complement for negative numbers
    extern const bool IGNORE_OVERFLOW;
    extern const bool DONT_IGNORE_OVERFLOW;

    //function headers
    IAS* startIAS();
    int run(IAS* ias);
    int fetch(IAS* ias);
    int execute(IAS* ias);
    bool isNegative(word number);
    word negative(word number);
    word absoluteval(word number);
    int setmem(IAS* ias, address adr, word value);

    int loadmq(IAS* ias);
    int loadmqmx(IAS* ias);
    int stormx(IAS* ias);
    int loadmx(IAS* ias);
    int loadnmx(IAS* ias);
    int loadamx(IAS* ias);
    int loadnamx(IAS* ias);
    int jumplmx(IAS* ias);
    int jumprmx(IAS* ias);
    int cjumplmx(IAS* ias);
    int cjumprmx(IAS* ias);
    int addmx(IAS* ias);
    int addamx(IAS* ias);
    int submx(IAS* ias);
    int subamx(IAS* ias);
    int mulmx(IAS* ias);
    int divmx(IAS* ias);
    int lsh(IAS* ias);
    int rsh(IAS* ias);
    int storlmx(IAS* ias);
    int storrmx(IAS* ias);

#endif