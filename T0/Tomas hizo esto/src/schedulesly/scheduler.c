#include "scheduler.h"

Scheduler *InitializeScheduler(InputFile *input_file)
{
    Scheduler *scheduler = (Scheduler *)calloc(1, sizeof(Scheduler));

    scheduler->qStart = atoi(input_file->lines[0][0]);
    scheduler->qDelta = atoi(input_file->lines[0][1]);
    scheduler->qMin = atoi(input_file->lines[0][2]);
    scheduler->nolines = 1;
    scheduler->ExpectedMAX = 10; // Variable, its the max number of
    // processes that i would expect to have to handle
    scheduler->time = 0;

    return scheduler;
}

ProcessGroup **InitializeProcessGroup(Scheduler *scheduler, InputFile *input_file, int totalGroups)
{
    // We create an array of process groups
    scheduler->processGroupArray = (ProcessGroup **)calloc(totalGroups, sizeof(ProcessGroup *));
    int max = scheduler->ExpectedMAX;
    for (int i = 1; i < input_file->len; ++i)
    {
        scheduler->processGroupArray[i - 1] = (ProcessGroup *)calloc(1, sizeof(ProcessGroup));
        // Now each process group inside the array will have its own process array
        scheduler->processGroupArray[i - 1]->processes = (Process **)calloc(max, sizeof(Process *)); // Create a process group
        scheduler->processGroupArray[i - 1]->gid = i;
        scheduler->processGroupArray[i - 1]->TI = atoi(input_file->lines[i][0]);
        scheduler->processGroupArray[i - 1]->q = scheduler->qStart;
    }

    return scheduler->processGroupArray;
}

Process *CreateProcess(ProcessGroup *processGroup, int pid, int ppid, int gid, int CI, int NH)
{
    Process *process = (Process *)calloc(1, sizeof(Process));
    process->pid = pid;
    process->ppid = ppid;
    process->gid = gid;
    process->CI = CI;
    process->NH = NH;
    process->state = "READY";
    int index = processGroup->noProcesses;
    processGroup->processes[index] = process;
    processGroup->noProcesses += 1;

    return process;
}

void add2Queue(Queue *queue, ProcessGroup *processGroup)
{
    if (queue->firstGroup == NULL)
    {
        queue->firstGroup = processGroup;
    }
    else
    {
        while (queue->previousGroup != NULL)
        {
            queue->previousGroup = queue->previousGroup;
        }
        queue->previousGroup = processGroup;
    }

    if (queue->firstGroup == processGroup)
    {
        processGroup->state = "READY";
    }
}

int calculate_q(int qStart, int qDelta, int qMin, int iteration)
{
    int q = qStart - (qDelta * iteration);
    if (q < qMin)
    {
        return qMin;
    }
    return q;
}

int countArgs(InputFile *inputFile, int fileLine)
{
    int count = 0;
    while (inputFile->lines[fileLine][count] != NULL)
    {
        // printf("%d\n", atoi(inputFile->lines[fileLine][count]));
        count += 1;
    }

    return count;
}

// 7 8 1 4 1 2 0 0 6 3
//   8   4   2     4  8

int RunGroup(InputFile *inputFile, Scheduler *scheduler, int fileLine, int total_n_args, ProcessGroup *processGroup, FILE *output_file, int time, int arg, int pid, int ppid)
{
    int CI = atoi(inputFile->lines[fileLine][arg]);     // Number of seconds itll work
    int NH = atoi(inputFile->lines[fileLine][arg + 1]); // Number of childs

    printf("CI: %d, NH: %d\n", CI, NH);

    if (processGroup->q >= CI) // There is enough job units to execute the process
    {
        processGroup->q -= CI;
        Process *process = CreateProcess(processGroup, pid, ppid, processGroup->gid, CI, NH);
        report_ENTER(output_file, process->pid, process->ppid, process->gid, scheduler->time, scheduler->nolines, total_n_args);
        report_RUN(output_file, process->pid, CI);
        printf("Process created: pid %d\n", process->pid);
        scheduler->time += CI; // We add the time the process is being executed

        if (NH == 0)
        {
            report_END(output_file, process->pid, time); // end process
            return 4;                                    // 2
        }
        else
        {

            for (int i = 0; i < NH; i++)
            { // 8 4
                pid += 1;
                arg += RunGroup(inputFile, scheduler, fileLine, total_n_args, processGroup, output_file, time, arg + 2, pid, process->pid);
                // llamamos con arg + 1 al padre y con arg + 2 al hijo
            }
            return 4;
        }
    }
    else
    { // There isnt enough job units to execute a new program, new iteration
        processGroup->iteration += 1;
        GENERAL_REPORT(output_file, scheduler->processGroupArray, scheduler->time, total_n_args);
        // check if new groups can enter now
    }
}

void report_IDLE(FILE *output_file, int time)
{
    fprintf(output_file, "IDLE %d\n", time);
}

void report_ENTER(FILE *output_file, int pid, int ppid, int gid, int time, int line, int num_args)
{
    fprintf(output_file, "ENTER %d %d %d TIME %d LINE %d ARG %d\n", pid, ppid, gid, time, line, num_args);
}

void report_RUN(FILE *output_file, int pid, int worked_time)
{
    fprintf(output_file, "RUN %d %d\n", pid, worked_time);
}

void report_WAIT(FILE *output_file, int pid)
{
    fprintf(output_file, "WAIT %d\n", pid);
}

void report_RESUME(FILE *output_file, int pid)
{
    fprintf(output_file, "RESUME %d\n", pid);
}

void report_END(FILE *output_file, int pid, int time)
{
    fprintf(output_file, "END %d TIME %d\n", pid, time);
}

void GENERAL_REPORT(FILE *output_file, ProcessGroup **processGroupArray, int time, int totalGroups)
{
    fprintf(output_file, "REPORT START\n");
    fprintf(output_file, "TIME %d\n", time);
    for (int i = 0; i < totalGroups; i++)
    {
        fprintf(output_file, "GROUP %d %d\n", processGroupArray[i]->gid, processGroupArray[i]->noProcesses);
        for (int j = 0; j < processGroupArray[i]->noProcesses; j++)
        {
            Process *process = processGroupArray[i]->processes[j];
            fprintf(output_file, "PROGRAM %d %d %d %s\n", process->pid, process->ppid, process->gid, process->state);
        }
    }
    fprintf(output_file, "REPORT END\n");
}

void freeScheduler(Scheduler *scheduler, int totalGroups)
{
    // The idea is to: Free each process that was created in its own processGroup, then
    // the processGroup, then the processGroupArray and finally the scheduler
    for (int i = 0; i < totalGroups; i++)
    {
        for (int j = 0; j < scheduler->processGroupArray[i]->noProcesses; j++)
        {
            printf("Process pid %d freed\n", scheduler->processGroupArray[i]->processes[j]->pid);
            free(scheduler->processGroupArray[i]->processes[j]); // free the process
        }
        free(scheduler->processGroupArray[i]->processes); // free the array of processes
        free(scheduler->processGroupArray[i]);            // free the processGroup
    }
    free(scheduler->processGroupArray); // free the array of processGroups
    free(scheduler);                    // free the scheduler
}

void freeQueue(Queue *queue)
{
    free(queue);
}