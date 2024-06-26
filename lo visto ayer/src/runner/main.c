#include <time.h>
#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include "../file_manager/manager.h"
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <wait.h>
#include "runner.h"

bool alarm1 = false;
int times_up = 0;


void alarm_handler(int signum) {
	printf("Alarm triggered\n");
	printf("%d\n", times_up);
	alarm1 = true;

	// You can add any action you want to perform when the alarm triggers
	
}


int main(int argc, char const *argv[])
{	
	signal(SIGALRM, alarm_handler);
	/*Lectura del input*/
	float max_time = -1.;
	if (argc == 5){
		max_time = (float) atoi(argv[4]);
		alarm(max_time);
	}
	printf("Max time: %f\n", max_time);
	char *file_name = (char *)argv[1];
	InputFile *input_file = read_file(file_name);

	char *output_file_name = (char *)argv[2];
	FILE *output_file = fopen(output_file_name, "a");

	char amount = atoi(argv[3]);

	int file_len = input_file->len;

	// printf("Number of lines: %d\n", file_len);
	// printf("Max amount of processes at the same time: %d\n", amount);


	/*Mostramos el archivo de input en consola*/
	// create array of processes
	Process* allProcess[file_len];
	int noProcess = 0;
	for (int i = 0; i < file_len; ++i)
	{	
		int argc = atoi(input_file->lines[i][0]);
		// printf("argc: %d ", argc);

		// printf("%d ", atoi(input_file->lines[i][0]));
		// printf("%s ", input_file->lines[i][1]);
		char *program_name = input_file->lines[i][1];
		char **arguments = &(input_file->lines[i][2]);
	
		// create process 
		allProcess[i] = (Process*)malloc(sizeof(Process));
		allProcess[i]->order = i;
		allProcess[i]->arguments = arguments;
		allProcess[i]->program_name = program_name;
		allProcess[i]->rejectSIG = false;
		allProcess[i]->num_arguments = argc;
		
		allProcess[i]->pid = -1;
		
		// printf("\n");
	}
	
	bool allDone = false;
	int procesessDone = 0;
	Process** Process_in_CPU = calloc(amount, sizeof(Process*));

	clock_t clockRunner = clock();
	int timer_flag = 0;
	while(procesessDone < file_len){
		if (alarm1){
			if(times_up == 0){
			
				printf("Time's up\n");
				for (int m = 0; m < amount; m++){
					if(Process_in_CPU[m]){
						Process_in_CPU[m]->status = 2;
						clock_t end = clock();
						Process_in_CPU[m]->time_taken = (double)((end - Process_in_CPU[m]->start) / CLOCKS_PER_SEC);
						kill(Process_in_CPU[m]->pid, SIGINT);
						Process_in_CPU[m] = 0;
					}
				}
				alarm(10);
				times_up = 2;
				alarm1 = false;
				printf("second alarm set\n");
			}
			else if(times_up == 2){
				printf("Time's up for real!\n");
				for (int m = 0; m < amount; m++){
					if(Process_in_CPU[m]){
						Process_in_CPU[m]->status = 2;
						clock_t end = clock();
						Process_in_CPU[m]->time_taken = (double)((end - Process_in_CPU[m]->start) / CLOCKS_PER_SEC);
						kill(Process_in_CPU[m]->pid, SIGTERM);
						Process_in_CPU[m] = 0;
					}
				}

			}
		}
		

			
			
		
		
		// 	timer_flag = 1;
		// 	break;
		// }
		// if(max_time != -1){
		// 	clock_t still_runnin = clock();
		// 	printf("Still running %f\n", (double)(still_runnin - clockRunner) / CLOCKS_PER_SEC);
		// 	double time_spent = (double)(still_runnin - clockRunner) / CLOCKS_PER_SEC;
		// 	if(time_spent > max_time){
		// 		printf("Max time reached\n");
		// 		for (int m = 0; m < amount; m++){
		// 			if(Process_in_CPU[m]){
		// 				kill(Process_in_CPU[m]->pid, SIGTERM);
		// 				Process_in_CPU[m] = 0;
		// 			}
		// 		}
		// 	}
		// }
		if(noProcess > 0){
			int status_child;
			pid_t ret;
			double tiempo_tomado;
			clock_t end;
			ret = waitpid(0, &status_child, WNOHANG);
			if (ret > 0){
				// printf("Process %d doneeeeeee\n", ret);
				noProcess--;
				procesessDone++;
				// Buscar el proceso padre
				for (int i_done = 0; i_done < amount; i_done++){
					if(Process_in_CPU[i_done] != 0 && Process_in_CPU[i_done]->pid == ret){
						end = clock();
						Process_in_CPU[i_done]->time_taken = (double)((end - Process_in_CPU[i_done]->start) / CLOCKS_PER_SEC);
						Process_in_CPU[i_done]->status = WEXITSTATUS(status_child);
						Process_in_CPU[i_done] = 0;
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
					// printf("Creating process %d\n", i);
					allProcess[i]->pid = fork();
					// printf("Process %d created\n", allProcess[i]->pid);
					allProcess[i]->start = clock();
					if (allProcess[i]->pid == 0){
						// child process
						// printf("Child process %d\n", i);
						char *args[allProcess[i]->num_arguments + 1];
						// printf("Arguments: %d\n", allProcess[i]->num_arguments);
						for (int j = 0; j < allProcess[i]->num_arguments; j++){
							args[j] = allProcess[i]->arguments[j];
						// printf("Argument: %s\n", allProcess[i]->arguments[j]);

						}
						args[allProcess[i]->num_arguments] = NULL;

						if(execvp(allProcess[i]->program_name, args) == -1){
							printf("Error\n");
						};
					}
					else{
						// parent process
						// allProcess[i]->pid = getpid() + 1;
						noProcess++;
						// printf(" child %d has started at  %d \n",  allProcess[i]->pid, (int)(allProcess[i]->start / CLOCKS_PER_SEC));
						// printf(" child %d program %s \n",  allProcess[i]->pid, allProcess[i]->program_name);
						for (int p = 0; p < amount; p++){
							if(!Process_in_CPU[p]){
								Process_in_CPU[p] = allProcess[i];
								break;
							};
						}
					}
				}
			}
		}
		
	}
	for (int process = 0; process < file_len; process++){
		char* program_name = allProcess[process]->program_name;
		int tiempo_tomado = (int) allProcess[process]->time_taken;
		// printf("Tiempo tomado por proceso: %d, %d\n",allProcess[process]->pid ,tiempo_tomado);
		fprintf(output_file, "%s,%d,%d\n", program_name, tiempo_tomado, allProcess[process]->status);
		// printf("Process %d written \n", allProcess[process]->pid);
	}
	for (int i = 0; i < file_len; ++i)
	{
		free(allProcess[i]);
	}
	free(Process_in_CPU);
	fclose(output_file);

	input_file_destroy(input_file);
}