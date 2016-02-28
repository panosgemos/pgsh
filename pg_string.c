/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Author: Panos K.G 
 *
 * Description:
 * This source has some auxiliary functions to the string.h of that add some special
 * features the already existing ones.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pg_error.h"
#include "pg_string.h"

/* Description: Tokenizes a given string according to an exact string delimiter.
 *
 * Arguments:	str:    String to be tokenized
 * 				dmstr: Delimiter used to seperate the tokens
 * Returns:		- On success, a null terminated array of token strings.
 * 				- On failure, NULL.
 *
 * Possible errors:		# No tokens found
 *						# Pointer to string given is NULL
 *					
 * Notes:		Programmer should free the space pointed by the returned pointer.
 */
 
char **ctokenize(const char *str, const char *delim) {
	char **tokens;	// Array of tokens
	char *tmpstr; // copy of str argument
	int token_number;	// number of tokens
	char * tmp_tkn;		// Temporary token used for copying
	int i;	// counter
	
	// Exceptions //
	
	if (str == NULL) {		// ERROR: NULL pointer passed
		pg_errno = ENULL;
		return NULL;
	}
	
	// Calculate number of tokens to allocate approprate space for the string array
	token_number = ctoken_counter(str, delim);
	if (token_number == 0) {	// No tokens found	
		return NULL;
	}
	
	// Allocate space for tokens plus one for the NULL indicator
	tokens = (char **)malloc((token_number+1) * sizeof(char *));
	if (tokens == NULL) {	// Sanity Check: Allocation Error
		perror("malloc");
		return NULL;
	}
	tmpstr = (char *)malloc( (strlen(str) + 1) * sizeof(char) );
	
	if (tmpstr == NULL) {	// Sanity Check: Allocation Error
		perror("malloc");
		return NULL;
	}
	strcpy(tmpstr, str); // Tamper with copied data, dont modify the original

	tmp_tkn = strtok(tmpstr, delim);	// First token
	
	// Copy first token to the first element of the token array
	tokens[0] = (char*)malloc( (strlen(tmp_tkn) + 1) * sizeof(char));
	strcpy(tokens[0], tmp_tkn);
	
	// Subsequent tokenizations
	for(i=1 ; i<token_number ; ++i) {
		tmp_tkn = strtok(NULL, delim);
		// Copy each token to the token array
		tokens[i] = (char*)malloc( (strlen(tmp_tkn) + 1) * sizeof(char));
		strcpy(tokens[i], tmp_tkn);
	}
	
	// Terminate the last token with '\0' case it is '\n' terminated
	
	int last_tkn_cnt = strcnt(tokens[token_number-1]);
	if (tokens[token_number-1][last_tkn_cnt-1] == '\n') {
		tokens[token_number-1][last_tkn_cnt-1] = '\0';
	}
	
		
	tokens[token_number] = (char *)0;	// Terminate the string array with NULL
	
	free(tmpstr);		// Free temporary allocated space
	
	return tokens;
	
}
 
/* Description: Calculates the number of tokens based on the given delimeter
 *
 * Arguments:	str:    String to be traversed
 * 				delimter: Delimeter of the tokens
 *
 * Returns:		- On success, number of tokens.
 * 				- On failure,
 *						# -1 NULL pointer to string given as an argument
 *					
 * Notes:		Return value 0, means that empty string was passed as an argument
 */

int ctoken_counter(const char * str, const char *delim) {
	int counter=0;	// delimeter counter
	const char *substr =  str;	// Initialize substr to the whole str
	
	
	if (str == NULL) {	// ERROR: NULL pointer passed
		return -1;
	} else if (*str == '\0') {	// Empty string passed
		return 0;
	}
	
	
	
	// Use substr to indicate the end of each search and the start of a new one
	int skip = skip_delim(substr, delim);
	
	substr+=skip;	// skip delimeters in the start if any
	while( (substr = strpbrk(substr, delim)) != NULL) {
		// Ignore delimeters that are placed sequentially one next to the other
		skip=skip_delim(substr, delim);
		substr+=skip;	// skip delimeters after the token
		++counter;
	}
	
	
	// Configure str to point to the character before '\n'
    str+=strcnt(str)-1;
    if (*str == '\n') {
 	   --str;
    }
        
    // If there are delimeters in the end of the string, counter'll be 1 unit larger
	if (strpbrk(str, delim) != NULL) {
		--counter;	// Subtract the extra measurement
	}
	
	return counter+1;
	
}
 
