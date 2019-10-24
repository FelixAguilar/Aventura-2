/*
* 
*
* Authors: Aguilar Ferrer, Felix
*          Bennasar Polzin, Adrian
*          Lopez Bueno, Alvaro
*
* Date: 
*/

//Constants.
#define _POSIX_C_SOURCE 200112L
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define PROMPT '$'

// Libreries.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    if (cmd){
        while(read_line(cmd)){
            execute_line(cmd);
        }
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

/*
 * Prepara la linea para la ejecucion y divide los tokens.
 */
int execute_line(char *line){
    char **args = malloc (sizeof(char *) * ARGS_SIZE);
    if (args){
        parse_args(args,line);
        check_internal(args);
    }
    free(args);
}

int parse_args(char **args, char *line){
    int i = 0;
    char *token = strtok(line, " ");
    args [i] = token;
    while (token != NULL){
        printf("%s\n", args[i]);
        i++;
        token = strtok(NULL, " ");
        args[i] = token;
    }
}

int check_internal(char **args){

}

int internal_cd(char **args){

}

int internal_export(char **args){

}

int internal_source(char **args){

}

int internal_jobs(char **args){

}
