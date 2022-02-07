#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

/* I did not plagiarize off of this, but this really helped me
* get started on this assignment:
* https://brennan.io/2015/01/16/write-a-shell-in-c/
* I got the idea of the general program layout from this link
* things like having the do while loop in main
* and building the readLine, getArgs, and execution lines
* separate
*/

/* Structure to store the user input
*/
int globalStatus = 0;
int processes[20];
int processIndx = 0;
int fgOnlyFlag = 0;

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
* Wow, this is really bad, but in desperation, this is what I came up with
* Takes pointer to token, returns pointer 
*/
char* expand(char* ptrToToken) {
	int count = 0;
	char pidStr [20];
	const char* tmp = ptrToToken;
	int i;
	int j = 0;
	int n;

	pid_t getpid();
	sprintf(pidStr, "%d", getpid);
	// Counts the occurences of $$ to allocate sufficient memory
	while (tmp = strstr(tmp, "$$")) {
		count++;
		tmp++;
	}
	char* newBlock = malloc((strlen(ptrToToken)) + count*strlen(pidStr)+1);
	for (i = 0; i <= strlen(ptrToToken); i++) {
		if (ptrToToken[i] == '$' && ptrToToken[i + 1] == '$') {		// If the current index and the next index is $$
			for (n = 0; n < strlen(pidStr); n++) {			// adds the pid string to the new memory allocation
				newBlock[j] = pidStr[n];
				j++;
			}
			i++;
		}
		else {									// Otherwise, copy the original string bit by bit
			newBlock[j] = ptrToToken[i];
			j++;
		}
	}
	ptrToToken = newBlock;			// Reassign pointerToToken, this doesn't really matter since I can't even free it
	return ptrToToken;				// Return
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
	int argFlag = 0;	// If arg flag is set, any additional arguments that are not input, output, or an ampersand are ignored
	char* saveptr;
	char* token = strtok_r(userIn, " ", &saveptr);			// Split first token from userIn


	while (token != NULL) {
		// Checks if input or output is entered, sets flag and progresses token to next value if it does
		// Once input or output is entered, argFlag is set
		if (strcmp(token, "<") == 0) {
			flag = 2;
			token = strtok_r(NULL, " ", &saveptr);
			argFlag = 1;
		}
		else if (strcmp(token, ">") == 0) {
			flag = 3;
			token = strtok_r(NULL, " ", &saveptr);
			argFlag = 1;
		}
		else if (strcmp(token, "&") == 0 && strtok_r(NULL, " ", &saveptr) == NULL && fgOnlyFlag == 0) {		// Only sets flag to 4 if ampersand is last value of input and fgOnly is off
			flag = 4;
		}
		else if (argFlag == 1) {			// ignores any additional arguments if argFlag is set
			token = strtok_r(NULL, " ", &saveptr);
			continue;
		}
		// Expands the '$$' if it is detected in the token
		if (strstr(token, "$$") != NULL) {
			token = expand(token);
		}
		// switch checks flag, allocates information to structure
		switch (flag) {
		// case 0 will be for the command
		case 0:
			currEntry->command = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currEntry->command, token);
			flag = 1;
			break;
		// case 1 will be for the arguments arguments will only be enterable after the command has been populated
			// additional arguments after a redirection will simply be ignored
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
			break;
		// case 3 is for redirectedOut
		case 3:
			currEntry->redirectedOut = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currEntry->redirectedOut, token);
			break;
		// case 4 is for the ampersand, signifying process is to be run in the background
		case 4:
			if (strcmp(token, "&") == 0) {
				currEntry->background = 1;
			}
			break;
		}
		token = strtok_r(NULL, " ", &saveptr);					// Split new token
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
	int i;
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
	printf("exit value %d\n", globalStatus);
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
	if ((currEntry->redirectedIn == NULL || currEntry->redirectedOut == NULL) && currEntry->background != 0) {
		int targetNull = open("/dev/null", O_RDWR);
		if (currEntry->redirectedIn == NULL){
			int changeInput = dup2(targetNull, 0);			// Redirects to dev/null if process is to be run in background 
		}													// and no redirection has been done
		if (currEntry->redirectedOut == NULL){
			int changeOutput = dup2(targetNull, 1);
		}													
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
	processes[processIndx] = spawnPid;
	processIndx++;

	switch (spawnPid) {
	case -1:
		perror("fork()\n");
		exit(1);
		break;
	case 0:  // Child process
		if (currEntry->background == 1) {
			fprintf(stdout, "Background PID is %d\n", getpid());
			fflush(stdout);
		}

		redirection(currEntry);
		sleep(5);
		execvp(currEntry->command, newArgv);

		perror("execvp\n");
		exit(EXIT_FAILURE);
		break;
	default:  // Parent Process
		if (currEntry->background == 1) {						// If background is set, no wait will occur
			fprintf(stdout, "Process continuing in background, returning to command line\n");
			fflush(stdout);
			pid_t childPid = waitpid(spawnPid, &childStatus, WNOHANG);
			sleep(2);
		}
		else {													// Foreground processes
			pid_t childPid = waitpid(spawnPid, &childStatus, 0);
			if (WIFSIGNALED(childStatus)) {						// Checks for abnormal exit
				fprintf(stdout, "Child %d exited abnornmally, exit code: %d\n", childPid, WTERMSIG(childStatus));
				fflush(stdout);
				globalStatus = WTERMSIG(childStatus);
			}
			else {
				fprintf(stdout, "Child %d exited normally, exit code: %d\n", childPid, WEXITSTATUS(childStatus));
				fflush(stdout);
				globalStatus = WEXITSTATUS(childStatus);
			}
		}
		break;
	}
	return;
}

