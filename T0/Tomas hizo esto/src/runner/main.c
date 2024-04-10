#include "runner.h"

int main(int argc, char const *argv[])
{	

	// First check if the number of arguments is correct
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
	while(1) {
		for (int )
	}
		for (int i = 0; i < input_file->len; ++i)
		{	
			printf("Line : %d, noProcesses : %d\n", i, noProcesses);
		// // Verify if the number of processes is less than the amount specified
		if (noProcesses >= amount)
		{
			printf("Waiting for processes %d\n", noProcesses);
			// In case that the amount of processes is greater than the specified amount
			// Wait for all the processes to finish
			waitpid();
			--noProcesses;
		}

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
			printf("Child process %d\n", getpid());
			printf("noProcesses %d\n", noProcesses);	
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
			printf("else: noProcesses %d\n", noProcesses);
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