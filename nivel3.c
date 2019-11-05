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
#include <errno.h>
#include <string.h>

// Function headers.
char * read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int aux_internal_cd(char *path, char c);
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
    if (prompt){

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
    }
    return 0;
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
    while(strchr(line, '\t') < strchr(line, '#')){
        token = strchr(line, '\t');
        *(token) = ' ';
    }

    // Gets the first token and saves it in args.
    token = strtok(line, " ");
    args[ntoken] = token;

    // Loop until obteining a token that is NULL or a comment.
    while (args[ntoken]){

        // If there is a token that starts with "#" then there is a comment.
        if(*(token) == '#'){

            // Stops the search for tokens and adds the sentinel at args.
            args[ntoken] = NULL;
        }
        else{

            // It obteins the next token and move by 1 the pointer of args.
            ntoken++;
            token = strtok(NULL, " ");

            // Saves in args the obteined token.
            args[ntoken] = token;
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

/*
* Function: internal_cd:
* ----------------------
* Changes the working directory for the one introduced as parameter. If there 
* is no arguments introduced it will go to the user home. Also it will acept 
* directory with blank spaces thanks to the auxiliary function.
*
*  args: pointer to the pointers for all tokens obteined from the line.
*
*  returns: 0 is it was executed correctly, -1 if an error has been produced.
*/
int internal_cd(char **args){

    // Allocates memory to show the working directory (temporal).
    char *pwd = (char *) malloc (sizeof(char) * COMMAND_LINE_SIZE);

    // Checks if there was an argument, if not, goes to the HOME.
    if(args[1]){

        // Allocates memory for the path introduced as argument.
        char *path = (char *) malloc (sizeof(char) * COMMAND_LINE_SIZE);

        // Copies the first argument to the path.
        strcpy(path, args[1]);
        
        // Creates the path including the characters c adding blanks.
        for (int i = 2; i < ARGS_SIZE && args[i] != NULL; i++){
            strcat(path, " ");
            strcat(path, args[i]);
        }

        // If there was blanks indicated by any character and treats them. 
        aux_internal_cd(path, '\"');
        aux_internal_cd(path, '\'');
        aux_internal_cd(path, '\\');

        printf("%s", path);
        
        // Changes the working directory and checks if it was successful. 
        if(chdir(path)){

            // Prints the error in stderr.
            fprintf(stderr, "chdir(): %s\n", strerror(errno));
        }else{

            // To show how it changes. (temporal).
            getcwd(pwd, COMMAND_LINE_SIZE);
            printf("[internal_cd()-> %s]\n", pwd);
        }

        // Liberates memory for the path.
        free (path);
    }else{

        // Changes the working directory and checks if it was successful.
        if(chdir(getenv("HOME"))){

            // Prints the error in stderr.
            fprintf(stderr, "chdir(): %s\n", strerror(errno));
        }else{
            
            // To show how it changes. (temporal).
            printf("[internal_cd()-> %s]\n", pwd);
            getcwd(pwd, COMMAND_LINE_SIZE);
        }
    }

    // Liberates memory for the pwd (temporal).
    free(pwd);
    return 0;
}

/*
* Function: aux_internal_cd:
* --------------------------
* Checks if there is blank spaces identified with the character c and if there 
* are it unifies the path and elimintates all characters c from the path adding 
* blank spaces between diferent tokens.
*
*  args: pointer to the pointers for all tokens obteined from the line.
*  path: pointer to the string char used to store the path.
*  c: char used as identifier as a space or union.
*
*  returns: 0 is it was executed correctly, -1 if an error has been produced.
*/
int aux_internal_cd(char *path, char c){

    // Checks if there is any character c in the path.
    if (strchr(path, c)){
        
        // Allocates for an auxiliary variable for the path.
        char *auxpath = (char *) malloc(sizeof(char) * COMMAND_LINE_SIZE);

        // Gets the first part from the path with out the character c.
        char *aux = strtok(path, &c);
        
        // Cleans the auxiliary path.
        strcpy(auxpath, "");

        //While there are characters c in the string path.
        while (aux){

            // Adds aux to the new path and gets the next part of it.
            strcat(auxpath, aux);
            aux = strtok(NULL, &c);
        }

        // Copies the content of auxpath to the path and eliminates it.
        strcpy(path, auxpath);
        free(auxpath);
        return 0;
    }
    return -1;        
}

/*
* Function: internal_export:
* --------------------------
* Changes an env variable indicated in the args with the new value.
*  
*  args: pointer to the pointers for all tokens obteined from the line.
*
*  returns: 0 is it was executed correctly, -1 if an error has been produced.
*/
int internal_export(char **args){

    // Checks if it have the arguments correctly.
    if(args[1] && !args[2]){

        // Divides the arg 1 using the = as separator.
        strtok(args[1],"=");
        char *token = strtok(NULL,"="); 

        // Checks if the estructure NAME=value was introduced correctly.
        if(args[1] && token){ 

            // Prints values introduced and the current one (temporal).    
            printf("nombre: %s\n",args[1]);
            printf("valor: %s\n", token);
            printf("antiguo valor: %s\n", getenv(args[1]));

            // Changes the values of the env variable.
            setenv(args[1],token,1);

            // Prints the new value of the env variable (temporal).
            printf("nuevo valor: %s\n", getenv(args[1]));
            return 0;
        }
    }   
    fprintf(stderr, "Error de sintaxis. Uso: export nombre=valor\n");
    return -1;
}

int internal_source(char **args){
    FILE *fp;
    char *line;
    fp = fopen(args[1],r); // Abre el fichero script en modo lectura (modo r)
    if(fp == NULL){
        fprintf(stderr, "File does not exist or can not be opened\n");
        exit(EXIT_FAILURE);
    }else{
        fgets(line,COMMAND_LINE_SIZE, fp);
        while(line != NULL){
            execute_line(line);
            fflush(fp);
            fgets(line,COMMAND_LINE_SIZE, fp);
        }
    }
    fclose(fp);
    return 0;
}

int internal_jobs(char **args){
    printf("This function will show the PID of the processes that are not in the foreground.\n");
    return 0;
}
