#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

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
	char* expand;

 	getline(&input, &buffsize, stdin);
	input[strlen(input) - 1] = 0;

	return input;
}

/* Expands $$
* ******WIP****
*/
char* word(char* ptrToToken) {
	int count = 0;

	pid_t getpid(void);
	char* newBlock = malloc((strlen(ptrToToken)) + count);
	while (ptrToToken = strstr(ptrToToken, "$$")) {
		
	}
	ptrToToken = newBlock;

	return ptrToToken;
}

/* function buildEntry
* takes pointer to user input as an argument
* tokenizes into an array of arguments
* returns pointer to structure
*/
struct entry* buildEntry(char* userIn) {
	struct entry* currEntry = malloc(sizeof(struct entry));

	// Wipe the structure
	currEntry->command = NULL;
	currEntry->arguments = NULL;
	currEntry->redirectedIn = NULL;
	currEntry->redirectedOut = NULL;
	currEntry->background = 0;

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
		// Expands the '$$' if it is detected in the token
		if (strstr(token, "$$") != NULL) {
			return;
		}
		// switch checks flag, allocates information to structure
		switch (flag) {
		// case 0 will be for the command
		case 0:
			currEntry->command = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currEntry->command, token);
			flag = 1;
			break;
		// case 1 will be for the arguments
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
		// case 2 is if there is a redirectedIn
		case 2:
			currEntry->redirectedIn = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currEntry->redirectedIn, token);
			flag = 4;
			break;
		// case 3 is for redirectedOut
		case 3:
			currEntry->redirectedOut = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currEntry->redirectedOut, token);
			flag = 4;
			break;
		// case 4 is for the ampersand, signifying process is to be run in the background
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
		forkChild(currEntry);
		return 0;
	}
	return 0;
}

/* Closes all background tasks and primes program to exit
* WIP
*/
void closeProcesses(struct entry* currEntry) {
	return;
}

/* Function changeDirectory
* Takes entry structure as an argument
* changes directory according to given arguments
*/
void changeDirectory(struct entry* currEntry) {
	char* homeDir = getenv("HOME");
	char* envPATH = getenv("PATH");

	char cwd[256];

	// If no argument given, changes directory to home
	if (currEntry->arguments == NULL) {
		chdir(homeDir);
		fprintf(stdout, "current directory set to HOME: %s\n", getcwd(cwd, sizeof(cwd)));
		fflush(stdout);
	}// If more than one argument is g iven, error is printed
	else if (currEntry->argCounter > 1) {
		fprintf(stdout, "Too many arguments found");
		fflush(stdout);
		return;
	}// chdir works with both absolute and relative paths
	else  {
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

/*Returns the status*/
void getStatus(struct entry* currEntry) {

	return;
}

/* Handles redirection of output and input
* I got a good amount of this from the Processes and I/O Exploration
*/
void redirection(struct entry* currEntry) {
	// Checks if redirectedIn exists
	if (currEntry->redirectedIn != NULL) {
		int targetIn = open(currEntry->redirectedIn, O_RDONLY);		// Opens file and checks for error
		if (targetIn == -1) {
			perror("source open()");
		}
		int changeInput = dup2(targetIn, 0);						// Duplicates input fd to stdin and checks for error
		if (changeInput == -1) {
			perror("dup2");
			exit(2);
		}
		fcntl(targetIn, F_SETFD, FD_CLOEXEC);						// Sets FD to close once child has finished execution
	}
	if (currEntry->redirectedOut != NULL) {
		int targetOut = open(currEntry->redirectedOut, O_WRONLY | O_CREAT | O_TRUNC, 0644);	// Opens file for writing and checks for error
		if (targetOut == -1) {
			perror("target open()");
		}
		int changeOutput = dup2(targetOut, 1);					// Duplicates targetOut fd to stdout and checks for error
		if (changeOutput == -1) {
			perror("dup2");
			exit(2);
		}
		fcntl(targetOut, F_SETFD, FD_CLOEXEC);					// Sets FD to close once child has finished execution
	}
	return;
}

/* function forkChild takes entry structure as an argument
* Responsible for forking a child process and executing other programs
*/
void forkChild(struct entry* currEntry) {
	char* newArgv[] = {									// WIP, need to figure out how to make the argument from the structure
		currEntry->command,
		currEntry->arguments,
		NULL
	};
	int childStatus;
	pid_t spawnPid = fork();

	switch (spawnPid) {
	case -1:
		perror("fork()\n");
		exit(1);
		break;
	case 0:  // Child process
		fprintf(stdout, "Child Process %d Executing: \n", getpid());
		fflush(stdout);
		if (currEntry->redirectedIn != NULL || currEntry->redirectedOut != NULL) {
			redirection(currEntry);
		}
		execvp(currEntry->command, newArgv);
		perror("execvp");
		exit(EXIT_FAILURE);
		break;
	default:  // Parent Process
		spawnPid = waitpid(spawnPid, &childStatus, 0);
		fprintf(stdout, "Parent Process Finished\n");
		fflush(stdout);
		break;
	}
	return;
}

/* main function
*/
int main(int argc, char* argv[]) {
	char* userInput;
	int leave;

	// main loop, continues until status
	do {
		printf(": ");
		userInput = readLine();
		if (strcmp(userInput, "") == 0 || userInput[0] == '#') {
			continue;
		}
		struct entry* arguments = buildEntry(userInput);
		leave = execute(arguments);

		free(userInput);
		free(arguments);

	} while (leave == 0);

	return EXIT_SUCCESS;
}
