#include <stdio.h>
#include <stdlib.h>

/* Description: Frees all the space that the given 2D pointer allocates.
 *
 * Arguments:	pointer	:	Pointer which space is going to be freed.
 *				size	:	Number of pointers to data that the 2d pointer holds
 *
 * Returns:		void 	: nothing
 *
 * Notes:		This version of free2d used the size of the pointer array to loop
 */
void free2d(void ** pointer, int size) {
	
	int i;
	for(i=0; i<size; ++i) {
		free(pointer[i]);
		pointer[i] = NULL;
	}
	free(pointer[i]);
	pointer[i]=NULL;
	
	free(pointer);
}

/* Description: Frees all the space that the given 2D pointer allocates. Pointer
 *				given must be NULL terminated.
 *
 * Arguments:	pointer	:	Pointer which space is going to be freed.
 *
 * Returns:		void 	: nothing
 *
 * Notes:		This version of free2d stops freeing the 1D pointers till NULL is met
 */
void free2d_n(void ** pointer) {
	int i=0;
	
	// Free N-1 1D pointers
	while (pointer[i] != NULL) {
		free(pointer[i]);
		pointer[i] = NULL;
		++i;
	}
	
	// Free last 1D pointer
	free(pointer[i]);
	pointer[i] = NULL;
	
	// Free 2D pointer
	free(pointer);
}
