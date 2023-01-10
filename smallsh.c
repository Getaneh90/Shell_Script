/*
 * Filename: smallsh.c
 * Author: Getaneh Kudna
 * Description: CS 344 Assignment 3 smallsh
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#define INPUT_LENGTH 2048
int allow_Bkground = 1; 


void commandExc(char*[], int*, struct sigaction, int*, char[], char[]);
void get_Input(char*[], int*, char[], char[], int);
void SIGTSTP_handler(int);
void print_Exit_Status(int);


/*****************************************************************************

// main function gets the input and then checks for comments and or if it is blank 
//It processes status, cd and exit. It then executes the command if no other functions are called. 
*****************************************************************************/


int main (){

	
	int i;
	int exit_stat = 0;
	int the_backGround = 0;

	char output_file[256] = "";
	char input_file[256] = "";
	
char* the_input[512];
	for (i=0; i<512; i++) {
		the_input[i] = NULL;
	}


	int the_pid = getpid();
	int first_cont = 1;
	 

//the redirection ^Z to SIGTSTP_handler()
struct sigaction sa_sigtstp = {0}; 
sa_sigtstp.sa_handler = SIGTSTP_handler; 
sigfillset(&sa_sigtstp.sa_mask);
sa_sigtstp.sa_flags = 0; 
sigaction(SIGTSTP, &sa_sigtstp, NULL); 

	
//signal handlers, except ^c 
struct sigaction sa_sigint = {0};
sa_sigint.sa_handler = SIG_IGN; 
sigfillset(&sa_sigint.sa_mask);
sa_sigint.sa_flags= 0;
sigaction(SIGINT, &sa_sigint, NULL); 


do {

//get the inputs
	get_Input(the_input, &the_backGround, input_file, output_file, the_pid);

// for comment or blank
if(the_input[0][0] == '#' || the_input[0][0] == '\0' ){
	continue;
}

//for exit 
else if(strcmp(the_input[0] , "exit") == 0){
		first_cont = 0; 
}

//for cd 
else if( strcmp(the_input[0], "cd") == 0){
	//change to given directory 
	if(the_input[1]) {
		if (chdir(the_input[1]) == -1){
			printf("Directory not found.\n");
			fflush(stdout); 
	}
}else {
		//if the direcotry is not specified, go to ~
		chdir(getenv("HOME")); 
		}

}
	
	//for status
else if (strcmp(the_input[0], "status") == 0){
	print_Exit_Status(exit_stat);
}

//esle
else{
	commandExc(the_input, &exit_stat, sa_sigint, &the_backGround, input_file, output_file);
}

//variable resets

 the_backGround = 0; 
	 
	 output_file[0] = '\0'; 
    input_file[0] = '\0';


	
for(i = 0; the_input[i]; i++){
	the_input[i] = NULL; 

}
	 

}

while(first_cont);

return 0; 

}


 

/*****************************************************************************
 *  get_Input() prompts users and parses the input to an array of words 
 * 
 * inputs: 
 * 		char[512] arr -- output array
 * 		int* back_ground boolean 
 * 		char* input_name  the name of input file
 * 		char* output_name 	name of output file 
 * 		int pid 			pid of smallsh
 * output: 
 * 		void 
 * 

*****************************************************************************/
void get_Input(char*arr[], int* back_ground, char input_name[], char output_name[], int pid){

 
	int j; 
	char the_input[INPUT_LENGTH];

//get an input
printf(": "); 


// flush 
fflush(stdout); 

fgets(the_input, INPUT_LENGTH, stdin); 


//if we got blank then return blank 
if(!strcmp(the_input, "")){
		arr[0] = strdup(""); 
		return; 
}

//remove any newlines from given
int check; 
check = 0; 
for(int i = 0; !check && i <INPUT_LENGTH; i++){
	if (the_input[i] == '\n'){
		the_input[i] = '\0';	//double check 
		check = 1; 
	}
}

//convert inputs into single strings
const char blank[2] = " ";
char *token = strtok(the_input, blank); 


for(int i = 0; token; i++){
	 
	 //check for < to denote the input file
if (!strcmp(token, "<")) {
		token = strtok(NULL, blank); 
		strcpy(input_name, token); 
	}

	 
	//check for & for background proecess
	else if(!strcmp(token, "&")){
		*back_ground = 1;
	}
	//check for > to denote output file
	else if (!strcmp(token, ">")){
		token = strtok(NULL, blank); 
		strcpy(output_name, token); 
	}

	//else its the command
	else {
			arr[i] = strdup(token);
			// neeed to replace the $$ sig with pid
			for (int j=0; arr[i][j]; j++) {
				if (arr[i][j] == '$' && arr[i][j+1] == '$') {
					arr[i][j] = '\0';
					snprintf(arr[i], 256, "%s%d", arr[i], pid);
				}
			}
		}
		//after that 
		token = strtok(NULL, blank);
	}

}

