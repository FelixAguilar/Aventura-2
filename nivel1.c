/*
* 
*
* Authors: Aguilar Ferrer, Felix
*          Bennasar Polzin, Adrian
*          Lopez Bueno, Alvaro
*
* Date: 
*/

// Libreries.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//Constants.
#define _POSIX_C_SOURCE 200112L
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define PROMPT '$'

// Function headers.
char * read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);

/*
* Here starts the program.
*/
int main(){
    char *cmd = (char *) malloc (sizeof(char) * COMMAND_LINE_SIZE);
    while(read_line(cmd)){

    }
    return 0;
}

/*
* Function: read_line:
* --------------------
* Prints the prompt and reads the input introduced by stdin.
*
*  line: pointer where will be stored the input introduced by stdin.
*
*  returns: pointer of input introduced.
*/
char * read_line(char *line){
    char *prompt = malloc (sizeof(char) * COMMAND_LINE_SIZE);
    if (prompt)
    {
        getcwd(prompt,COMMAND_LINE_SIZE);
        printf ("%s %c ",prompt, PROMPT);
        fgets(line, COMMAND_LINE_SIZE, stdin);
        free(prompt);
        return line;
    }
}

int execute_line(char *line){

}

int parse_args(char **args, char *line){
    
}

int check_internal(char **args){

}

int internal_cd(char **args){
    printf("This function will change the directory.\n")
    return 0;


}

int internal_export(char **args){
    printf("This function will asign values to environment variables.\n")
    return 0;

}

int internal_source(char **args){
    printf("This function will execute a command line file.\n")
    return 0;

}

int internal_jobs(char **args){
    printf("This function will show the PID of the processes that are not in 
    the foreground.\n")
    return 0;

}