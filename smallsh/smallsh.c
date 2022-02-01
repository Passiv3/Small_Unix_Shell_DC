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

/* Structure to store the user input
*/
struct entry {
	char* command;
	char* arguments;
	char* redirectedIn;
	char* redirectedOut;
	int background;
	int argCounter;
};

/* readLine function
* gets user input
* returns a pointer to input
*/
char* readLine(void) {
	char* input = NULL;
	size_t buffsize = 0;

	getline(&input, &buffsize, stdin);
	input[strlen(input) - 1] = 0;

	return input;
}

/* function buildEntry
* takes pointer to user input as an argument
* tokenizes into an array of arguments
* returns pointer to structure
*/
struct entry* buildEntry(char* userIn) {
	struct entry* currEntry = malloc(sizeof(struct entry));
	size_t newSpace;
	int counter = 0;
	int flag = 0;		// flag 0 = initial, 1 = arguments, 2 = input, 3 = output, 4 = ampersand check
	char* saveptr;
	char* token = strtok_r(userIn, " ", &saveptr);

	while (token != NULL) {
		// Checks if input or output is entered, sets flag and progresses token to next value if it does
		if (strcmp(token, "<") == 0) {
			flag = 2;
			token = strtok_r(NULL, " ", &saveptr);
		}
		else if (strcmp(token, ">") == 0) {
			flag = 3;
			token = strtok_r(NULL, " ", &saveptr);
		}
		// switch checks flag, allocates information to structure
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
				counter += 1;
				currEntry->argCounter = counter;
			}
			else {
				currEntry->arguments = calloc(strlen(token) + 1, sizeof(char));
				strcpy(currEntry->arguments, token);
				counter += 1;
				currEntry->argCounter = counter;
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
			if (strcmp(token, "&") == 0) {
				currEntry->background = 1;
			}
			break;
		}
		token = strtok_r(NULL, " ", &saveptr);
	}
	return currEntry;
}

/* function execute
* takes pointer to entry structure
*/
int execute(struct entry* currEntry) {
	if (strcmp(currEntry->command, "exit") == 0) {
		closeProcesses(currEntry);
		return 1;
	}
	else if (strcmp(currEntry->command, "cd") == 0){
		changeDirectory(currEntry);
		return 0;
	}
	else if (strcmp(currEntry->command, "status") == 0) {
		getStatus(currEntry);
		return 0;
	}
	else {
		return;
	}
	return 0;
}

void closeProcesses(struct entry* currEntry) {
	return;
}

void changeDirectory(struct entry* currEntry) {
	char* homeDir = getenv("HOME");
	char* envPATH = getenv("PATH");

	char cwd[256];

	if (currEntry->arguments == NULL) {
		chdir(homeDir);
		fprintf(stdout, "current directory set to HOME: %s\n", getcwd(cwd, sizeof(cwd)));
		fflush(stdout);
	}
	else if (currEntry->argCounter > 1) {
		fprintf(stdout, "Too many arguments found");
		fflush(stdout);
		return;
	}
	else if (currEntry->arguments[0] == '/') {
		if (chdir(currEntry->arguments) == -1) {
			fprintf(stdout, "Error has occurred");
			fflush(stdout);
		}
		else {
			fprintf(stdout, "current directory set to: %s\n", getcwd(cwd, sizeof(cwd)));
			fflush(stdout);
		}
	}
	return;
}

void getStatus(struct entry* currEntry) {

	return;
}

/* main function
*/
int main(int argc, char* argv[]) {
	char* userInput;
	int again;

	// main loop, continues until status
	do {
		printf(": ");
		userInput = readLine();
		struct entry* arguments = buildEntry(userInput);
		again = execute(arguments);

		free(userInput);
		free(arguments);

	} while (again == 0);

	return EXIT_SUCCESS;
}
