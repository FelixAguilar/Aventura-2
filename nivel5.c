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
#define N_JOBS 64

// Libreries.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

// Structure for the storage of a process.
struct info_process {
    pid_t pid;
    char status; // ’E’, ‘D’, ‘F’
    char command_line[COMMAND_LINE_SIZE]; // Command
};

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
void reaper(int signum);
void ctrlc(int signum);

// Creates the struct for the job list.
static struct info_process jobs_list[N_JOBS];

// Global variable for number of non-terminated background processes
static int n_pids = 1;

/*
* Main program, here starts the execution.
*/
int main ( int argc, char **argv ) {
    
    // Sets action (reaper) for signal SIGCHILD.
    signal(SIGCHLD, reaper);
    
    // Sets action (ctrlc for signal SIGINT.
    signal(SIGINT, ctrlc);
    
    //Inicialize tha fields for jobs_list[0].
    jobs_list[0].pid = 0;
    jobs_list[0].status = 'E';
    strcpy(jobs_list[0].command_line, argv[0]);
    printf("%s", jobs_list[0].command_line);
    
    // Allocates memory for the input command line.
    char *cmd = (char *) malloc (sizeof(char) * COMMAND_LINE_SIZE);
    
    // If there is enough memory for cmd then execute the loop.
    if (cmd){
        
        // Read line and execute it.
        while(read_line(cmd)){
            strcpy(jobs_list[0].command_line, cmd);
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

            // Checks if it is an internal command if not then execute this.
            if(check_internal(args)){

                // Creates a new thread and obteins his pid.
                pid_t pid = fork();

                // If it is the father then execute this.
                if (pid > 0){

                    
                    int status;
                    
                    // Waits until the sons ends with pause.
                    jobs_list[0].pid = pid;
                    while (jobs_list[0].pid) {
                        pause();
                    }

                    // If was finished with exit.
                    if(WIFEXITED(status)){
                        printf("[Proceso hijo %d finalizado con exit(), estado: %d]\n", pid, WEXITSTATUS(status));
                    }
                    
                    // If was finished with a signal.
                    else if(WIFSIGNALED(status)){
                        printf("[Proceso hijo %d finalizado por señal, estado: %d]\n", pid, WTERMSIG(status));
                    }
                }
                
                // If it is the son execute this.
                else if (pid == 0){
                    printf("[execute_line()→ PID padre: %d]\n", getppid());
                    printf("[execute_line()→ PID hijo: %d]\n", getpid());
                    
                    // Sets standard action for SIGCHILD.
                    signal(SIGCHLD, SIG_DFL);
                    
                    // Sets ignore for SIGINT.
                    signal(SIGINT, SIG_IGN);
                    
                    // Executes the command introduced using args.
                    if(execvp(args[0], args)){

                        // if there is an error then shows it and exits.
                        perror(args[0]);
                        exit(EXIT_FAILURE);
                    }

                    // Else exit.
                    exit(EXIT_SUCCESS);
                }

                // If an error ocurred creating the son then error and exit.
                else{
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
            }
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

    // Checks if line is empty or not.
    if (line)
    {

        // Changes all the tabs with blanks.
        while(strchr(line, '\t')){
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
*  returns: 0 if it is a internal command, else -1.
*/
int check_internal(char **args){
    
    // Return value.
    int internalCom = -1;

    // Internal commands.
    const char cd[] = "cd";
    const char export[]= "export";
    const char source[] = "source";
    const char jobs[] = "jobs";
    const char ex[] = "exit";
    
    //Checks if it is an internal command, updates return value and calls it.
    if(!strcmp(args[0],cd)){
        internal_cd(args);
        internalCom = 0;
    }
    else if(!strcmp(args[0],export)){
        internal_export(args);
        internalCom = 0;
    }
    else if(!strcmp(args[0],source)){
        internal_source(args);
        internalCom = 0;
    }
    else if(!strcmp(args[0],jobs)){
        internal_jobs(args);
        internalCom = 0;
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
            perror("chdir");
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
            perror("chdir");
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

/*
* Function: internal_source:
* --------------------------
* Allows the execution of multiple predefined commands contained in a script
* file. 
*  args: pointer to the pointers for all tokens obtained from the line.
*
*  returns: EXIT_SUCCES if executed correctly or EXIT_FAILURE if there was a 
*  problem.
*/
int internal_source(char **args){
    // allocates memory for a variable where the command to be executed
    // will be stored
    char *line = (char *) malloc(sizeof(char)*COMMAND_LINE_SIZE); 
    if(line){
        // opens a file in reading mode
        FILE *fp = fopen(args[1],"r");
        if(fp){
            // obtains the lines one by one and we execute them calling the 
            // execute_line method.
            while(fgets(line,COMMAND_LINE_SIZE, fp)){
                execute_line(line);
                // empties the buffer after reading each line to avoid
                // problems when a wrong written command is read from the file.
                fflush(fp);
            }
            // the file is closed and we free the memory that was reserved for
            // the line variable.
            fclose(fp);
            free(line);
            return EXIT_SUCCESS;
        }else{
            // if there was a problem we notify it.
            fprintf(stderr, "File does not exist or can not be opened\n");
            // frees the allocated memory if there was an error aswell.
            free(line);
        }
    }
    return EXIT_FAILURE;
}

int internal_jobs(char **args){
    printf("This function will show the PID of the processes that are not in the foreground.\n");
    return 0;
}

int jobs_list_add(pid_t pid, char status, char *command_line){
    if(n_pids < N_JOBS){
        jobs_list[n_pids].pid = pid;
        jobs_list[n_pids].status = status;
        strcpy(jobs_list[n_pids].command_line,command_line);
        n_pids++;
    }else{
        fprintf(stderr, "No more jobs can be added because the maximun has been reached\n");
    }
}

int jobs_list_find(pid_t pid){
        int position = 0;
        while(pid != jobs_list[position].pid){
            position++;
        }
        return position;
}

int  jobs_list_remove(int pos){
        if(0 < pos < N_JOBS-1){
            // saves the info of the last job
            pit_t pidUltimo = jobs_list[n_pids-1].pid;
            char statusUltimo = jobs_list[n_pids-1].status;
            char *command_line_ultimo;
            strcpy(command_line_ultimo, jobs_list[n_pids-1].command_line); 
            // overwrites the job of the specified position
            jobs_list[pos].pid = pidUltimo;
            jobs_list[pos].status = statusUltimo;
            strcpy(jobs_list[pos].command_line,command_line_ultimo);
            n_pids--;
        }else{
            fprintf(stderr,"The specified position is not correct");
        }
        
}



void reaper(int signum){
    
    signal(SIGCHLD, reaper);
}

void ctrlc(int signum){
    
    signal(SIGINT, ctrlc);
}
