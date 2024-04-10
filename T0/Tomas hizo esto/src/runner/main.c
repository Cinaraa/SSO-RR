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

void handle_alarm(int sig, file_len, pro)
{
    for (int i = 0; i < file_len; i++)
    {
        if (processes[i].pid > 0) // If the process is still running
        {
            kill(processes[i].pid, SIGKILL); // Send SIGKILL signal to the process
        }
    }
}

int main(int argc, char const *argv[])
{
	// First check if the number of arguments is correct
	int file_len;
	if (argc < 4 || argc > 5)
	{
		printf("Usage: %s input_file output_file parameter1 parameter2 [optional_parameter]\n", argv[0]);
		return 1;
	}

	// Read inputs
	char *file_name = (char *)argv[1];
	InputFile *input_file = read_file(file_name);

	char *output_file_name = (char *)argv[2];
	FILE *output_file = fopen(output_file_name, "a");

	char amount = atoi(argv[3]);

	file_len = input_file->len;

	printf("Number of lines: %d\n", input_file->len);
	printf("Max amount of processes at the same time: %d\n", amount);

	int noProcesses = 0; // Number of processes running at the same time

	processes = calloc(input_file->len, sizeof(struct Process)); // Array to store the Process instances

	for (int i = 0; i < input_file->len; ++i)
	{

		// // Verify if the number of processes is less than the amount specified
		// if (noProcesses >= amount)
		// {
		// 	// In case that the amount of processes is greater than the specified amount
		// 	// Wait for all the processes to finish
		// 	wait(NULL);
		// 	--noProcesses;
		// }

		// Verify if the first argument is -1
		if (atoi(input_file->lines[i][0]) == -1)
		{
			int timeout = atoi(input_file->lines[i][2]);
			waitForProcesses(processes, input_file->len, timeout, &noProcesses); // Wait for all the processes to finish
			processes[i].pid = -1;
			continue;
		}

		// Extract the program name and arguments from the input file
		char *program_name = input_file->lines[i][1];
		char **arguments = &(input_file->lines[i][2]);
		printf("Number of arguments: %s\n", input_file->lines[i][0]);
		printf("Name of the executable program: %s\n", program_name);

		// Store the program name and arguments
		processes[i].program_name = program_name;
		processes[i].arguments = arguments;
		clock_gettime(CLOCK_MONOTONIC, &processes[i].start); // Save the start time for the process

		// Create a new process
		int pid = fork();
		if (pid == 0) // Check if its the child process
		{
			// Check if we have to set an alarm
			// set_alarm(argc, argv);

			// Execute the program
			programExecution(program_name, arguments);
		}
		else if (pid < 0) // Handle error in the creation of the child process
		{
			perror("Error forking process");
			printf("Error forking process\n");
			exit(EXIT_ERROR); // Defined as 1
		}
		else // Parent process
		{
			processes[i].pid = pid; // Store the PID of the child process
			++noProcesses;			// Increment the number of processes running
		}
	}

	printf("Number of processes: %d\n", noProcesses);

	waitForProcesses(processes, input_file->len, 0, &noProcesses); // Wait for all the processes to finish
	// and get the exit status of each process and end times

	printProcesses(processes, input_file->len, output_file); // Print the results to the output file

	// Free the memory allocated for the processes array
	free(processes);

	fclose(output_file); // Close the output file & Destroy Input File
	input_file_destroy(input_file);
}