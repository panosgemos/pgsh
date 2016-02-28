/* The pgmank shell is an elementary shell that just executes the commands given to
 * it. It also stores the history of the command to a specified file.
 *
 * Technically, it creates a child process for every command entered. Then using exec
 * it substitutes the child process with a specific program which then waits till its
 * termination. It prints specific messages if terminated naturally or by a signal.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h> // fork(), sleep()
#include <sys/wait.h> // wait()
#include <sys/types.h> 
#include <signal.h>
#include <string.h>
#include "pg_error.h"
#include "processes.h"	// create_child(), wait_child()
#include "pg_string.h"	// astrcat()
#include "pgsh.h"

// Static Function Prototypes //
static int check_redirect_symb(char * string);
static char * pipe_redirect_filename(char **pipe_tokens, int tokens, enum 
	RedirectType redirection);

// Functions //

/* Description: All the functionallity of the pgsh shell is implemented in this 
 * 				function.
 *	
 * Arguments:		history: Filename of the current session history
 * 
 * Return Value:	void: Nothing
 *
 * Possible Errors: Not yet discovered
 *
 * Notes:			None
 *
 */
int pgsh(const char *history) {
	
	// Variables //
	
	int i;				// Counter
	char *cmd_line;		// Whole command line
	char *tmpstr;
	FILE *historyPtr;	// Pointer to history file
	
	// File Configurations //
	
	historyPtr = load_history(history);
	
	// History Load error checking
	switch(pg_errno) {
		case ENULL:
			perror("load_history");
			exit(EXIT_FAILURE);
		case ENOFILE:
			fprintf(stderr, "History file '%s' not found\n", history);
			exit(EXIT_FAILURE);
		case EPERM:
			fprintf(stderr, "No permissions to write to file '%s'\n", history);
			exit(EXIT_FAILURE);
		case EOPEN:
			perror("fopen");
			exit(EXIT_FAILURE);
	}
	
	// Functional Code //
	
	intro();	// Print introduction screen
	
	do {
		printf("%s", "pgsh:$ ");
		cmd_line = enter_command();				// Read command from terminal
		
		// Tradeoff bug when executing with ctrd+d instead of enter
		// will produce command history on the same line
		// cmd_line[strlen(cmd_line)-1] = '\0';	// Substitute '\n' with '\0'
		
		// If user just pressed enter, prompt new shell line
		if (cmd_line[0] == '\n') {
			continue;
		}
		
		
		
		append_command(historyPtr, cmd_line);	// Store command to history
		
		// Command handling (analyze and execution) //
		
		// Clean cmd line from empty pair of double quotes
		tmpstr = cmd_line;	// store old string pointer for later memory free
		cmd_line = strepclean(cmd_line, '"');
		
		// Strepclean allocated new space for the clean string
		if (tmpstr != cmd_line) {
			free(tmpstr);	// free memory
		}	// else strepclean return the same string because it was clean
		
		// Handle entered command line
		if ( handle_cmd_line(cmd_line) == SPEXIT) {
			break;
		}
		
		
		free(cmd_line);
		
	} while(1) ;
	
	free(cmd_line);
	
	fclose(historyPtr);
	puts("Exited pgsh shell");
	
}

/* Description: Identifies the type of special command. 
 *					# exit	(exits pg shell)
 *					# cd	(changes current working directory)
 *	
 * Arguments:		cmd: Command's first argument
 * 
 * Return Value:	- on success,  0
 *					- on failure, -1 and sets pg_errno to
 *						# ENULL 	: If NULL pointer was given as an argument
 *						# EMPTSTR	: If empty string was passed
 *
 * Notes:			None
 *
 */
 
int special_cmd_id(char * cmd) {
	
	// Exceptions //
	
	if (cmd == NULL) {
		pg_errno = ENULL;
		return -1;
	}
	
	if (*cmd == '\0') {
		pg_errno = EEMPTSTR;
		return -1;
	}
	
	if (strstr("exit", cmd) != NULL) {
		return SPEXIT;
	} else if (strstr("cd", cmd) != NULL) {
		return SPCD;
	} else {	// No special command entered
		return NOSP;
	}
}

/* Description: Prints the introductory screen of the pgsh shell.
 *	
 * Arguments:		void: None
 * 
 * Return Value:	void: No return value
 *
 * Notes:			None
 */ 

