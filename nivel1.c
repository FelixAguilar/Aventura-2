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
#include <string.h>

// Constants.
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
* Main program, here starts the execution.
*/
int main(){
    
    // Allocates memory for the input command line.
    char *cmd = (char *) malloc (sizeof(char) * COMMAND_LINE_SIZE);
    
    // If there is enough memory for cmd then execute the loop.
    if (cmd){
        
        // Read line and execute it.
        while(read_line(cmd)){
                 execute_line(cmd);
        }
    }
    return 0;
}

/*
* Function: read_line:
* --------------------
* Prints the prompt and reads the input introduced in stdin.
*
*  line: pointer where will be stored the input introduced by stdin.
*
*  returns: pointer of input introduced.
*/
char * read_line(char *line){
    
    // Allocates memory for the prompt and check if it has been able to do it.
    char *prompt = malloc (sizeof(char) * COMMAND_LINE_SIZE);
    if (prompt)
    {
        // Gets the current work directory.
        getcwd(prompt,COMMAND_LINE_SIZE);
        
        // Prints the prompt and the separator.
        printf ("%s %c ",prompt, PROMPT);
        
        // Reads input introduced in stdin by the user.
        fgets(line, COMMAND_LINE_SIZE, stdin);
        
        // frees the memory for prompt.
        free(prompt);
    }
    return line;
}

/*
* Function: execute_line:
* --------------------
* runs the different functions that will prepare and execute the command line
* introduced by the user.
*
*  line: pointer where will be stored the input introduced by stdin.
*
*  returns: -1 if it has failed or 0 if it was executed correctly.
*/
int execute_line(char *line){
    
    // Allocates memory for the pointers to tokens.
    char **args = malloc (sizeof(char *) * ARGS_SIZE);
    
    // Checks if it has been done correctly.
    if (args){
        
        // Obteins all arguments in the line.
        parse_args(args,line);
        
        // If there is no arguments then skip the execution.
        if(args[0]){
        check_internal(args);
        }
        
        // liberates the memory for tha arguments.
        free(args);
        return 0;
    }
    return -1;
}

/*
* Function: parse_args:
* ---------------------
* Divides the input line into differents segments into tokens that are divided
* by blank spaces " " and elimintates the comments that are after "#".
*
*  line: pointer where will be stored the input introduced by stdin.
*
*  returns: -1 if it has failed or 0 if it was executed correctly.
*/
int parse_args(char **args, char *line){
    
    // Count for the tokens and pointer to the begining for each token.
    int ntoken = 0;
    char * token;

    line = strtok(line, "\n");
    token = strtok(line, " ");
    args [ntoken] = token;
    while (token != NULL){
        printf("[parse_args() ->token %i = %s]\n", ntoken, args[ntoken]);
        ntoken++;
        token = strtok(NULL, " ");
        args[ntoken] = token;
    }
    return ntoken;
}

int check_internal(char **args){
    int internalCom=0;
    const char cd[] = "cd";
    const char export[]= "export";
    const char source[] = "source";
    const char jobs[] = "jobs";
    const char ex[] = "exit";
    
    if(!strcmp(args[0],cd)){
        internal_cd(args);
        internalCom=1;
    }
    else if(!strcmp(args[0],export)){
        internal_export(args);
        internalCom=1;
    }
    else if(!strcmp(args[0],source)){
        internal_source(args);
        internalCom=1;
    }
    else if(!strcmp(args[0],jobs)){
        internal_jobs(args);
        internalCom=1;
    }
    else if(!strcmp(args[0],ex)){
        exit(0);
    }

    return internalCom;
}

int internal_cd(char **args){
    printf("This function will change the directory.\n");
    return 0;


}

int internal_export(char **args){
    printf("This function will asign values to environment variables.\n");
    return 0;

}

int internal_source(char **args){
    printf("This function will execute a command line file.\n");
    return 0;

}

int internal_jobs(char **args){
    printf("This function will show the PID of the processes that are not in the foreground.\n");
    return 0;

}
