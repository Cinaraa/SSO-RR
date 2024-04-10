#include "runner.h"

int file_len;
Process *processes;

void programExecution(char *program, char **arguments)
{
    execvp(program, arguments);
    // In case of error
    perror("Error executing external program");
    exit(1);
}

void waitForProcesses(Process *processes, int file_len, int timeout, int *noProcesses)
{
    if (timeout != 0)
    {
        // Set up the signal handler
        signal(SIGALRM, handle_alarm);

        // Set an alarm to go off after {timeout} seconds
        alarm(timeout);
    }

    while ((*noProcesses) > 0)
    {
        int status;
        pid_t pid = waitpid(-1, &status, 0); // Wait for any child process to finish

        if (pid > 0) // A child process has finished
        {
            // Find the process with this pid
            int i;
            for (i = 0; i < file_len; i++)
            {
                if (processes[i].pid == pid)
                {
                    break;
                }
            }

            if (i < file_len)
            {
                // The process has ended
                clock_gettime(CLOCK_MONOTONIC, &processes[i].end); // End time
                processes[i].status = status;
                --(*noProcesses);
            }
        }
    }
}

void printProcesses(Process *processes, int file_len, FILE *output_file)
{
    for (int i = 0; i < file_len; ++i)
    {

        if (processes[i].pid == -1) // If the process was not executed
        {
            continue;
        }

        if (WIFEXITED(processes[i].status)) // Check if child process finished
        {
            double time_taken = processes[i].end.tv_sec - processes[i].start.tv_sec;
            time_taken = round(time_taken); // Round the time taken to the nearest integer
            // Write the time taken by the process to the output file
            fprintf(output_file, "%s,%.0f,%d\n", processes[i].program_name, time_taken, WEXITSTATUS(processes[i].status));
        }
        else
        {
            printf("Child process did not end normally\n");
            double time_taken = processes[i].end.tv_sec - processes[i].start.tv_sec;
            time_taken = round(time_taken); // Round the time taken to the nearest integer
            // Write the time taken by the process to the output file
            fprintf(output_file, "%s,%.0f,%d\n", processes[i].program_name, time_taken, EXIT_ERROR);
        }
    }
}

void set_alarm(int argc, char const *argv[])
{
    if (argc == 5)
    {
        int max = atoi(argv[4]);
        printf("An alarm was set for %d seconds\n", max);
        alarm(max);
    }
}

void handle_alarm(int sig)
{
    for (int i = 0; i < file_len; i++)
    {
        if (processes[i].pid > 0) // If the process is still running
        {
            kill(processes[i].pid, SIGKILL); // Send SIGKILL signal to the process
        }
    }
}