void handle_SIGINT(int signo) {
	char* message = "Caught SIGINT\n";
	write(STDOUT_FILENO, message, 15);

}

void handle_SIGTSTP(int signo) {
	if (fgOnlyFlag == 0) {
		char* message = "Entering foreground-only mode (& is now ignored)\n";
		write(STDOUT_FILENO, message, 50);
		fgOnlyFlag = 1;
	}
	else if (fgOnlyFlag == 1) {
		char* message = "Exiting foreground-only mode\n";
		write(STDOUT_FILENO, message, 30);
		fgOnlyFlag = 0;
	}
}

// SIGCHLD handler
void handle_SIGCHLD(int signo) {
	char* message = "Caught SIGCHLD\n";
	write(STDOUT_FILENO, message, 15);
}

// createSignals initiates the required materials to handle signals
void createSignals() {
	struct sigaction SIGCHLD_action = { 0 };
	struct sigaction SIGINT_action = { 0 };
	struct sigaction SIGTSTP_action = { 0 };

	sigset_t fgbg;
	sigset_t shbg;
	

	// SIGCHLD handler
	SIGCHLD_action.sa_handler = handle_SIGCHLD;
	SIGCHLD_action.sa_flags = SA_RESTART;
	sigaction(SIGCHLD, &SIGCHLD_action, NULL);

	// SIGINT handler (ctrl+c), shell and children in background processes ignore SIGINT
	// SIGINT should cause children in foreground to self-terminate
	// Parent must then print PID
	SIGINT_action.sa_handler = handle_SIGCHLD;
	SIGINT_action.sa_flags = SA_RESTART;

	// SIGTSTP handler (ctrl+z), both foreground and background children ignore SIGTSTP
	// Parent process running the shell receives SIGSTP
	SIGTSTP_action.sa_handler = SIG_IGN;
	SIGTSTP_action.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	return;
}

/* main function
*/
int main(int argc, char* argv[]) {
	char* userInput;
	int leave;
	createSignals();
	// main loop, continues until status
	do {
		sleep(2);
		printf(": ");
		userInput = readLine();
		if (strcmp(userInput, "") == 0 || userInput[0] == '#') {
			continue;
		}
		struct entry* arguments = buildEntry(userInput);
		if (arguments->command == NULL) {
			continue;
		}
		leave = execute(arguments);

		free(userInput);
		free(arguments);

	} while (leave == 0);

	return EXIT_SUCCESS;
}
