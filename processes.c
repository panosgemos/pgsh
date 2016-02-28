#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h> // fork(), sleep()
#include <sys/wait.h> // wait function
#include <sys/types.h> 
#include <fcntl.h>
#include <signal.h>
#include "pg_string.h"
#include "pg_file.h"
#include "pg_error.h"
#include "processes.h"

// Creates a child process which will execute the function given as a parameter
// The function does not take any arguments nor return any value.
// The child should be waited by the wait_child function.

// Return Values: Returns 0 on success, -1 on failure.

pid_t create_child_func( void (*func)(void) ) {
	
	pid_t pid;
	pid=fork();	// Create child
	
	if (pid>=0) {	// fork success
		if (pid == 0) {	// Child code
			(*func)();	// Execute child's function
			exit(EXIT_SUCCESS);	// Child exited successfully
		} else {		// Parent code
			return 0;
		}
	} else {		// fork failure
		pg_errno = EFORK;
		return -1;
	}
	
}

/* Description: Creates a child which then executes the given command along with its 
 * 				arguments using the execl function
 *
 * Arguments:	cmd: full path of the command to be executed
 *				args: null terminated string array of the command's arguments
 * Returns:		0 on success, -1 on failure.
 * Notes:		First argument (args[0]) should contain the command name
 */
pid_t create_child_full( char *cmd, char **args ) {
	
	pid_t pid;
	pid=fork();	// Create child
	
	if (pid>=0) {	// fork success
		if (pid == 0) {	// Child code
			execv(cmd, args);	// Execute child's function
			exit(EXIT_FAILURE);	// Child exited due to execv failure
		} else {		// Parent code
			return 0;
		}
	} else {		// fork failure
		pg_errno = EFORK;
		return -1;
	}
	
}

/* Description: Creates a child which then executes the given command along with its 
 * 				arguments using the execv function
 *
 * Arguments:	args: null terminated string array of the command's arguments.
 *
 * Returns:		on success Child PID   	(Father)
 *				on failure
 *					# EFORK: Fork error   	(Father)
 *				
 * Notes:		First argument (args[0]) should contain the command name.
 *				This function may return twice. Once for the parent and 
 *				once for the child. If the child returns, an error occured
 *				in execvp, so the parent code should handle it accordingly.
 */
pid_t create_child( char **args ) {
	
	pid_t pid;
	pid=fork();	// Create child
	if (pid>=0) {	// fork success
		if (pid == 0) {	// Child code
			execvp(args[0], args);	// Execute child's function
			perror(args[0]);
			exit(EXIT_FAILURE);	// Child exited due to execv failure
		} else {		// Parent code
			return pid;
		}
	} else {		// fork failure
		pg_errno = EFORK;
		return -1;
	}
	
}

/* Description: Waits for a child process specified by its pid. 
 *
 * Arguments:	pid: Process ID of the child to be waited
 * Returns:		- On success, 0
 *				- On failure,
 *					# EFCHLD	 : 	Child could not execute the function or error 
 *						 			occured at execvp.
 *					# EWAIT 	 : 	No such child process.
 *					# EARG		 : 	Wrong process ID
 *					# EUNKNOWN	 :	Unknown error. Check errno
 *
 * Notes:		When used with create_child function as an input, and it returns -2
 *				for the child, then wait_child returns -3 indicating that the child
 *				had an error. In that case also, wait_child returns -1 to the father.
 */


