#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

/* I did not plagiarize off of this, but this really helped me
* get started on this assignment:
* https://brennan.io/2015/01/16/write-a-shell-in-c/
* I got the idea of the general program layout from this link
* things like having the do while loop
* and building the readLine, getArgs, and execution lines
* separate
*/

struct entry {
	char* command;
	char* arguments;
	char* redirectedIn;
	char* redirectedOut;
	int background;
};

/* readLine function
* gets user input
* returns a pointer to input
*/
char* readLine(void) {
	char* input = NULL;
	size_t buffsize = 0;

	getline(&input, &buffsize, stdin);

	return input;
}

/* function buildEntry
* takes user input as an argument
* tokenizes into an array of arguments
* returns array
*/
struct entry* buildEntry(char* userIn) {
	struct entry* currEntry = malloc(sizeof(struct entry));
	size_t newSpace;
	int flag = 0;		// flag 0 = initial, 1 = arguments, 2 = input, 3 = output, 4 = ampersand check

	char* saveptr;
	char* token = strtok_r(userIn, " ", &saveptr);
	while (token != NULL) {

		if (strcmp(token, "<") == 0) {
			flag = 2;
			token = strtok_r(NULL, " ", &saveptr);
		}
		else if (strcmp(token, ">") == 0) {
			flag = 3;
			token = strtok_r(NULL, " ", &saveptr);
		}
		switch (flag) {
		case 0:
			currEntry->command = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currEntry->command, token);
			flag = 1;
			break;
		case 1:
			if (currEntry->arguments != NULL){
				newSpace = sizeof(currEntry->arguments) + sizeof(token) + 2;
				currEntry->arguments = realloc(currEntry->arguments, newSpace);
				strcat(currEntry->arguments, " ");
				strcat(currEntry->arguments, token);
			}
			else {
				currEntry->arguments = calloc(strlen(token) + 1, sizeof(char));
				strcpy(currEntry->arguments, token);
			}
			break;
		case 2:
			currEntry->redirectedIn = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currEntry->redirectedIn, token);
			flag = 4;
			break;
		case 3:
			currEntry->redirectedOut = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currEntry->redirectedOut, token);
			flag = 4;
			break;
		case 4:
			if (strcmp(token, "&\n") == 0) {
				currEntry->background = 1;
			}
			break;
		}
		token = strtok_r(NULL, " ", &saveptr);
	}
	
	return currEntry;
}

/* function execute
* 
*/
int execute(struct entry* argument) {
	
	return 0;
}

/* main function
*/
int main(int argc, char* argv[]) {
	char* userInput;
	int again;

	// main loop, continues until status
//	do {
		printf(": ");
		userInput = readLine();
		struct entry* arguments = buildEntry(userInput);
		again = execute(arguments);

		free(userInput);

//	} while (status);

	return EXIT_SUCCESS;
}