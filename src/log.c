#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <dirent.h>
#include "../headers/ias.h"
#include "../headers/program_loader.h"
#include "../headers/log.h"

//global variables
char* LOGS_PATH = NULL;
DIR* LOGS_DIR = NULL;
DIR* LOG_DIR = NULL; //difference between it and LOGS_DIR is that the former stores the logs for a particular IAS program execution, while the latter is just a root directory for all the logs
char* LOG_DIR_NAME[NAME_MAX];
FILE* LOG_REGISTER = NULL;
FILE* LOG_MEMORY_DMP = NULL;
char* MEMORY_DUMP_FILE_BUFFER;

//constants
const char* REGISTER_LOGFILE_NAME = "register_logs";
const char* MEMORYDMP_LOGFILE_NAME = "memory_dump";
const char* LOGS_PATH_PREFIX_LINUX = "/home/";
const char* LOGS_PATH_POSTFIX = "/IAS_LOGS/";
const size_t MEMORY_DUMP_FILE_BUFFER_SIZE = 1024;
//creates the logspath based on the username
int create_logspath() {
    char* username = getenv("USER");
    strcpy(LOGS_PATH, strcat(LOGS_PATH_PREFIX_LINUX, strcat(username, LOGS_PATH_POSTFIX)));
    
    return SUCCESSFUL;
}

//check if the logs directory exists
bool exists_logsdir() {
    LOGS_DIR = opendir(LOGS_PATH);
    if(!LOGS_DIR) {
        return false;
    }

    closedir(LOGS_DIR);
    return true;
}

//creates a logs directory
int create_logsdir() {
    const int status = mkdir(LOGS_PATH, 755);
    if(status != 0) {
        perror("Failed to create a logs directory.\n");
        return errno;
    }

    return SUCCESSFUL;
}

//creates a log directory for a specific execution
int create_logdir() {
   generate_logdirname();
   LOG_DIR = mkdir(LOG_DIR_NAME, 755);

   if(!LOG_DIR) {
        perror("Failed to create a log directory\n");
        return errno;
   }
   return SUCCESSFUL;
}

//genearte a name for a logdir based on the current time
int generate_logdirname() {
    //get the current time
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);

    //generate the file name
    strftime(LOG_DIR_NAME, NAME_MAX, "%Y-%m-%d_%H-%M-%S", timeinfo);

    return SUCCESSFUL;
}

//generate a new register logfile
int createlog_register(char* logdirname) {
    //create the register logfile
    LOG_REGISTER = fopen(strcat(LOGS_PATH, strcat(logdirname, strcat('/', REGISTER_LOGFILE_NAME))), "a");
    if(!LOG_REGISTER) {
        perror("Failed to create a register logfile.\n"); 
        return errno;
    }

    //append initial contents to the register file
    fprintf(LOG_REGISTER, "Cycle 0:-\n");
    fprintf(LOG_REGISTER, "PC: 0, IR: 0, MAR: 0, MBR: 0, IBR: 0, AC: 0, MQ: 0\n\n");

    fclose(LOG_REGISTER);
    return SUCCESSFUL;
}

//generate a new memory dump logfile
int createlog_memorydmp(char* logdirname, Data* ndata, size_t length) {
    //create the memory dump logfile
    LOG_MEMORY_DMP = fopen(strcat(LOGS_PATH, strcat(logdirname, strcat('/', REGISTER_LOGFILE_NAME))), "w");
    if(!LOG_MEMORY_DMP) {
        perror("Failed to create a memory dump file.\n");
        return errno;
    }

    /*
        Append initial contents to the memory dump file
        
        The way this will work is that the function will be provided with a Data array containing the
        addresses and values of all the memory words that have been changed in the IAS memory
    */
   
    //copy new data into memory dump file buffer
    copyDataIntoMemdumpBuffer(ndata, length);

    //append contents of the the buffer to the memory dump file
    for(int i = 0; i<MEMORY_DUMP_FILE_BUFFER_SIZE; i++) {
        fprintf(LOG_MEMORY_DMP, "%i: %li\n", i, MEMORY_DUMP_FILE_BUFFER[i]);
    }

   fclose(LOG_MEMORY_DMP);
   return SUCCESSFUL;
}

//update the register logfile
int updatelog_register(char* logdirname, IAS* ias) {
    LOG_REGISTER = fopen(strcat(LOGS_PATH, strcat(logdirname, strcat('/', REGISTER_LOGFILE_NAME))), "a");
    if(!LOG_REGISTER) {
        fprintf(stderr, "Trying to update a register logfile that does not exist.\n");
        return FILE_DOES_NOT_EXIST;
    }

    fprintf(LOG_REGISTER, "PC: %i, IR: %i, MAR: %i, MBR: %li, IBR: %i, AC: %li, MQ: %li\n\n",
            ias -> pc, ias -> ir, ias -> mar, ias -> mbr, ias -> ibr, ias -> ac, ias -> mq);
    return SUCCESSFUL;
}


char* createMemorydmpFileBuffer(size_t size) {
    char* buffer = (word*) calloc(size, sizeof(word));
    if(!buffer) {
        perror("Failed to allocate memory to memory dump file buffer.\n");
        return NULL;
    }

    return buffer;
}

int copyDataIntoMemdumpBuffer(Data* data, size_t length) {
    for(int i = 0; i<length; i++) {
        MEMORY_DUMP_FILE_BUFFER[data[i].adr] = data[i].val;
    }
    
    return SUCCESSFUL;
}

bool hasChanged_word(address adr, Data* data, size_t length) {
    for(int i = 0; i<length; i++) {
        if(data[i].adr == adr) {return true;}
    }
    return false;
}