void intro(void) {
	puts("* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *");
	puts("* ________________Welcome to pgsh Ver 0.0.2________________ *");
	puts("* Type your commands like in your terminal and have fun !!! *");
	puts("* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *");
	printf("%s", "\n\n");
}

/* Description: Loads the history file of the current session.
 *	
 * Arguments:		filename: Filename of the current session history
 * 
 * Return Value:	- on success, returns pointer to the opened history file
 *						and sets errnum to zero
 *					- on failure, returns NULL and sets pg_errno to
 *						# ENULL 	: If NULL pointer was given as an argument
 *						# ENOFILE	: If file does not exist
 *						# EPERM		: If write permission is denied
 *						# EOPEN		: Error in fopen
 *
 * Possible Errors:	- Error while opening file using fopen
 *
 * Notes:			In case the file does not exist, function new_history 
 *					can be called in order to create a new history file.
 *
 */ 
FILE * load_history(const char * filename) {
	
	FILE *historyPtr;
	
	if (filename == NULL) {
		pg_errno = ENULL;
		return NULL;
	}
	
	if ( access(filename, F_OK) == 0 ) {
		if ( access(filename, W_OK) == 0 ) {
			historyPtr = fopen(filename, "a");
			if (historyPtr == NULL) {
				pg_errno = EOPEN;
				exit(EXIT_FAILURE);
			}
		} else {
			pg_errno = EPERM;	// Write permission denied
			return NULL;
		}
	
	} else {
		pg_errno = ENOFILE; 		// File does not exist
		return NULL;
	}
	
	pg_errno = EOK;				// Load success
	
	return historyPtr;		
}

/* Description: 	Creates a new history file.
 *	
 * Arguments:		filename: Filename of the history file to be created
 * 
 * Return Value:	- on success, returns history FILE pointer
 *					- on failure, returns NULL
 
 *
 * Possible Errors:	1) Filename given is NULL
 *					2) Error creating file using fopen function
 *
 * Notes:			Used along with load_history result's output in order
 *					to create a new history if it does not already exist.
 *					Function does not close the file by its one. It is 
 *					the programmer's responsibility.
 *
 */ 
FILE * new_history(const char * filename) {
	FILE *fPtr;
	
	// Sanity Check: NULL argument
	if(filename == NULL) {
		fprintf(stderr, "%s\n", "Filename cannot be NULL");
		return NULL;
	}
	
	// Open file for append mode
	fPtr = fopen(filename, "a");
	if(fPtr == NULL) {
		perror("fopen");	// fopen failed
		return NULL;
	}
	
	return fPtr;
}

/* Description: 	Appends command entered to the history file.
 *	
 * Arguments:		historyPtr: FILE pointer of the current session history
 *					command:	Command to be appended to the history file
 * 
 * Return Value:	- on success, returns  0
 *					- on failure, returns -1
 *
 *
 * Possible Errors:	1) FILE pointer given is NULL
 *					2) Error appending contents to history file
 *
 * Notes:			In case the file does not exist, function new_history 
 *					can be called in order to create a new history file.
 *
 */ 
int append_command(FILE *historyPtr, const char * command) {

	int error_indicator;
	
	// Sanity Checks: NULL arguments
	if (historyPtr == NULL) {
		fprintf(stderr, "%s\n", "FILE pointer cannot be NULL");
	}
	
	if (command == NULL) {
		fprintf(stderr, "%s\n", "Command cannot be null");
	}
	
	// Append typed command to history
	error_indicator=fprintf(historyPtr, "%s", command);
	
	if (error_indicator < 0) {	// Error occured while appending line to history
		perror("fprintf");
		return -1;
	}
	
	return 0;
	
}


/* Description: 	Handles the command line typed by the user.
 *					It is responsible for detecting pipeline commands, input and 
 *					output redirections and reporting syntax or execution errors.
 *					It also handles the special commands exit and cd.
 *	
 * Arguments:		cmd_line: Command line to be executed
 * 
 * Return Value:	- on success, returns  >= 0 :
 *						NOSP 	: Normal command executed
 *						SPEXIT 	: Exit command entered
 *						SPCD	: Change working directory command entered
 *					- on failure, returns -1
 *
 * Notes:			It prints appropriate messages to stderr in case of an error.
 *
 */ 