/* Description: Skips delimeters that are placed sequentially one next to the other
 *
 * Arguments:	str:    String to be traversed
 *				delim:	delimeter set to skip
 * 				
 * Returns:		- On success, the number of characters skipped.
 * 				- On failure, -1.
 *
 * Possible errors:		# NULL pointer to string given as an argument
 *					
 * Notes:		None
 */

int skip_delim(const char *str, const char *delim) {
	char *tmp_str;
	int offset=0;
	
	if (str == NULL || delim == NULL) {
		return -1;
	}
	
	// Allocate space for temp_str
	tmp_str = (char *)malloc( 2 * sizeof(char));
	strncpy(tmp_str, str, 1); // copy the next delim_cnt chars to tmp_str
	tmp_str[1] = '\0';	// NULL terminate the string
	
	while(strpbrk(tmp_str, delim)!=NULL) {
		++str;
		++offset;
		free(tmp_str);
		tmp_str = (char *)malloc( 2 * sizeof(char));
		strncpy(tmp_str, str, 1);
		tmp_str[1] = '\0';
	}
	
	free(tmp_str);
	
	return offset;
	
}
 
/* Description: Tokenizes a given string according to an exact string delimiter.
 *
 * Arguments:	str:    String to be tokenized
 * 				dmstr: Delimiter used to seperate the tokens
 * Returns:		- On success, a null terminated array of token strings.
 * 				- On failure, NULL.
 *
 * Possible errors:		# No tokens found
 *						# Pointer to string given is NULL
 *					
 * Notes:		None
 */

char **stokenize(const char *str, const char *dmstr) {
	char **tokens;	// Array of tokens
	char *tmpstr; // copy of str argument
	int token_number;	// number of tokens
	char * tmp_tkn;		// Temporary token used for copying
	int i;	// counter
	
	if (str == NULL) {		// ERROR: NULL pointer passed
		return NULL;
	}
	
	// Calculate number of tokens to allocate approprate space for the string array
	token_number = stoken_counter(str, dmstr);
	if(token_number == 0) {
		return NULL;
	}
	// Allocate space for tokens plus one for the NULL indicator
	tokens = (char **)malloc((token_number+1) * sizeof(char *));
	if (tokens == NULL) {
		perror("malloc");
		return NULL;
	}
	tmpstr = (char *)malloc( (strlen(str) + 1) * sizeof(char) );
	strcpy(tmpstr, str); // Tamper with copied data, dont modify the original

	tmp_tkn = sstrtok(tmpstr, dmstr);	// First token
	
	// Copy first token to the first element of the token array
	tokens[0] = (char*)malloc( (strlen(tmp_tkn) + 1) * sizeof(char));
	strcpy(tokens[0], tmp_tkn);
	
	// Subsequent tokenizations
	for(i=1 ; i<token_number ; ++i) {
		tmp_tkn = sstrtok(NULL, dmstr);
		// Copy each token to the token array
		tokens[i] = (char*)malloc( (strlen(tmp_tkn) + 1) * sizeof(char));
		strcpy(tokens[i], tmp_tkn);
		
	}
	
	// Terminate the last token with '\0' case it is '\n' terminated
	
	int last_tkn_cnt = strcnt(tokens[token_number-1]);
	if (tokens[token_number-1][last_tkn_cnt-1] == '\n') {
		tokens[token_number-1][last_tkn_cnt-1] = '\0';
	}
	
		
	tokens[token_number] = (char *)0;	// Terminate the string array with NULL
	
	free(tmpstr);
	
	return tokens;
	
}

