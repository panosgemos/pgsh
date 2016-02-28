#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "pg_file.h"
#include "pg_error.h"

/* Description: Redirects the input and or the output of the current process.		
 *
 * Arguments:	input: 		Input redirection file
 *				output:		Output redirection file
 *				append:		Append data(TRUE) or Overwrite them(FALSE).
 * 		
 * Returns:		- On success, backup file descriptor of the original standard output
 * 				- On failure, -1
 *						# ENULL: Both input and output are NULL
 *						# EOPEN: Error while opening files
 *						# EDUP : Error while duplicating file descriptors
 *
 *
 * Notes:		- If NULL is given to either input or output, the file descriptor
 *				  for that remains the same.
 *				- If the output file does not exit, it is created automatically.
 *				
 */

int redirect(char *input, char *output, int append) {
	int inFd, outFd;	// Input and output file descriptor
	int backOutFd;		// Backup stdout file descriptor
	int openFlag;
	
	if (input == NULL && output == NULL) {
		pg_errno = ENULL;
		return -1;
	}
	
	if (input != NULL) {
	
		// Open input file
		inFd = open(input, O_RDONLY);
		if(inFd==-1) {	
			pg_errno = EOPEN;
			return -1;
		}
	
		// Redirect input file
		if (dup2(inFd, STDIN_FILENO) == -1) {
			pg_errno = EDUP;
			return -1;
		}
		close(inFd);
	
	}
	
	// Backup original stdout file descriptor
	backOutFd = dup(STDOUT_FILENO);
	
	//write(backOutFd, "Not printed !", 13);
	if (backOutFd == -1) return -2;
	
	if (output != NULL) {
		
		// Configure open flag argument
		openFlag = O_WRONLY | O_CREAT;
		if (append) {
			openFlag |= O_APPEND; // Add append flag
		} else {
			openFlag |= O_TRUNC;  // Add truncate flag (delete previous contents)
		}
	
		// Open output file
		outFd = open(output, openFlag, 0644);
		if(outFd==-1) {
			return -1;
		}
	
		// Redirect output file
		if(dup2(outFd, STDOUT_FILENO) == -1) {
			return -2;
		}
		close(outFd);
	}
	
	return backOutFd;
	
}
