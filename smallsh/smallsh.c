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
static int globalStatus = 0;
static int fgOnlyFlag = 0;

struct entry {
	char* command;
	char* arguments[512];
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
	currEntry->redirectedIn = NULL;
	currEntry->redirectedOut = NULL;
	currEntry->argCounter = 0;
	currEntry->background = 0;

	size_t newSpace;
	int counter = 0;
	int flag = 0;		// flags: 0 = initial, 1 = arguments, 2 = input, 3 = output, 4 = ampersand check
	int argFlag = 0;	// If arg flag is set, any additional arguments that are not input, output, or an ampersand are ignored
	char* saveptr;
	char* token = strtok_r(userIn, " ", &saveptr);			// Split first token from userIn
	char* newBlockptr;

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
		// Only sets flag to 4 if ampersand is last value of input
		else if (strcmp(token, "&") == 0 && strtok_r(NULL, " ", &saveptr) == NULL) {
			flag = 4;
		}
		// ignores any additional arguments if argFlag is set
		else if (argFlag == 1) {
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
			newBlockptr = (char*)calloc(strlen(token) + 1, sizeof(char));
			strcpy(newBlockptr, token);
			currEntry->arguments[currEntry->argCounter] = newBlockptr;
			counter += 1;
			currEntry->argCounter = counter;
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
			// Handles ampersand if foreground only mode is on
			if (fgOnlyFlag == 1) {
				break;
			}
			if (strcmp(token, "&") == 0) {
				currEntry->background = 1;
			}
			break;
		}
		// Split new token
		token = strtok_r(NULL, " ", &saveptr);
	}
	return currEntry;
}

/* function execute
* takes pointer to entry structure
*/
int execute(struct entry* currEntry) {
	if (strcmp(currEntry->command, "exit") == 0) {
		closeProcesses();
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
void closeProcesses() {
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
	if (currEntry->arguments[0] == NULL) {
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
		if (chdir(currEntry->arguments[0]) == -1) {
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
	int i;
	int j = 1;
	char* newArgv[512] = {									// Populates with command, arguments, and last will be NULL
		currEntry->command,
	};
	for (i = 0; i <= currEntry->argCounter; i++) {
		if (i == currEntry->argCounter) {
			newArgv[j] = NULL;
		}
		else {
			newArgv[j] = currEntry->arguments[i];
			j++;
		}
	}

	int childStatus;
	pid_t spawnPid = fork();

	switch (spawnPid) {
	case -1:
		perror("fork()\n");
		globalStatus = 1;
		exit(1);
		break;
	case 0:  // Child process
	{
		// SIGINT handler (ctrl+c), background processes must ignore
		if (currEntry->background == 1) {
			struct sigaction SIGINT_action = { 0 };
			SIGINT_action.sa_handler = SIG_IGN;
			SIGINT_action.sa_flags = SA_RESTART;
			sigaction(SIGINT, &SIGINT_action, NULL);
		}
		// SIGTSTP handler (ctrl+z), both foreground and background children ignore SIGTSTP
		struct sigaction SIGTSTP_action = { 0 };
		SIGTSTP_action.sa_handler = SIG_IGN;
		SIGTSTP_action.sa_flags = SA_RESTART;
		sigaction(SIGTSTP, &SIGTSTP_action, NULL);

		// Handles redirection and then executes
		redirection(currEntry);
		execvp(currEntry->command, newArgv);

		perror("execvp\n");
		globalStatus = 1;
		exit(1);
		break;
	}
	default:  // Parent Process
		if (currEntry->background == 1) {						// If background is set, no wait will occur
			fprintf(stdout, "process %d continuing in background, returning to command line\n", spawnPid);
			fflush(stdout);
		}
		else {													// Foreground processes
			pid_t childPid = waitpid(spawnPid, &childStatus, 0);
			if (WIFSIGNALED(childStatus)) {						// Checks for abnormal exit
				fprintf(stdout, "child %d exited abnornmally, exit code: %d\n", childPid, WTERMSIG(childStatus));
				fflush(stdout);
				globalStatus = WTERMSIG(childStatus);
			}
			else {												// Otherwise, checks for normal exit
				globalStatus = WEXITSTATUS(childStatus);
			}
		}
		break;
	}
	return;
}

void handle_SIGINT(int signo) {
	char* message = "Terminated by signal ";
	write(STDOUT_FILENO, message, 22);

}

void handle_SIGTSTP(int signo) {
	if (fgOnlyFlag == 0) {
		char* message = "Entering foreground-only mode (& is now ignored)\n: ";
		write(STDOUT_FILENO, message, 52);
		fgOnlyFlag = 1;
	}
	else if (fgOnlyFlag == 1) {
		char* message = "Exiting foreground-only mode\n: ";
		write(STDOUT_FILENO, message, 33);
		fgOnlyFlag = 0;
	}
}

// createSignals initiates the required materials to handle signals for the shell
void createSignals() {
	struct sigaction SIGINT_action = { 0 };
	struct sigaction SIGTSTP_action = { 0 };
	
	// SIGINT handler (ctrl+c), shell is set to ignore SIGINT
	// SIGINT should cause children in foreground to self-terminate
	// Parent must then print PID
	SIGINT_action.sa_handler = handle_SIGINT;
	SIGINT_action.sa_flags = SA_RESTART;
	sigaction(SIGINT, &SIGINT_action, NULL);


	// SIGTSTP handler (ctrl+z), shell is set to change to foreground only mode
	// Parent process running the shell receives SIGSTP
	SIGTSTP_action.sa_handler = handle_SIGTSTP;
	SIGTSTP_action.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	return;
}

void checkBG() {
	int tempStatus;
	pid_t childProcess = waitpid(-1, &tempStatus, WNOHANG);			// Check background processes
	if (childProcess > 0) {
		fprintf(stdout, "background process %d has finished: exit value %d\n", childProcess, tempStatus);
		fflush(stdout);
	}
}

/* main function
*/
int main(int argc, char* argv[]) {
	char* userInput;
	int leave;
	createSignals();
	int i;

	// main loop, continues until leave is set to 1
	do {
		checkBG();
		fprintf(stdout, ": ");
		fflush(stdout);
		userInput = readLine();
		if (userInput[0] == '#') {
			continue;
		}
		struct entry* arguments = buildEntry(userInput);
		if (arguments->command == NULL) {
			continue;
		}
		leave = execute(arguments);

		// cleanup
		free(userInput);
		free(arguments->command);
		free(arguments->redirectedOut);
		free(arguments->redirectedIn);
		for (i = 0; i < arguments->argCounter; i++) {
			free(arguments->arguments[i]);
		}
	} while (leave == 0);

	return EXIT_SUCCESS;
}
