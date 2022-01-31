#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

/* I did not plagiarize off of this, but this really helped me
* get started on this assignment:
* https://brennan.io/2015/01/16/write-a-shell-in-c/
* After it got me started, I closed it because I felt like it was cheating,
* but I got the general layout and function structure from it
*/

/* readLine function
* gets user input
* returns pointer to input
*/
char* readLine(void) {
	char* input = NULL;
	size_t buffsize = 0;

	getline(&input, &buffsize, stdin);

	return input;
}

/* function getArgs
* takes user input as an argument
* tokenizes into an array of arguments
* returns array
*/
char getArgs(char* userIn) {
	char* arguments[sizeof(userIn)];
	char* token;

	token = strtok_r(userIn, " ");

}

/* function execute
* 
*/
int execute(void) {
	
	return 0;
}

/* main function
*/
int main(int argc, char* argv[]) {
	char* userInput;
	char* arguments;
	int status;

	// main loop, continues until status
//	do {
		printf(": ");
		userInput = readLine();
		arguments = getArgs(userInput);
		free(userInput);

//	} while (status);

	return EXIT_SUCCESS;
}