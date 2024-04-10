#pragma once
#include <stdio.h>  // FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include <stdbool.h>
#include <string.h> // strcmp
#include "../file_manager/manager.h"

typedef struct Process
{
    int pid;                         // Process ID
    int ppid;                        // Parent Process ID, 0 if the OS is the parent
    int gid;                         // Group Process PID where it belongs
    int CI;                          // Time interval in which the process could create a child process
    int NH;                          // Number of child processes that the process will create
    int CF;                          // Time interval in which the process will finish after creating all its child processes
    char *state;                     // Process state : READY, RUNNING, WAITING, FINISHED
    struct Process *executing_child; // Posible child process
} Process;

typedef struct ProcessGroup
{
    Process **processes; // Array of processes
    int gid;             // Group ID
    int TI;              // Time interval in which the processes arrive to the queue
    int q;               // Job units assigned to the group of processes
    int noProcesses;     // Number of processes in the group
    char *state;         // Group state : READY, RUNNING, WAITING, FINISHED
    int iteration;
} ProcessGroup;

typedef struct Scheduler
{
    ProcessGroup **processGroupArray; // Array of group processes -> processesArray = [processGroup1, pg2, pg3, ...]
    int qStart;                       // Job units initialy assigned to each group of processes
    int qDelta;                       // How many Job units are substracted from each group of processes each iteration
    int qMin;                         // Minimum Job units assigned to each group of processes each iteration
    int time;
    int activeGroups;
    int idle_time;
    int nolines;
    int noProcesses; // Number of processes in the group, helps with the free
    int totalGroups;
    int ExpectedMAX;
} Scheduler;

typedef struct Queue
{
    ProcessGroup *firstGroup;
    ProcessGroup *previousGroup;
} Queue;

Scheduler *InitializeScheduler(InputFile *input_file);

ProcessGroup **InitializeProcessGroup(Scheduler *scheduler, InputFile *input_file, int totalGroups);

Process *CreateProcess(ProcessGroup *processGroup, int pid, int ppid, int gid, int CI, int NH);

void add2Queue(Queue *queue, ProcessGroup *processGroup);

int calculate_q(int qStart, int qDelta, int qMin, int iteration);

int countArgs(InputFile *input_file, int fileLine);

int RunGroup(InputFile *inputFile, Scheduler *scheduler, int fileLine, int total_n_args, ProcessGroup *processGroup, FILE *output_file, int time, int arg, int pid, int ppid);

void report_IDLE(FILE *output_file, int time);

void report_ENTER(FILE *output_file, int pid, int ppid, int gid, int time, int line, int num_args);

void report_RUN(FILE *output_file, int pid, int worked_time);

void report_WAIT(FILE *output_file, int pid);

void report_RESUME(FILE *output_file, int pid);

void report_END(FILE *output_file, int pid, int time);

void GENERAL_REPORT(FILE *output_file, ProcessGroup **processGroupArray, int time, int totalGroups);

void freeScheduler(Scheduler *scheduler, int totalGroups);

void freeQueue(Queue *queue);
