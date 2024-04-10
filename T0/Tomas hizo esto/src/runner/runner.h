#pragma once
#include <stdbool.h> /* importamos booleanos*/
#include <stdlib.h>  /* librerias */
#include <stdio.h>   // FILE, fopen, fclose, etc.
#include "../file_manager/manager.h"
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include "math.h"

#define EXIT_ERROR 1
#define EXIT_SUCCESS 0

typedef struct Process
{
    int pid;               // Process ID
    char *program_name;    // Name of the program
    char **arguments;      // Arguments to the program
    struct timespec start; // Start time
    struct timespec end;   // End time
    int status;            // Exit status
} Process;

extern int file_len; // Declare file_len as extern
extern Process *processes; 

void programExecution(char *program, char **arguments);

void waitForProcesses(Process *processes, int file_len, int timeout, int *noProcesses);

void printProcesses(Process *processes, int file_len, FILE *output_file);

void set_alarm(int argc, char const *argv[]);

void handle_alarm(int sig);