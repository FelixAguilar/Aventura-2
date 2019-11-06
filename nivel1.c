/*
* 
*
* Authors: Aguilar Ferrer, Felix
*          Bennasar Polzin, Adrian
*          Lopez Bueno, Alvaro
*
* Date: 
*/

// Constants.
#define _POSIX_C_SOURCE 200112L
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define PROMPT '$'

// Libreries.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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
*  args: pointer to the pointers for all tokens obteined from the line.
*  line: pointer where will be stored the input introduced by stdin.
*
*  returns: the number of tokens obteined from line.
*/
int parse_args(char **args, char *line){
    
    // Count for the tokens and pointer to for each token.
    int ntoken = 0;
    char * token;

    // Checks and cleans the character "\n" at the end of the string line.
    line = strtok(line, "\n");

    // Changes all the tabs before # with blanks.
    while(strchr(line, '\t')){
        token = strchr(line, '\t');
        *(token) = ' ';
    }

    // Gets the first token and saves it in args.
    token = strtok(line, " ");
    args[ntoken] = token;

    // Prints the obteined token (temporal).
    printf("[parse_args() ->token %i = %s]\n", ntoken, args[ntoken]);

    // Loop until obteining a token that is NULL or a comment.
    while (args[ntoken]){

        // If there is a token that starts with "#" then there is a comment.
        if(*(token) == '#'){

            // Stops the search for tokens and adds the sentinel at args.
            args[ntoken] = NULL;

            // Prints de correction if there is a comment (temporal).
            printf("[parse_args() ->token %i  corregido = %s]\n", ntoken, args[ntoken]);
        }
        else{

            // It obteins the next token and move by 1 the pointer of args.
            ntoken++;
            token = strtok(NULL, " ");

            // Saves in args the obteined token.
            args[ntoken] = token;

            // Prints the obteined token (temporal).
            printf("[parse_args() ->token %i = %s]\n", ntoken, args[ntoken]);
        }
    }
    return ntoken;
}

/*
* Function: check_internal:
* -------------------------
* Checks if the first token is an internal command or not and if it is calls it
* and returns 1, or if is not an internal then returns 0 and it not calls at a 
* function.
*
*  args: pointer to the pointers for all tokens obteined from the line.
*
*  returns: 1 if it is a internal command, else 0.
*/
int check_internal(char **args){
    
    // Return value.
    int internalCom=0;

    // Internal commands.
    const char cd[] = "cd";
    const char export[]= "export";
    const char source[] = "source";
    const char jobs[] = "jobs";
    const char ex[] = "exit";
    
    //Checks if it is an internal command, updates return value and calls it.
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

    // returns if it was an internal command.
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
