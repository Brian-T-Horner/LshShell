/* Implemententation of lsh shell from brennan.io/2015/01/16/wire-a-shell-in-c/ */

// Imports
#include <stdio.h> // fprintf(), printf(), stderr, getchar(), perror()
#include <stdlib.h> // malloc(), realloc(), free(), exit(), execvp()
#include <unistd.h> // chdir(), fork(), exec(), pid_t
#include <sys/wait.h> // waitpid() and associated macros
#include <string.h> // strcmp(), strtok()

// Definitions
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
#define LSH_RL_BUFSIZE 1024


/* Function declarations for builtin shell commands */
int lsh_cd(char **);
int lsh_help(char **);
int lsh_exit(char **);
int lsh_num_builtins();

/* Function declarations for run time shell helpers */
int lsh_execute(char **);
char **lsh_split_line(char *);
char * lsh_readline(void);
void lsh_loop(void);
int lsh_launch(char **);
char *lsh_read_line2(void);

/* List of builtin commands*/
char *builtin_str[] = {
	"cd",
	"help",
	"exit"
};


int (*builtin_func[]) (char **) = {
	&lsh_cd,
	&lsh_help,
	&lsh_exit
};



int main(int argc, char * argv[]){
	// Load config files
	
	// Run command loop to take inputs
	lsh_loop();

	//Prefom cleanup
	

	return EXIT_SUCCESS;
}


/*
 *  List builtin command functions
 *
 */


/* Function to return the number of builtin functions */
int lsh_num_builtins(){
	return sizeof(builtin_str) / sizeof(char *);
}

/* Function to change directory */
int lsh_cd(char **args){
	if(args[1] == NULL){
		fprintf(stderr, "lsh:expected argument to \"cd\"\n");
	}else{
		if(chdir(args[1]) != 0) {
			perror("lsh");
		}
	}
	return 1;
}


/* Function for users to get help information w/ included builtin functions */
int lsh_help(char ** args) {

	printf(" Brian Horner's version of Stephen Brenna's LSH shell\n");
	printf("Type program names and arguments, and hit enter.\n");
	printf("The following are built in:\n");

	for(int i = 0; i< lsh_num_builtins(); i++){
		printf(" %s\n", builtin_str[i]);
	}

	printf("Use the man comman for information on other programs.\n");
	return 1;
}

/* Function to exit the shell */
int lsh_exit(char **args){
	return 0;
}









/*
 * Shell running procedures
 *
 */



/* Function to execute the shell */
int lsh_execute(char **args){
	if(args[0] == NULL){ // An empty command was entered
		return 1;
	}

	for(int i=0; i < lsh_num_builtins(); i++){
		if(strcmp(args[0], builtin_str[i]) == 0){ //If command matches a builtin function using string compare
			return(*builtin_func[i])(args); // Pass the arguments to that function
		}
	}
	return lsh_launch(args);
}

/* Function taking list of arguments, forking the process and runs the processes w/ error checking */
int lsh_launch(char **args){
	pid_t pid, wpid;
	int status;
	
	pid = fork();
	if(pid == 0){ // If it is the child process
		if(execvp(args[0], args) == -1){ // if returns -1 or at all there is an error
			perror("lsh"); // print system error with program name
		}
		exit (EXIT_FAILURE);
	}else if (pid < 0){ //error forking
		perror("lsh"); // print system error with program name
	}else{ // parent process & fork was sucessful
	   do{
		   wpid = waitpid(pid, &status, WUNTRACED); //wait til the process's state to change 
	   }while (WIFEXITED(status) && WIFSIGNALED(status)); // wait until processes are exited or killed
	}
	return 1;

}


/*Parse the line given by readline into arguments for execution*/
char **lsh_split_line(char *line){
	int bufsize = LSH_TOK_BUFSIZE;
	int position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if(!tokens){
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, LSH_TOK_DELIM);
	while(token != NULL){
		tokens[position] = token;
		position++;
		
		if(position >= bufsize){
			bufsize += LSH_TOK_BUFSIZE;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if(!tokens){
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, LSH_TOK_DELIM);
		// free(token)???
	}
	tokens[position]=NULL;
	return tokens;
}


/* General shell loop */
void lsh_loop(void){
	char *line;
	char **args;
	int status;

	do {
		
        	printf(":) ");
		line = lsh_readline(); // Read input
		args = lsh_split_line(line); // Parse input
		status = lsh_execute(args); // Execute commands
		
		free(line); // Free memory
		free(args); // Free memory
	   } while (status);
}


/*Read a line from stdin with getline. Newer implementation*/
char* lsh_read_line2(void){
	char *line = NULL;
	ssize_t bufsize = 0; // have getline allocate a buffer for us
	
	if(getline(&line, &bufsize, stdin) ==1){
		if(feof(stdin)){
			exit(EXIT_SUCCESS); // we found EOF
		} else {
			perror("readline");
				exit(EXIT_FAILURE);
	
		}
	}
	return line;
}


/*Read a line from stdin w/ realloc for higher buffer size*/
// TODO: refactor with getline()
char *lsh_readline(void){
	// Declarations
	int bufsize = LSH_RL_BUFSIZE;
	int position = 0;
	char * buffer = malloc(sizeof(char) *bufsize); 
	int c;

	if(!buffer){
		fprintf(stderr, "lsh:allocation error\n");
		exit(EXIT_FAILURE);
	}

	while(1){
		c = getchar(); // read a character
		
		// If we hit an EOF replace with \n and return
		if(c == EOF || c == '\n'){
			buffer[position] = '\0';
			return buffer;
		}else{
			buffer[position] = c;
		}
		position++;

		// If we exceeded the buffer, reallocate
		if(position >= bufsize){
			bufsize += LSH_RL_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			if(!buffer){
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}
