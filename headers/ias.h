//enumerating errors
typedef enum {
    SUCCESSFUL,
    FATAL_INVALID_PROGRAM_ARGUMENTS,
    FATAL_COULD_NOT_GET_WORKING_DIRECTORY,
    FATAL_COULD_NOT_READ_SOURCE_CODE,
    FATAL_FAILED_TO_START_IAS,
    INTEGER_OVERFLOW,

} Error;

typedef enum {
    MODE_READ_FROM_SOURCE_FILE,
    MODE_INTERACTIVE,
} Mode;

//defining opcodes (as mentioned in the "Computer Organization and Architecture Textbook", by William Stallings (11th ed.))
typedef uint8_t opcode;
typedef uint16_t address; 
typedef uint32_t half_word;
typedef uint64_t word;

opcode LOAD_MQ = 0b00001010; //transfer contents of MQ to AC 
opcode LOAD_MQ_MX = 0b00001001; //transfer contents of memory location X to MQ
opcode STOR_MX = 0b00100001; //store value in AC in memory location X
opcode LOAD_MX = 0b00000001; //load value from memory location X to AC
opcode LOAD_nMX = 0b00000010; //load negative the value from memory location X to AC
opcode LOAD_aMX = 0b00000011; //load absolution the value from memory location X to AC
opcode LOAD_naMX = 0b00000100; //load negative the absolute value from memory location X to AC
opcode JUMP_lMX = 0b00001101; //take instruction from the left half of memory word X
opcode JUMP_rMX = 0b00001110; //take instruction from teh right half of memory word X
opcode CJUMP_lMX = 0b00001111; //if AC is nonnegative, take instruction from left half of memory word X
opcode CJUMP_rMX = 0b00010000; //if AC is nonnegative, take instruction from right half of memory word X
opcode ADD_MX = 0b00000101; //add value from memory location X to AC
opcode ADD_aMX = 0b00000111; //add absolute the value from memory location X to AC
opcode SUB_MX = 0b00000110; //subtract the value from memory location X from AC
opcode SUB_aMX = 0b00001000; //substract absolute the value from memory location X from AC
opcode MUL_MX = 0b00001011; //multiply value from memory location X by MQ, store most significat bits in AC, lest significant in MQ
opcode DIV_MX = 0b00001100; //divide AC by value from memory location X, put the quotient in MQ, the remainder in AC
opcode LSH = 0b00010100; //left shift AC by one bit (i.e. multiply by 2)
opcode RSH = 0b00010101; //right shift AC by one bit (i.e. divide by 2)
opcode STOR_lMX = 0b00010010; //replace left address by 12 rightmost bits in AC
opcode STOR_rMX = 0b00010011; //reaplce right address by 12 rightmost bits in AC  

//defining program structures
const address ADDRESS_MASK = 0b0000111111111111;
const half_word HALF_WORD_MASK = 0b00000000000011111111111111111111;
const word WORD_MASK =     0b0000000000000000000000001111111111111111111111111111111111111111;
const word SIGN_BIT_MASK =                      0b0000000000000000000000001000000000000000000000000000000000000000;
const word SIGN_BIT_POSITIVE_TO_NEGATIVE_MASK = 0b0000000000000000000000001000000000000000000000000000000000000000; //same value as SIGN_BIT_MASK, varaible declared to enhace program readability
const word SIGN_BIT_NEGATIVE_TO_POSITIVE_MASK = 0b1111111111111111111111110111111111111111111111111111111111111111;
const word LEFT_ADDRESS_WORD_MASK             = 0b0000000000000000000000000000000011111111111100000000000000000000;
const word RIGHT_ADDRESS_WORD_MASK            = 0b0000000000000000000000000000000000000000000000000000111111111111;
const word LEFT_INSTRUCTION_WORD_MASK =         0b0000000000000000000000001111111100000000000000000000000000000000;
const word RIGHT_INSTRUCTION_WORD_MASK =        0b0000000000000000000000000000000000000000000011111111000000000000;
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

//defining program constants
const uint64_t MAX_INTEGER = 549755813887; //the max number that could be represented by a 40-bit word that uses a sign bit and two's complement for negative numbers
bool IGNORE_OVERFLOW = true;
bool DONT_IGNORE_OVERFLOW = false;

IAS* startIAS();
bool isNegative(word number);
word negative(word number);
word absoluteval(word number);

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