int wait_child(pid_t pid) {
	
	int hasEnded=0;	// Boolean value that indicates the executing state of the child
	pid_t endPID=1;	// PID of waited child (set to 1 to enter while loop)
	int status;		// exit status of child process

	if (pid <= 0) {
		pg_errno = EARG;
		return -1;
	}
	
	// Needs while instead of if, in case the stopped child is terminated by a signal
	// Loops are performed only when continue signal is sent to the child process
	// after it was stopped.
	while (endPID>0) {	// prevents from entering with endPID==-1
	
		endPID = waitpid(pid, &status, WUNTRACED); // Return if stopped by a signal
			
		if (endPID == -1) {		// No such child
			pg_errno = EWAIT;
			return -1;
		}
	
	
		if(WIFEXITED(status)) {	// Exited naturally
			if(WEXITSTATUS(status) == EXIT_FAILURE) {
				pg_errno = EFCHLD;
				return -1; // child exited unsuccessfully
			}
			return 0;	// child exited successfully
		}
	
		// Process signaled 
		else if (WIFSIGNALED(status)) {	// Termination signal
			printf("Terminated by signal %d\n", WTERMSIG(status) );
			return 0;
		} else if (WIFSTOPPED(status)) {	// Stop signal
			printf("\nStopped by signal %d\n", WSTOPSIG(status) );
			// if the child process is stopped, wait for a SIGCONT signal
			endPID = waitpid(endPID, &status, WCONTINUED);
		  	if (WIFCONTINUED(status)) {		// Continue signal
				puts("\nProcess continued");
			}
		} else {
			pg_errno = EUNKNOWN;
			return -1;	// Unknown error occured
		}
	}

}

/* Description: Creates a child that executes the given command with redirected
 *				input and or output.		
 *
 * Arguments:	cmd:		Command to be executed
 * 				input: 		Filename of the input file
 *				output:		Filename of the output file
 *				append:		Append data(TRUE) or Overwrite them(FALSE).
 *
 * Returns:		- On success, pid of the created process
 * 				- On failure, -1
 *						# EFORK		: 	Fork error   	(Father)
 *						# EEXEC		: execvp error 	(Child)
 *
 *
 * Notes:		This function may return twice. Once for the parent and 
 *				once for the child. If the child returns, an error occured
 *				in execvp, so the parent code should handle it accordingly.			
 */

pid_t create_child_r(char **cmd, char *input, char *output, int append) {

	pid_t pid;
	pid=fork();	// Create child
	if (pid>=0) {	// fork success
		if (pid == 0) {	// Child code
			int redirect_status; // Stores the return value of redirect function
			
			// Redirect input and output before of the child
			redirect_status = redirect(input, output, append);
	
			if (redirect_status < 0) {	// Redirection failed
				pg_perror("redirect");
				exit(EXIT_FAILURE);	// Child exited due to redirection failure
			}
			execvp(cmd[0], cmd);	// Execute child's function
			perror(cmd[0]);		// Print error
			exit(EXIT_FAILURE);	// Child exited due to execvp failure
		} else {		// Parent code
			return pid;
		}
	} else {		// fork failure
		pg_errno = EFORK;
		return -1;
	}
}

/* Description: Given an array of tokenized commands, it sequentially connects
 * 				them with a pipe and then executes them, redirecting input and or
 *				output in truncating or appending mode. It is same as pipe_chain,
 *				but with the support of redirection.
 *
 * Arguments:	commands:	Commands array
 *				n:			Number of commands
 *				input:		First process's filename used for input redirection
 *				output:		Last process's filename used for output redirection
 *				append:		Appending or truncating mode
 *
 * Returns:		- On success,  0
 * 				- On failure, -1 and sets pg_errno to:
 *						# ENULL : 	NULL pointer passed as an argument
 *						# EARG	:	Wrong arguments passed (in==1 or out==0)
 *						# EFCHLD:	Execution of a child failed
 *						# EWAIT : 	Error while waiting children
 *						# EPIPE :	Error creating pipe
 *						
 *
 * Notes:		Children participating in the pipe are siblings.
 *				If both input and output are NULL, then standard input and
 *				standard output is used. In that case, 5th argument is ignored.
 *			
 */