int handle_cmd_line(char * cmd_line) {
	
	int i;
	int pipe_token_cnt;
	int cmd_tkn_cnt;
	int append=1;			// Redirection append mode
	pid_t childPid;
	int skip;			// Number of tokens skipped from the end to exclude
						// redirection symbols and filenames
	
	char *input=NULL;			// Redirection input filename
	char *output=NULL;			// Redirection output filename
	char **pipe_tokens;		// Command line splitted by pipes
	char **redirect_cmd;	// Commands with redirection
	char ***pipe_commands;	// Command line splitted by pipes and arguments
	
	
	// Count the number of commands participating in a pipe connection
	pipe_token_cnt = ctoken_counter(cmd_line, "|");
	
	if (pipe_token_cnt > 1) {	// Command entered has a pipe
		pipe_tokens = ctokenize(cmd_line, "|");
		
		// Allocate space for the pipe commands
		
		pipe_commands = (char***)malloc( pipe_token_cnt * sizeof(char**));
		if (pipe_commands == NULL) {
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		
		// Check for input and output redirection in the the pipeline commands //
		
		// check input redirection
		input = pipe_redirect_filename(pipe_tokens, pipe_token_cnt, REDIN);
		
		if (input == NULL) { // No redirection exists or error occurred
			switch (pg_errno) { 
				case EOK: // No redirection input without append found
					break;
				case EARG:	// Wrong redirect type
					fprintf(stderr, "%s\n", "No such redirect error");
					return -1;
				case ESYNTAX:
					return -1;
			}
		}
		
		output = pipe_redirect_filename(pipe_tokens, pipe_token_cnt, REDOUTA);
		if (output == NULL) {
			
			switch (pg_errno) {
				case EOK: // No input redirection with append
					// check redirection without append
					output = pipe_redirect_filename(pipe_tokens, pipe_token_cnt,
						REDOUT);
					append = 0; 	// Set append to zero
					if (output == NULL) { // No redirection exists or error occurred
						switch (pg_errno) {
							case EOK: // No redirection input without append found
								break;
							case EARG:	// Wrong redirect type
								fprintf(stderr, "%s\n", "No such RedirectType");
								return -1;
							case ESYNTAX:
								fprintf(stderr, "%s\n", pg_strerror(ESYNTAX));
								return -1;
						}
					}
					break;
				case EARG:
					fprintf(stderr, "%s\n", "No such redirect error");
					return -1;
				case ESYNTAX:
					fprintf(stderr, "%s\n", pg_strerror(ESYNTAX));
					return -1;
			}
		}
		
		// Tokenize pipe subtokens to commands and arguments
		
		for(i=0; i<pipe_token_cnt; ++i) {
			pipe_commands[i] = ctokenize( pipe_tokens[i], " " );
		}
		
		free2d_n(pipe_tokens);
		
		// Check for special command
		
		switch(special_cmd_id(pipe_commands[0][0])) {
			case SPEXIT:
				return SPEXIT;
			case SPCD:
				if ( shell_chdir(pipe_commands[0]) == -1 ) {
					perror("cd");
					return -1;
				}
				return SPCD;
			case -1:
				pg_perror("special_cmd_id");
				return -1;
		}
		
		// Execute commands in the pipe
		if (pipe_chain_r(pipe_commands, pipe_token_cnt, input, output, append)==-1){
			pg_perror("pipe_chain_r");
			return -1;
		}
		
		// Memory Clean Up //
		
		if (input!=NULL) {
			free(input);
		}
		if (output!=NULL) {
			free(output);
		}
		
		// Free space for each pipe command
		for(i=0; i<pipe_token_cnt; ++i) {
			free2d_n(pipe_commands[i]);
		}
		free(pipe_commands);
		
	} else {// No pipes, check just for redirection though
		
		cmd_tkn_cnt = ctoken_counter(cmd_line, " ");
		if ( cmd_tkn_cnt < 1) {
			fprintf(stderr, "%s\n", "No tokens in command");
			return -1;
		}
		redirect_cmd = ctokenize(cmd_line, " ");
		
		// Check for special command
		
		switch(special_cmd_id(redirect_cmd[0])) {
			case SPEXIT:
				return SPEXIT;
			case SPCD:
				if ( shell_chdir(redirect_cmd) == -1) {
					perror("cd");
					return -1;
				}
				return SPCD;
			case -1:
				pg_perror("special_cmd_id");
				return -1;
		}
		
		// If there are more or than four tokens
		// ( cmd args redir_symb fname redir_symb fname ) check for a second redirect
		// symbol in the third from last position
		if (cmd_tkn_cnt >= 5) {
			skip = 2;
		}
		
		// If there are more than two tokens (command args redirect_symbol filename)
		// check if the one before the last is a redirect symbol
		else if (cmd_tkn_cnt >= 3) {
			skip = 0;
		} else {	// No redirection (two or less tokens)
			skip = -1;
		}
	
		while (skip >= 0) {	// If there is dual or single redirection
							// loops twice for dual, one for single
			
			// Check the kind of redirection
			switch ( check_redirect_symb( redirect_cmd[cmd_tkn_cnt - 2 - skip]) ) {
				case REDIN:
					input = redirect_cmd[cmd_tkn_cnt - 1 - skip];
					break;
				case REDOUTA:
					output = redirect_cmd[cmd_tkn_cnt - 1 - skip];
					break;
				case REDOUT:
					output = redirect_cmd[cmd_tkn_cnt - 1 - skip];
					append = 0;	// Set append to zero
					break;
				default: // No redirection found on this position, move to next one
					skip-=2;
					continue;
			}
			
			// Remove redirection part from the command part by setting the
			// redirection symbols as NULL
			redirect_cmd[cmd_tkn_cnt - 2 - skip] = (char*)NULL;
			
			skip-=2;
			
		}
		
		
	// NOTE : If there are two tokens, then there is no way for redirection to exist
		
		// No redirection, just execute command
		if (input == NULL && output == NULL) {
			childPid = create_child(redirect_cmd);	
		} else { // Command with redirection
			childPid = create_child_r(redirect_cmd, input, output, append);	
		}
		
		wait_child(childPid);	// Wait for child to execute command
		
		switch(pg_errno) {
			case EFCHLD:
				//fprintf(stderr, "Failed to execute command : '%s'\n", redirect_cmd[0]);
				pg_errno = EOK;		// Reset pg_errno
				return -1;
			case EWAIT:
				perror("wait");
				pg_errno = EOK;		// Reset pg_errno
				return -1;
			case EARG:
				fprintf(stderr, "%s\n", "wait_child: Process ID cannot be negative");
				pg_errno = EOK;		// Reset pg_errno
				return -1;
			case EUNKNOWN:
				pg_perror("wait_child");
				pg_errno = EOK;		// Reset pg_errno
				return -1;
		}
		
		free2d_n(redirect_cmd);
		
	}
	return NOSP;	// Command line to handle executed successfully
}

/* Description: 	Searches for an input or output redirection, in appending mode
 *					or not in the given pipe tokens. If one is found, its filename
 *					is returned, NULL otherwise. It also filters the pipe token
 *					that contain the redirection file in the 
 *	
 * Arguments:		pipe_tokens : Tokens separated from a command containing pipes
 *					tokens		: Number of tokens
 *					redirection : Type of redirection to be searched
 *						# REDIN 	= Input redirection without append
 * 						# REDOUT	= Output redirection without append
 * 						# REDOUTA 	= Output redirection with append
 *
 * Return Value:	- on success, returns redirection filename
 *					- on failure, returns NULL
 *
 * Possible Errors: 	# EARG   	: No such RedirectType
 *						# EOK	 	: No error, but no redirection found
 *						# ESYNTAX	: More than one redirection symbols found in 
 *									  one command.
 *
 * Notes:			None
 *
 */ 
static char * pipe_redirect_filename(char **pipe_tokens, int tokens, enum 
	RedirectType redirection) {
	
	char **redirect_tkn;	// Holds the tokens left and right of the redirection
							// symbol (i.e command left and filename right
	char **temp2d;			// Used to temporary store pointer to be freed
	char *redir_fname;		// Redirection filename
	char redirSymbol[3];	// String containing the redirection symbol
	int index;		// Index of the pipe token that contain the redirection symbol
	
	// 
	switch (redirection) {
		case REDIN:
			strcpy(redirSymbol, "<");
			index=0;
			break;
		case REDOUT:
			strcpy(redirSymbol, ">");
			index=tokens-1;
			break;
		case REDOUTA:
			strcpy(redirSymbol, ">>");
			index=tokens-1;
			break;
		default:
			pg_errno = EARG;
			return NULL;
	}
	
	// Check if there is an output redirection without append
		switch ( stoken_counter(pipe_tokens[index], redirSymbol) ) {
			case 1:		// Does not exist
				pg_errno = EOK;
				return NULL;
			case 2:		// Exists
			
				// Split command and redirection filename
				redirect_tkn = stokenize(pipe_tokens[index], redirSymbol);
				
				// Place only the command into the pipe token where previously 
				// existed both command and redirection filename
				// Place '\0' just after the command, skipping the redirection
				// filename, so that only the command is stored in pipe_tokens[index]
				pipe_tokens[index][strlen(redirect_tkn[0])] = '\0';
				
				break;
			case -1:	// Null pointer passed to stoken_counter
				pg_perror("stoken_counter");
				pg_errno = EOK; 	// Reset pg_errno
				return NULL;
			default:	// More than 3 tokens found
				pg_errno = ESYNTAX;
				return NULL;
		}
	
	// Clear redirection filename from extra spaces
	
	temp2d = redirect_tkn;
	redirect_tkn = ctokenize(redirect_tkn[1], " ");
	
	free2d_n(temp2d);
	
	redir_fname = redirect_tkn[0];
	
	free(redirect_tkn);	// Free only the 2D pointer, not the 1D inside
						// because it is used by redir_fname
	
	return redir_fname;
	
}

/* Description: 	Checks the given string if it contains a redirection symbol and
 *					returns the enum type of that symbol
 *	
 * Arguments:		string : Tokens separated from a command containing pipes
 *						
 *
 * Return Value:	- on success, returns redirection enum type
 *					
 *					 	# REDIN 	= Input redirection without append
 * 						# REDOUT	= Output redirection without append
 * 						# REDOUTA 	= Output redirection with append
 *	
 *					- on failure, returns -1
 *						# Redirection symbol not found
 *
 * Notes:			None
 *
 */ 
static int check_redirect_symb(char * string) {
	if (strstr (string, "<")) {
		return REDIN;
	} 
	
	if (strstr (string, ">>")) {
		return REDOUTA;
	} else if (strstr (string, ">")) {
		return REDOUT;
	} 
	
	return -1;
}

/* Description: It changes the current shell's working directory
 *	
 * Arguments:		cmd: Command with arguments
 * 
 * Return Value:	- on success,  0
 *					- on failure, -1 and sets pg_errno to :
 *						# ENULL 	: If NULL pointer was given as an argument
 *						# ENOENV 	: 	No such environment variable
 *						# ECHDIR	: 	Could not change directory. Check errno
 *										for more details.
 *
 * Possible Errors:	- Error while opening file using fopen
 *
 * Notes:			In case the file does not exist, function new_history 
 *					can be called in order to create a new history file.
 *
 */ 
int shell_chdir(char ** cmd) {
	
	// Exceptions //
	
	// NULL argument
	if (cmd == NULL) {
		pg_errno = ENULL;
		return -1;
	}
	
	char * home;
	
	// At least one token
	if (cmd[0] != NULL) {
		
		// There are two tokens (command and path)
		if (cmd[1] != NULL) {
			
			// Change to home directory
			if (strstr("~", cmd[1]) != NULL) {
				return chdir_home() == 0 ? 0 : -1;
			}
		
			if (chdir(cmd[1]) == -1) {
				pg_errno = ECHDIR;
				return -1;
			}
			
		
		// If it is only one token, (i.e cd) change to the home directory
		} else {
			return chdir_home() == 0 ? 0 : -1;
		}
	}
	return 0;
}

/* Description: Makes home, the current working directory
 *	
 * Arguments:		void: Nothing
 * 
 * Return Value:	- on success,  0
 *					- on failure, -1 and sets pg_errno to :
 *						# ENOENV 	: 	No such environment variable
 *						# ECHDIR	: 	Could not change directory. Check errno
 *										for more details.
 *
 * Notes:			None
 *
 */ 
int chdir_home(void) {
	// Get HOME path
	char *home = getenv("HOME");	
	if (home == NULL) {
		fprintf(stderr, "HOME environment variable is not set\n");
		pg_errno = ENOENV;
		return -1;
	}
			
	// Change directory to home
	if (chdir(home) == -1) {
		pg_errno = ECHDIR;
		return -1;
	}
	
	return 0;
}