/* Description: Returns the number of a specified character's occurances inside the
 *				given string.
 *
 * Arguments:	str:    String to be traversed
 * 				character: Character to be searched
 * Returns:		- On success, number of characters occurances.
 * 				- On failure, -1.
 *
 * Possible errors:		# NULL pointer to string given as an argument
 *					
 * Notes:		None
 */

int char_counter(const char *str, char character) {
	int counter = 0;
	int i=0;
	
	if(str == NULL) {
		return -1;
	}
	
	while(str[i]!= '\0') {
		if (str[i] == character) {
			++counter;
		}
		++i;
	}
	
	return counter;	
}

/* Description: Calculates the number of tokens based on the exact string given as a
 *				delimiter.
 *
 * Arguments:	str:    String to be traversed
 * 				dmstr: Delimeter of the tokens
 *
 * Returns:		- On success, number of tokens.
 * 				- On failure,
 *						# -1 NULL pointer to string given as an argument
 *					
 * Notes:		Return value 0, means that empty string was passed as an argument
 */

int stoken_counter(const char * str, const char *dmstr) {
	int counter=0;	// delimeter counter
	const char *substr =  str;	// Initialize substr to the whole str
	
	
	if (str == NULL) {	// ERROR: NULL pointer passed
		pg_errno = ENULL;
		return -1;
	} else if (*str == '\0') {	// Empty string passed
		return 0;
	}
	
	
	
	// Use substr to indicate the end of each search and the start of a new one
	int skip = strskp(substr, dmstr);
	
	substr+=skip;	// skip delimeters in the start if any
	while( (substr = strstr(substr, dmstr)) != NULL) {
		// Ignore delimeters that are placed sequentially one next to the other
		skip=strskp(substr, dmstr);
		substr+=skip;	// skip delimeters after the token
		++counter;
	}

	// If there are delimeters in the end of the string, counter'll be 1 unit larger
	if (strstr(str+(strcnt(str) - strcnt(dmstr) -1), dmstr) != NULL) {
		--counter;	// Substruct the extra measurement
	}
	return counter+1;
	
}
 
/* Description: Skips exact string occurences in given string, till the first
 * 				character of the string that differs from the searching one.
 *
 * Arguments:	str:    String to be traversed
 *				skstr:	string to skip
 * 				
 * Returns:		- On success, the number of characters skipped.
 * 				- On failure, -1.
 *
 * Possible errors:		# NULL pointer to string given as an argument
 *					
 * Notes:		None
 */

int strskp(const char *str, const char *skstr) {
	char *tmp_str;
	int offset=0;
	
	if (str == NULL || skstr == NULL) {
		return -1;
	}
	
	int skstr_cnt = strcnt(skstr);	// Number of skstr's characters
	
	// Allocate space for temp_str
	tmp_str = (char *)malloc( (skstr_cnt + 1) * sizeof(char));
	strncpy(tmp_str, str, skstr_cnt); // copy the next skstr_cnt chars to tmp_str
	tmp_str[skstr_cnt] = '\0';	// NULL terminate the string
	
	while(strstr(tmp_str, skstr)!=NULL) {
		str+=skstr_cnt;
		offset+=skstr_cnt;
		free(tmp_str);
		tmp_str = (char *)malloc( (skstr_cnt + 1) * sizeof(char));
		strncpy(tmp_str, str, skstr_cnt);
		tmp_str[skstr_cnt] = '\0';
	}
	
	free(tmp_str);
	
	return offset;
	
}

