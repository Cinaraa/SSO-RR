#include "scheduler.h"

int main(int argc, char const *argv[])
{
	/*Lectura del input: ./testsT0/P2/inputs/in01.txt */
	char *file_name = (char *)argv[1];
	InputFile *input_file = read_file(file_name);

	char *output_file_name = (char *)argv[2];
	FILE *output_file = fopen(output_file_name, "a");

	int totalGroups = input_file->len - 1;
	bool isIdle = false;
	int fileLine;

	// Initialize the OS scheduler
	Scheduler *scheduler = InitializeScheduler(input_file);
	scheduler->processGroupArray = InitializeProcessGroup(scheduler, input_file, totalGroups);
	scheduler->totalGroups = totalGroups;

	Queue *activeGroups = (Queue *)calloc(1, sizeof(Queue));

	while (1)
	{
		// group 0 is in the line 1 of the input file -> i + 1
		// group 1 is in the line 2 of the input file
		for (int group = 0; group < scheduler->totalGroups; ++group) // Iterate over the groups,
		{
			fileLine = group + 1;
			ProcessGroup *processGroup = scheduler->processGroupArray[group];

			if (scheduler->time >= processGroup->TI) // Check if its time for the group to start
			{
				processGroup->state = "READY";

				if (isIdle)
				{
					report_IDLE(output_file, scheduler->idle_time);
					isIdle = false;
					scheduler->idle_time = 0;
				}

				add2Queue(activeGroups, processGroup);

				if (activeGroups->firstGroup == processGroup) // if its the first group in the queue
				{
					scheduler->nolines++;
					processGroup->state = "RUNNING";	 // we start the group
					processGroup->q = scheduler->qStart; // Assignate the initial q units of work

					int arg = 1;
					int total_n_args = countArgs(input_file, fileLine);

					// RunGroup(input_file, scheduler, fileLine, total_n_args, processGroup, output_file, scheduler->time, arg, 1, 0);
				}

				// Create the first process of the group
				// Process *process = CreateProcess(i, 0, processGroup->gid, CI, NH);

				scheduler->activeGroups++;
			}
		}

		// if (scheduler->activeGroups == 0) // If there are no active groups, OS goes to IDLE state
		// {
		// 	isIdle = true;
		// 	scheduler->time++;
		// 	scheduler->idle_time++;
		// 	continue;
		// }

		// If all groups have entered and finished, end the simulation
		if (scheduler->activeGroups == 0 && scheduler->time >= scheduler->qStart)
		{
			break;
		}

		if (scheduler->time > 50)
		{
			break; // tool to stop the infinite loop for now
		}
		scheduler->time++;
		break;
	}

	freeScheduler(scheduler, scheduler->totalGroups);
	freeQueue(activeGroups);
	input_file_destroy(input_file);
	fclose(output_file);
}