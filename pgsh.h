#ifndef PGSH_H
#define PGSH_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

// Define word SILENT in order not to print error messages
#ifndef SILENT
#define PRINT(x) puts(x)
#endif

// Enumerations

enum RedirectType {
	REDIN,		// Input redirection
	REDOUT,		// Output redirection
	REDOUTA		// Output redirection with append
};

// Enumerations

enum SpecialCmd {
	NOSP,		// Exit command
	SPEXIT,		// Change directory command
	SPCD		// No special command
};

// Function Prototypes

int pgsh(const char * history);
void intro();
FILE * load_history(const char * filename);
int append_command(FILE *historyPtr, const char * command);
FILE * new_history(const char * filename);
int handle_cmd_line(char * cmd_line);
int special_cmd_id(char * cmd);
int shell_chdir(char ** cmd);
int chdir_home(void);

#endif