/* Description: Like strtok but treats the skstr string as a whole not as a char set.
 *
 * Arguments:	str:    string to be traversed
 *				skstr:	exact string delimiter 
 *						# Pass NULL to signal tokenizing from the previous position
 *						  of the former tokenized string.
 * 				
 * Returns:		- On success, substring begging from the token's first character.
 * 				- On failure, NULL.
 *
 * Possible errors:		# Cannot stokenize string
 *						# skstr was set to NULL in the first call
 *					
 * Notes:		None
 */

char * sstrtok(char *str, const char *dmstr) {
	int skip;	
	char *startPtr;
	static char *endPtr=NULL;	// Used for subsequent calls, in order to continue
								// tokenizing from the previous string	
	
	if(str == NULL) {		// Subsequent Calls
		if (endPtr == NULL || *endPtr == '\0') {
			return NULL;
		}
		startPtr = endPtr;
	} else {				// First Call
		startPtr = str;
		skip=strskp(str, dmstr); // Skip delims in the beggining of str, if any
		startPtr+=skip;
	}
	
	// Store the starting position of the next delimiter to endPtr
	endPtr = strstr(startPtr, dmstr);
	
	// Last token to return
	if (endPtr == NULL && startPtr != NULL) {
		return startPtr;
	}
	
	skip=strskp(endPtr, dmstr); // Find the next tokens offset from endPtr
	*endPtr='\0';	// Terminate the token with NULL
	endPtr+= skip;	// set endPtr to point to next token
	
	if (*endPtr == '\n') {
		*endPtr = '\0';
	}
	
	return startPtr;
	
}

/* Description: Counts the number of the given string's characters. Same as strlen
 *
 * Arguments:	str:    String to be traversed
 * 				
 * Returns:		- On success, number of tokens.
 * 				- On failure, -1.
 *
 * Possible errors:		# NULL pointer to string given as an argument
 *					
 * Notes:		None
 */

int strcnt(const char * str) {

	int counter=0;

	if (str == NULL) {
		return -1;
	}
	
	// Change pointer on each loop to point to the next character
	while (*(str++) != '\0') {
		++counter;
	}
	
	return counter;
	
}

/* Description: Concatenates a string array in to one string interleaving
 * 				each string with the given delimiter.
 *
 * Arguments:	strarray:	String array to be concatenated
 * 				delim: 		String delimeter
 *				start:		String array start index
 *				end:		String array end index
 *
 * Returns:		- On success, concatenated strings seperated by given delimiter
 * 				- On failure, NULL.
 *
 * Possible errors:		# NULL pointer given as an argument to strarray
 *						# NULL pointer given as an argument to delim
 *						# Start index is negative
 *						# End index is lower than start index
 *						# Error allocating memory
 *
 *
 * Notes:		None
 */
 
char * astrcat( char ** strarray, char * delim, int start, int end) {
 	
	int count;	// Number of final string's characters without the delimeters
	int space;	// Exact space to be allocated for the final string
				// space = count + delimeters + 1(null char)
				// delimeters = (#strings -1)*delim_lenght =
				// 			  = (end-start+1 -1)*delim_length =
				//			  = (end-start)*delim_length
 	
	char * finalstr;	// String to be returned
	int pos; // current index position in the final string in the copying process
 	int i, j;	// counters;
 	
 	// Argument checks
 	if (strarray == NULL) {
 		fprintf(stderr, "%s\n", "astrcat: String array cannot be NULL");
 		return NULL;
 	}
 	if (delim == NULL) {
 		fprintf(stderr, "%s\n", "astrcat: String delimiter cannot be NULL");
 		return NULL;
 	}
 	if (start < 0) {
 		fprintf(stderr, "%s\n","astrcat: Start index must be positive");
 		return NULL;
 	}
 	if (end < start) {
 		fprintf(stderr, "%s\n","astrcat: End index must be lower than start index");
 		return NULL;
 	}
 	
 	// If start is the same as end, return the token on index start
 	if (start == end) {
 		return strarray[start];
 	}
 	
 	// Calculate the total number of characters from all the array's strings
 	count=0;
 	for(i=start; i<=end; ++i) {
 		count += strlen(strarray[i]);
 	}
 	
 	space = count + (end-start)*strlen(delim) + 1;
 	
 	// Memory Allocation
 	finalstr = (char *)malloc(space*sizeof(char));
 	if(finalstr==NULL) {
 		perror("malloc");
 		return NULL;
 	}
 	
 	// Loop for every string within the range start - end -1
 	pos=0;
 	for(i=start; i<end; ++i) {
 	
 		// Append string entry to final string
 		j=0;
 		while(strarray[i][j] != '\0') {
 			finalstr[pos] = strarray[i][j];
 			++pos;
 			++j;
 		}
 		
 		// Append delimeter to final string
 		j=0;
 		while(delim[j] != '\0') {
 			finalstr[pos] = delim[j];
 			++pos;
 			++j;
 		}
 	}

 	// Append final string entry to final string
 	i=0;
 	while (strarray[end][i] != '\0') {
 		finalstr[pos] = strarray[end][i];
 		
 		++pos;
 		++i;
 	}
 	
 	finalstr[pos] = '\0';	// Null terminate string
 	
 	return finalstr;
 	
}