/*****************************************************************************
print_Exit_Status - call exit status 
input - int child_exit 	
output - void
*****************************************************************************/
void print_Exit_Status(int child_exit){

if(WIFEXITED(child_exit)){
	//printf if exited by status
	printf("exit value %d\n", WEXITSTATUS(child_exit)); 
}else {
	//if the termination by the signal
	printf("terminated by signal %d\n", WTERMSIG(child_exit));
		}	

}
 

/*****************************************************************************
sigtstp_handler - use boolean logic 

 input- sig_No
 output - void
*****************************************************************************/
void SIGTSTP_handler(int sig_No){
	//chekc, n if bakcgorund is 1 then assing it to 0 then show ouput message
	if(allow_Bkground ==1 ) {
		char* msg  = "Entering foreground-only mode (& is now ignored)\n";
		write(1, msg, 49); 
		fflush(stdout); 
		allow_Bkground = 0; 
	}

	//check and if its 0 then assign it to 1 then show a massage 
	else {
		char* msg = "Exiting foreground-only mode\n";
		write (1, msg, 29); 
		fflush(stdout); 
		allow_Bkground = 1; 
	}
}



/*****************************************************************************
 *  commandexe(): executes the command that was parsed to the array arr[] var
 * 
 
 * inputs: 
 * 		char[512] arr -- array with the command info
 * 		int* child_Exit_stat -- the status of commmand 
 * 		struct sigaction sigA --   for SIGINT
 * 		int* back_ground 	-- boolen to check if it is back ground proecess or not
 * 		char input_name[] . -- name of input file
 * 		char output_name[] -- name of ouput file
 * output: 
 * 		void 
 * 
 * 
 ****************************************************************************/

void commandExc(char* arr[], int* child_Exit_stat, struct sigaction sa , int* back_ground , char input_name[], char output_name[]){
	
pid_t spawn_Pid = -5; 
int the_input;
int the_output; 
int the_result; 
 

//to fork it, following code is given code from lecture
spawn_Pid = fork(); 

switch (spawn_Pid)
{
case -1: ; 
	perror("breach!\n"); 
	exit(1); 
	break;
case 0: ;
	// process will  take ^C as default 
	//sigA.sigA_handler = SIG_DFL;
	sa.sa_handler = SIG_DFL; 


	sigaction(SIGINT, &sa, NULL); 

	//for handling the input also given code from lec
	if (strcmp(input_name, "")) {
				// then open 
				the_input = open(input_name, O_RDONLY);
				if (the_input == -1) {
					perror("Unable to open input file\n");
					exit(1);
				}

				// after that assign the var
				the_result = dup2(the_input, 0);
				if (the_result == -1) {
					perror("Unable to assign input file\n");
					exit(2);
				}
				// trigger the close
				fcntl(the_input, F_SETFD, FD_CLOEXEC);
			}


//now take care of the output, this code also from lecture
if(strcmp(output_name, "")){
	//then open
	the_output = open(output_name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (the_output == -1){
		perror("Unable to open output file\n");
				exit(1);
			}

	//after that assign the var
	the_result = dup2(the_output, 1); 
	if(the_result == -1){
		perror("Unable to assign output file\n");
					exit(2);
	}

	//trigger close
	fcntl(the_output, F_SETFD, FD_CLOEXEC);

}

	//execute
	if(execvp(arr[0], (char* const*)arr)){
		printf("%s: no such file or directory\n", arr[0]);
				fflush(stdout);
				exit(2);
	}
	break;


default:
		//execute a proecss in the background 
		// execute only if allow background 

		if(*back_ground && allow_Bkground){
			pid_t actual_Pid = waitpid(spawn_Pid, child_Exit_stat, WNOHANG); 
			printf("background pid  is %d\n", spawn_Pid);
			fflush(stdout); 
		}

		//else exectue regular
		else{
			pid_t actual_Pid = waitpid(spawn_Pid, child_Exit_stat, 0);
			
		}
		//check if there is  background porecsses that were terminated
		while ((spawn_Pid = waitpid(-1, child_Exit_stat, WNOHANG)) > 0 )
		{
			printf("child %d terminated\n", spawn_Pid);
			print_Exit_Status(*child_Exit_stat); //a func
			fflush(stdout); 
		}
   }

}

 
 
