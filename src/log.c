//TODO: Implement memorydmp functions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include "../headers/ias.h"
#include "../headers/log.h"

//global variables
char* LOGS_PATH = NULL;
FILE* LOGS_DIR = NULL;
FILE* LOG_DIR = NULL; //difference between it and LOGS_DIR is that the former stores the logs for a particular IAS program execution, while the latter is just a root directory for all the logs
FILE* LOG_REGISTER = NULL;
FILE* LOG_MEMORY_DMP = NULL;

//constants
const char* REGISTER_LOGFILE_NAME = "register_logs";
const char* MEMORYDMP_LOGFILE_NAME = "memory_dump";
const char* LOGS_PATH_PREFIX_LINUX = "/home/";
const char* LOGS_PATH_POSTFIX = "/IAS_LOGS/";

//creates the logspath based on the username
int create_logspath() {
    char* username = getenv("USER");
    strcpy(LOGS_PATH, strcat(LOGS_PATH_PREFIX_LINUX, strcat(username, LOGS_PATH_POSTFIX)));
    
    return SUCCESSFUL;
}

//check if the logs directory exists
bool exists_logsdir() {
    LOGS_DIR = fopen(LOGS_PATH, "r");
    if(!LOGS_DIR) {
        return false;
    }

    return true;
}

//creates a logs directory
int create_logsdir() {
    LOGS_DIR = fopen(LOGS_PATH, "w");
    if(!LOGS_DIR) {
        perror("Failed to create a logs directory.\n");
        return errno;
    }

    return SUCCESSFUL;
}

//creates a log directory for a specific execution
int create_logdir() {
   LOG_DIR = fopen(generate_logdirname(), "w");
   if(!LOG_DIR) {
        perror("Failed to create a log directory\n");
        return errno;
   }
   return SUCCESSFUL;
}

//genearte a name for a logdir based on the current time
char* generate_logdirname() {
    char* dirname = "";

    //get the current time
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);

    //generate the file name
    strftime(dirname, sizeof(dirname), "%Y-%m-%d_%H-%M-%S", timeinfo);

    return dirname;
}

//generate a new register logfile
int createlog_register() {
    //create the register logfile
    LOG_REGISTER = fopen(strcat(LOGS_PATH, strcat(generate_logdirname(), strcat('/', REGISTER_LOGFILE_NAME))), "a");
    if(!LOG_REGISTER) {
        perror("Failed to create a register logfile.\n"); 
        return errno;
    }

    //append initial contents to the register file
    fprintf(LOG_REGISTER, "Cycle 0:-\n");
    fprintf(LOG_REGISTER, "PC: 0, IR: 0, MAR: 0, MBR: 0, IBR: 0, AC: 0, MQ: 0\n\n");

    return SUCCESSFUL;
}

//generate a new memory dump logfile
int createlog_memorydmp() {
    //create the memorydump logfile
    LOG_MEMORY_DMP = fopen(strcat(LOGS_PATH, strcat(generate_logdirname(), strcat('/', MEMORYDMP_LOGFILE_NAME))), "a");
    if(!LOG_MEMORY_DMP) {
        perror("Failed to create a memory dump file.\n");
        return errno;
    }

    //TOOD: append initial contents to the memorydump logfile
    

    return SUCCESSFUL;
}

//update the register logfile
int updatelog_register(IAS* ias) {
    if(!LOG_REGISTER) {
        printf("Trying to update a register logfile that does not exist.\n");
        return FILE_DOES_NOT_EXIST;
    }

    fprintf(LOG_REGISTER, "PC: %i, IR: %i, MAR: %i, MBR: %li, IBR: %i, AC: %li, MQ: %li\n\n",
            ias -> pc, ias -> ir, ias -> mar, ias -> mbr, ias -> ibr, ias -> ac, ias -> mq);
    return SUCCESSFUL;
}

//update the memory dump logfile
int updatelog_memorydmp(IAS* ias) {
    //TODO: Implement function
}