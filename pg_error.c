#include <stdio.h>
#include "pg_error.h"

const char * const error_messages[ERROR_CODES] = {
	"Everything is OK",						// EOK 		0
	"Unkown error",							// EUNKNOWN 1
	"NULL pointer passed to function", 		// ENULL 	2
	"No tokens found",						// ENOTOKEN 3
	"String parse error",					// EPARSE 	4
	"Empty string",							// EEMPTSTR 5
	"fork error, check errno for details",	// EFORK	6
	"exec error, check errno for details", 	// EEXEC	7
	"pipe error, check errno for details", 	// EPIPEF	8
	"dup error, check errno for details", 	// EDUP		9
	"wait error, check errno for details", 	// EWAIT	10
	"Child execution failed",				// EFCHLD	11
	"Cannot open file",						// EOPEN	12
	"Wrong arguments passed",				// EARG		13
	"Wrong syntax",							// ESYNTAX	14
	"Write permission denied"				// EWPERM	15
	"File does not exist"					// ENOFILE	16
	"Cannot change directory"				// ECHDIR	17
	"No such environment variable"			// ENOENV	18
};


/* Description: Prints a user custom message along with the most recent error
 * 				message corresponding to the last value of pg_errno
 *
 * Arguments:	messsage:	Custom message
 *
 * Returns:		void: nothing
 *
 * Possible errors:		# EINVAL: NULL pointer passed to str argument
 */
void pg_perror(char * message) {

	if (message == NULL) {
		pg_errno = ENULL;
		return;
	}
	
	if (pg_errno >= ERROR_CODES) {
		pg_errno = EUNKNOWN;
	}

	fprintf(stderr, "%s: %s\n", message, error_messages[pg_errno]);
}

/* Description: Constructs a string containing the given error codes
 *				corresponding message
 *
 * Arguments:	errcode:	Error code
 *				
 * Returns:		- On success, a string containing the error message.
 * 				- On failure, NULL.
 */
const char * pg_strerror(int errcode) {

	if (errcode >= ERROR_CODES) {
		errcode = EUNKNOWN;
	}

	return error_messages[errcode];
}

//__DEPRICATED__//
/* Choosed to define the array globaly in order to take advantage of the const
 * keyword which would prevent the initialization of the array after it is defined
 * such way in this function.
 */
/* Description: Given an error number, it returns the string represantation
 *				of the error message. On the first call it initializes the 
 *				error messages
 *
 * Arguments:	errnumber:	Error code
 *				
 * Returns:		- On success, a string containing the error message.
 *
 * Note:		It does not check for validity of the errnumber. Programmer
 *				is responsible to pass valid errnumber to this function.
 *				The scope of this function is limited only to this file.
 */
static char * error_message(int errnumber) {
	
	// String array containing the error messages that correspond to the error codes
	char static *error_messages[ERROR_CODES] ;
	static int initialized=0;
	
	if (!initialized) {		// Initialize only on the first call
		// Declaration of the error code messages
		error_messages[EOK] = "Everything is OK";
		error_messages[EUNKNOWN] = "Unkown Error";
		error_messages[ENULL] = "NULL pointer passed to function";
	}
}

// TODO: error_message() has not be tested