int pipe_chain_r(char ***commands, int n, char *input, char *output, int append) {
	int inFd, outFd;	// Input and output file descriptor
	int backOutFd;		// Backup stdout file descriptor
	int openFlag;
	
	// Exceptions //
	if (input == NULL && output == NULL) {
		return pipe_chain(commands, n, STDIN_FILENO, STDOUT_FILENO);
	}
	
	// Open input file
	if (input != NULL) {
		inFd = open(input, O_RDONLY);
		if(inFd==-1) {	
			pg_errno = EOPEN;
			return -1;
		}
	
	} else {	// No input file defined, use standard input
		inFd = STDIN_FILENO;
	}
	
	// Open output file
	if (output != NULL) {
		
		// Configure open flag argument
		openFlag = O_WRONLY | O_CREAT;
		if (append) {
			openFlag |= O_APPEND; // Add append flag
		} else {
			openFlag |= O_TRUNC;  // Add truncate flag (delete previous contents)
		}

		outFd = open(output, openFlag, 0644);
		if(outFd==-1) {
			pg_errno = EOPEN;
			return -1;
		}
	} else {	// No output file defined, use standard output
		outFd = STDOUT_FILENO;
	}
	
	return pipe_chain(commands, n, inFd, outFd);
}

/* Description: Given an array of tokenized commands, it sequentially connects
 * 				them with a pipe and then executes them.
 *
 * Arguments:	commands:	Commands array
 *				n:			Number of commands
 *				inFd:			First process's fd used for input redirection
 *				outFd:		Last process's fd used for output redirection
 *
 * Returns:		- On success,  0
 * 				- On failure, -1 and sets pg_errno to:
 *						# ENULL : 	NULL pointer passed as an argument
 *						# EARG	:	Wrong arguments passed (in==1 or out==0)
 *						# EFCHLD:	Execution of a child failed
 *						# EWAIT : 	Error while waiting children
 *						# EPIPE :	Error creating pipe
 *						
 *
 * Notes:		Children participating in the pipe are siblings.			
 */
int pipe_chain(char ***commands, int n, int inFd, int outFd) {
	int i, j;
	pid_t pid;
	int pipeFd [2];
	int status;
	int waitFlag;
	int remain_proc;	// Remaining processes to be waited.
	
	// Exceptions //
	if (commands == NULL) {
		pg_errno = ENULL;
		return -1;
	}

	// The first process should get its input from the file file descriptor.
	// Check if the input descriptor is different that STDOUT_FILENO
	
	if (inFd == STDOUT_FILENO || outFd == STDIN_FILENO) {
		pg_errno = EARG;
		return -1;
	}
	
	remain_proc = n;	// Initialize number of remaining processes
	
	// Note the loop bound, we spawn here all, but the last stage of the pipeline.
	for (i = 0; i < n - 1; ++i) {
		
		if (pipe(pipeFd) == -1) {
			pg_errno = EPIPEF;
			return -1;
		}
			

		// f[1] is the write end of the pipe, we carry `in` from the prev iteration.
		if (spawn_proc(commands[i], inFd, pipeFd[1]) == -1) { // Error in spawn_proc
			if(pg_errno == ENULL) {			// Father error occured 
				pg_perror("spawn_proc");	// NULL pointer passed
				return -1;
			} else if (pg_errno == EEXEC) { // Child error occured
				fprintf(stderr, "%s: %s\n", "No such command", commands[i][0]);	
				exit(EXIT_FAILURE);
			} else {	// EDUP(child) or EFORK(father)
				pg_perror("spawn_proc");
				exit(EXIT_FAILURE);
			}
		}

      	// No need for the write end of the pipe, the child will write here.
		close (pipeFd[1]);

      	// Keep the read end of the pipe, the next child will read from there.
		inFd = pipeFd[0];
		
		// Loop for every created process so far and checking its return status
		// via wait (without hanging)
		for (j=0; j<i; ++j) {
	
			waitFlag = waitpid(-1, &status, WNOHANG);
	
			if (waitFlag == -1) {	// No child to wait or error in wait, check errno
				// Ignore ECHILD error because there might be no processes to wait
				if (errno != ECHILD) {
					pg_errno = EWAIT;
					return -1;
				} else {	
					// all children so far have finished
				}
				
			} else if (waitFlag == 0) {	// There are still children processes running
				break;
			} else {			// A process has finished
				if (WIFEXITED(status)) {	// Process exited normally
					// process finished with error (because exec failed)
					if (WEXITSTATUS(status)==EXIT_FAILURE) {
						pg_errno = EFCHLD;
						return -1;	// 
					}
				}
				--remain_proc;	// Decrement the number of processes by one
			}
		}
		
		// Check to see if any of the created processes had an error in executing
		// command (usually command did not exist) checking their status
		
	}
	
	// Execute the last stage using a child process.
	if ( (pid = fork()) == 0 ) {	// Child code
	
	// Last stage of the pipeline - set stdin be the read end of the previous pipe
    // and redirect output to the given file descriptor. 
		if (inFd != 0) {
			if ( dup2(inFd, 0) == -1) {
				pg_errno = EDUP;
				return -1;
			}
		}
		
		// Redirect last processes's output
		if (dup2(outFd, STDOUT_FILENO) ==-1) {
			pg_errno = EDUP;
			return -1;
		}
	
		execvp (commands[i][0], commands[i]);
		// execvp failed
		pg_errno = EEXEC;
		fprintf(stderr, "%s: %s\n", "No such command", commands[i][0]);	
		exit(EXIT_FAILURE);
	} else if (pid == -1) {
		pg_errno = EFORK;
		pg_perror("fork");
		exit(EXIT_FAILURE);
	}
	
	// Wait for all the remaining processes
	for (i=0 ; i< remain_proc + 1; ++i) {
		waitFlag = wait(&status);
		if(WIFEXITED(status)) {
			// Child failed mostly because command to execute does not exist
			if(WEXITSTATUS(status) == EXIT_FAILURE) {
				pg_errno = EFCHLD;
				return -1;
			}
		}
	}
	
	return 0;	// Function execution success
}

