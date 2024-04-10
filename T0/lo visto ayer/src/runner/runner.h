#include <stdbool.h> 
#include "../file_manager/manager.h"
#include <time.h>


typedef struct Process
{
    int pid;               // Process ID
    char *program_name;    // Name of the program
    char **arguments;      // Arguments to the program
    int num_arguments;     // Number of arguments
    clock_t start; // Start time
    clock_t end;   // End time
    int status;            // Exit status
    bool rejectSIG;         // Reject signal
} Process;
