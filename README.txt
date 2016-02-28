##################### PGSH - Panos Gemos Shell #####################

PGSH, is a tiny and simple shell that:
	1) Can execute commands with multiple arguments
	2) Supports input and or output redirection
	3) Supports command pipelining
	4) Saves command history to a file
	
NOTES: The whole source for this project was written in pure C,
	   without the use of any third-party library. The only
	   source file that was not written by me was the getline.c
	   which I inserted later for compatibility with Solaris.
	    
Author: Panos Gemos

--------------------------------------------------------------------

# Usage Examples:

1) cat file1 file2 file3
2) ls -al > out.txt
3) ls -al | tr a-z A-Z | tail -2
4) ps | tail -2 > output.txt
5) tail -1 < out.txt | xargs ls -al > output.txt

-- Version 0.0.1 (9/10/2015) --

# Features to be Implemented:

1) User specific profiles (Login Mechanism)
	i) Save passwords encrypted.
2) Separate history files for different users
3) Change current directory
4) Ability to change cmd prompt

# Bugs

1) Configure what to do when ctrl-D is pressed. Should ignore accepting a command
2) Exclude tokenizing in text inside quotes.

-- Version 0.0.2 (11/10/2015) --

# Added Features

1) Change current directory
2) Solaris Compatible

# Fixed Bugs

1) Memory leak issues (memory was not freed)