/* Description: Removes the dirt double characters from the dirty string.
 *				(	string empty pair clean   )
 *
 * Arguments:	dirty_str:  String to be cleaned
 *				dirt:		Dirty character to be removed
 * Returns:		- On success, the cleaned string with dirty characters removed.
 * 				- On failure, NULL.
 *						
 *
 * Possible errors:		# ENULL: 	NULL pointer passed to str argument
 *						# EPARSE: 	Odd number of delims, pairs cannot be shaped
 *						# EEMPTSTR:	Empty string passed
 *					
 * Notes:		The original string is not tampered. Copy of it "clean version" will
 *				be returned. Remember to free space if you are not going to use the
 *				returned string.
 */

char * strepclean(char * dirty_str, char dirt) {
	
	char * clean_str;
	
	int i;		// Dirt string index
	int j;		// Clean string index
	int dirt_cnt=0;
	int flag=0;	// true indicates opening dirt, false indicates closing dirt
	int placeDirt=0;	// indicates dirt characters to place (0 or 1) 

	// Exceptions //
	
	// Check if NULL pointer passed
	if (dirty_str == NULL) {
		pg_errno = ENULL;
		return NULL;
	}
	
	// Check if empty string was passed
	if (dirty_str == '\0') {
		pg_errno = EEMPTSTR;
		return NULL;
	}
	
	dirt_cnt = char_counter(dirty_str, dirt); // Count the dirty characters
		
	// If that number is odd,
	if (dirt_cnt%2 == 1) {
		pg_errno = EPARSE;		// Issue a string parse error
		return NULL;
	}
	
	
	// If no dirty characters were found
	if (dirt_cnt == 0) {
		return dirty_str;	// The string is clean. Return it
	}
	
	// Allocate space for the clean string, equal to the dirty_string's space
	clean_str = (char *)malloc( ( strlen(dirty_str)+1 ) * sizeof(char));
	
	// Loop for every character in the dirty string
	i=0;
	j=0;
	while(dirty_str[i] != '\0') {
		
		
		// dirty character found
		if (dirty_str[i] == dirt ) {
			flag^=1;	// Invert edge_flag
			++i;
		}
		// clean character found
		else {
			// Place opening or closing dirt character to clean string
			if(flag) {
				clean_str[j] = dirty_str[i-1];
				++j;
			}
			
			do {
				clean_str[j] = dirty_str[i];
				++j;
				++i;
			} while(dirty_str[i] != dirt && dirty_str[i] != '\0');
			
			
			flag^=1;	// Dirt is open again
			
			if (!flag) {
				clean_str[j] = dirty_str[i];
				++j;
			}
			
			if (dirty_str[i] != '\0') {
				++i;
			}
		}
	}
	
	clean_str[j] = '\0';	// Null terminate string
	return clean_str;
}


