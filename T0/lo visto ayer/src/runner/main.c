#include <time.h>
#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include "../file_manager/manager.h"
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <wait.h>
#include "runner.h"


int main(int argc, char const *argv[])
{
	/*Lectura del input*/
	
	char *file_name = (char *)argv[1];
	InputFile *input_file = read_file(file_name);

	char *output_file_name = (char *)argv[2];
	FILE *output_file = fopen(output_file_name, "a");

	char amount = atoi(argv[3]);

	int file_len = input_file->len;

	printf("Number of lines: %d\n", file_len);
	printf("Max amount of processes at the same time: %d\n", amount);


	/*Mostramos el archivo de input en consola*/
	// create array of processes
	Process* allProcess[file_len];
	int noProcess = 0;

	for (int i = 0; i < file_len; ++i)
	{	

		int argc = atoi(input_file->lines[i][0]);
		printf("argc: %d ", argc);

		printf("%d ", atoi(input_file->lines[i][0]));
		printf("%s ", input_file->lines[i][1]);
		char *program_name = input_file->lines[i][1];
		char **arguments = &(input_file->lines[i][2]);
	
		// create process 
		allProcess[i] = (Process*)malloc(sizeof(Process));
		allProcess[i]->arguments = arguments;
		allProcess[i]->program_name = program_name;
		allProcess[i]->rejectSIG = false;
		allProcess[i]->num_arguments = argc;
		
		allProcess[i]->pid = -1;
		
		printf("\n");
	}
	
	bool allDone = false;
	int procesessDone = 0;
	// clock_t clockRunner = clock();
	while(procesessDone < file_len){
		if(noProcess > 0){
			int status_child;
			pid_t ret;
			double tiempo_tomado;
			clock_t end;
			ret = waitpid(0, &status_child, WNOHANG);
			if (ret > 0){
				printf("Process %d doneeeeeee\n", ret);
				noProcess--;
				procesessDone++;
				// Buscar el proceso padre
				for (int i = 0; i < file_len; i++){
					if(allProcess[i]->pid == ret){
						end = clock();
						allProcess[i]->end = end;
						printf("Start: %d\n", (int)(allProcess[i]->start));
						printf("End: %d\n", (int)(allProcess[i]->end));
						char* program_name = allProcess[i]->program_name;
						tiempo_tomado = (double)((allProcess[i]->end - allProcess[i]->start) / CLOCKS_PER_SEC);
						printf("Tiempo %d\n", tiempo_tomado);
						fprintf(output_file, "%s %f %d\n", program_name, tiempo_tomado, WEXITSTATUS(status_child));
						printf("Process %d done\n", i);
					}
				}
			}
		}
		if(noProcess < amount){
			for(int i = 0; i < file_len; i++){
				if (noProcess == amount) {
					break;
				}
				if (allProcess[i]->pid == -1){ 
					printf("Creating process %d\n", i);
					allProcess[i]->pid = fork();
					printf("PID: %d\n", allProcess[i]->pid);
					allProcess[i]->start = clock();
					if (allProcess[i]->pid == 0){
						// child process
						printf("Child process %d\n", i);
						char *args[allProcess[i]->num_arguments + 2];
						args[0] = allProcess[i]->program_name;
						for (int j = 0; j < allProcess[i]->num_arguments; j++){
							args[j + 1] = allProcess[i]->arguments[j];
						}
						args[allProcess[i]->num_arguments + 1] = NULL;
						if (i == 3){
							args[1] = "childtime";
							args[2] = 20;
							args[1] = "REDES";
							args[1] = 67;
							execvp(allProcess[i]->program_name, args);
						}
						else{
						if(execvp(allProcess[i]->program_name, args) == -1){
							printf("Error\n");
						};}
					}
					else{
						// parent process
						allProcess[i]->pid = getpid() + 1;
						noProcess++;
					}
					break;
				}
			}
		}
	}
	for (int i = 0; i < file_len; ++i)
	{
		free(allProcess[i]);
	}
	fclose(output_file);

	input_file_destroy(input_file);
}