/* Description: Spawns a process with redirected standard input and output file
 *				descriptors and executes the given command.
 *
 * Arguments:	command:	Command to be executed in the created process
 *				in:			Input file descriptor
 *				out:		Output file descriptor
 *
 * Returns:		- On success,  pid of the created child
 * 				- On failure, -1 and sets pg_errno to:
 *						# ENULL: NULL pointer passed as an argument
 *						# EFORK: fork error   	
 *						# EEXEC: execvp error
 *						# EDUP : dup error
 *
 * Notes:		All errors above apart from ENULL, are better described by
 *				the errno and not pg_errno globar error variable.
 */
int spawn_proc (char **command, int in, int out) {
	pid_t pid;
	
	// Exceptions //
	if(command==NULL) {		// Command given is NULL
		pg_errno=ENULL;
		return -1;
	}
	
	// Create child process
	if ((pid = fork ()) == 0) {  // Child Code
  		
  		// If file descriptor is not STDIN, redirect it
		if (in != 0) {
			if ( dup2(in, 0) == -1 ) {
				pg_errno = EDUP;	// dup error
				return -1;
			}
			close(in);
		}
		
		// If file descriptor is not STDOUT, redirect it
		if (out != 1) {
			if ( dup2(out, 1) == -1 ) {
				pg_errno = EDUP;	// dup error
				return -1;
			}
			close(out);
		}
		
		execvp(command[0], command);	// Execute command
		
		// Error: execvp, returned -1
		pg_errno = EEXEC;
		return -1;
	} else if (pid == -1) {		// Fork error
		pg_errno = EFORK;
		return -1;
	}

	return pid;
}

/* Description: Prompts user to enter a command along with its arguments.
 *
 * Arguments:	void: Nothing
 * Returns:		- On success, the command line entered.
 * 				- On failure, NULL.
 *					
 * Notes:		None
 */

char *enter_command() {
	char *cmd_list=NULL;	// indicate that getline will allocate space
	size_t len=0;
	if ( getline(&cmd_list, &len, stdin) == -1 ) {
		return NULL;
	}
	
	return cmd_list;
}


/* 	TODO Handle children that return from exec with error
		 Create pipe_chine_r with redirection support and appending mode	

*/
