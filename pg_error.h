#ifndef PG_ERROR_H
#define PG_ERROR_H

#define ERROR_CODES 19	// Number of error codes

// Definition of ErrorType data type
enum ErrorType {
	EOK,
	EUNKNOWN,
	ENULL,
	ENOTOKEN,
	EPARSE,
	EEMPTSTR,
	EFORK,
	EEXEC,
	EPIPEF,
	EDUP,
	EWAIT,
	EFCHLD,
	EOPEN,
	EARG,
	ESYNTAX,
	EWPERM,
	ENOFILE,
	ECHDIR,
	ENOENV
};

// MAIN_FILE macro must be defined to the main source file, before including this one !
#ifdef MAIN_FILE
enum ErrorType pg_errno;
#else
extern enum ErrorType pg_errno; // definition in secondary source files
#endif

// Function Definitions //

void pg_perror(char * message);
const char * pg_strerror(int errcode);


#endif
