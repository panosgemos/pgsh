#include <stdio.h>
#include <stdlib.h>
#define MAIN_FILE
#include "pg_error.h"
#include "pgsh.h"

int main(void) {
		
		FILE * historyPtr;
		
		// If history.dat does not exist, create a new one
		if ( access("history.txt", F_OK) != 0 ) {
			historyPtr = new_history("history.txt");
			fclose(historyPtr);
		}
		
		pgsh("history.txt");
}
