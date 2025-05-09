#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "../headers/ias.h"
#include "../headers/program_loader.h"
#include "../headers/log.h"

//global variables
char* LOGS_PATH = NULL;
DIR* LOGS_DIR = NULL;
DIR* LOG_DIR = NULL; //difference between it and LOGS_DIR is that the former stores the logs for a particular IAS program execution, while the latter is just a root directory for all the logs
char LOG_DIR_NAME[NAME_MAX];
FILE* LOG_REGISTER = NULL;
FILE* LOG_MEMORY_DMP = NULL;

//constants
const char* REGISTER_LOGFILE_NAME = "register_logs";
const char* MEMORYDMP_LOGFILE_NAME = "memory_dump";
const char* LOGS_PATH_PREFIX_LINUX = "/home/";
const char* LOGS_PATH_POSTFIX = "/IAS_LOGS/";

//initiate the logging module
int startLogging(IAS* ias) {
    int returnval;

    //create a logs dir if it does not exist
    returnval = create_logspath();
    if(returnval != SUCCESSFUL) {
        return returnval;
    }
    if(!exists_logsdir()) {
       returnval = create_logsdir(); 
       if(returnval != SUCCESSFUL) {
            return returnval;
       }
    }

    //create a new log dir specifically for the current execution
    returnval = create_logdir();
    if(returnval != SUCCESSFUL) {
        return returnval;
    }

    //create a new register log file
    returnval = createlog_register(LOG_DIR_NAME);
    if(returnval != SUCCESSFUL) {
        return returnval;
    }

    //create a new memory dump log file
    returnval = createlog_memorydmp(LOG_DIR_NAME, ias);
    if(returnval != SUCCESSFUL) {
        return returnval;
    }

    return returnval;
}

//creates the logspath based on the username
int create_logspath() {
    char* username = getenv("USER");
    char LOCAL_PREFIX[256];
    char LOCAL_POSTFIX[256];
    
    strcpy(LOCAL_PREFIX, LOGS_PATH_PREFIX_LINUX);
    strcpy(LOCAL_POSTFIX, LOGS_PATH_POSTFIX);
    strcpy(LOGS_PATH, strcat(LOCAL_PREFIX, strcat(username, LOCAL_POSTFIX)));
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
   errno = 0; //setting errno to zero to check if it changes as calling mkdir as that would mean that an error occurred
   generate_logdirname();
   mkdir(LOG_DIR_NAME, 755);

   if(errno != 0) {
        perror("Could not create a new log dir.\n");
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
    //creating local copies of the global vars so that the global ones would not be affected
    char LOCAL_LOGS_PATH[256];
    char LOCAL_LOG_DIR_NAME[256];
    char LOCAL_REGISTER_LOGFILE_NAME[256];
    strcpy(LOCAL_LOGS_PATH, LOGS_PATH);
    strcpy(LOCAL_LOG_DIR_NAME, logdirname);
    strcpy(LOCAL_REGISTER_LOGFILE_NAME, REGISTER_LOGFILE_NAME);
    //create the register logfile
    LOG_REGISTER = fopen(strcat(LOCAL_LOGS_PATH, strcat(LOCAL_LOG_DIR_NAME, strcat("/", LOCAL_REGISTER_LOGFILE_NAME))), "w");
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
int createlog_memorydmp(char* logdirname, IAS* ias) {
    //create local copies of the global vars so that their values wouldn't change
    char LOCAL_LOGS_PATH[256];
    char LOCAL_LOG_DIR_NAME[256];
    char LOCAL_MEMORY_DMP_FILENAME[256];
    strcpy(LOCAL_LOGS_PATH, LOGS_PATH);
    strcpy(LOCAL_LOG_DIR_NAME, logdirname);
    strcpy(LOCAL_MEMORY_DMP_FILENAME, MEMORYDMP_LOGFILE_NAME);
    //create the memory dump logfile
    LOG_MEMORY_DMP = fopen(strcat(LOCAL_LOGS_PATH, strcat(LOCAL_LOG_DIR_NAME, strcat("/", LOCAL_MEMORY_DMP_FILENAME))), "a");
    if(!LOG_MEMORY_DMP) {
        perror("Failed to create a memory dump file.\n");
        return errno;
    }

    //append contents of the the buffer to the memory dump file
    for(int i = (address) 0; i<IAS_MEMORY_WORD_COUNT; i++) {
        fprintf(LOG_MEMORY_DMP, "%i: %li\n", i, ias -> m -> memory[i]);
    }

   fclose(LOG_MEMORY_DMP);
   return SUCCESSFUL;
}

//update the register logfile
int updatelog_register(char* logdirname, IAS* ias) {    //creating local copies of the global vars so that the global ones would not be affected
    char LOCAL_LOGS_PATH[256];
    char LOCAL_LOG_DIR_NAME[256];
    char LOCAL_REGISTER_LOGFILE_NAME[256];
    strcpy(LOCAL_LOGS_PATH, LOGS_PATH);
    strcpy(LOCAL_LOG_DIR_NAME, logdirname);
    strcpy(LOCAL_REGISTER_LOGFILE_NAME, REGISTER_LOGFILE_NAME);
    //create the register logfile
    LOG_REGISTER = fopen(strcat(LOCAL_LOGS_PATH, strcat(LOCAL_LOG_DIR_NAME, strcat("/", LOCAL_REGISTER_LOGFILE_NAME))), "a");
    if(!LOG_REGISTER) {
        fprintf(stderr, "Trying to update a register logfile that does not exist.\n");
        return FILE_DOES_NOT_EXIST;
    }

    fprintf(LOG_REGISTER, "PC: %i, IR: %i, MAR: %i, MBR: %li, IBR: %i, AC: %li, MQ: %li\n\n",
            ias -> pc -> register_value, ias -> ir -> register_value, ias -> mar -> register_value, ias -> mbr -> register_value
            , ias -> ibr -> register_value, ias -> ac -> register_value, ias -> mq -> register_value);
    
    fclose(LOG_REGISTER);
    return SUCCESSFUL;
}
