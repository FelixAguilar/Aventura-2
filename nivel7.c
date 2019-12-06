/*
* This program is a minishell in which you can execute the internal commands 
* and any external command. Here the library "readline" is disabled. 
*
* Authors: Aguilar Ferrer, Felix
*          Bennasar Polzin, Adrian
*          Lopez Bueno, Alvaro
*
* Date: 15/12/2019
*/

// Comment this to not use library readline.
//#define USE_READLINE

// Constants:
#define _POSIX_C_SOURCE 200112L
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define PROMPT " > $: "
#define N_JOBS 64
#define FOREGROUND 0
#define EXECUTED 'E'
#define STOPPED 'D'
#define FINALIZED 'F'

// Libraries:
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

// Libraries for readline:
#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

// Function headers:
int print_prompt();
char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int aux_internal_cd(char *path, char c);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);
int internal_fg(char **args);
int internal_bg(char **args);
int jobs_list_add(pid_t pid, char status, char *command_line);
int jobs_list_find(pid_t pid);
int jobs_list_remove(int pos);
int is_background(char **args);
int is_output_redirection(char **args);
void reaper(int signum);
void ctrlc(int signum);
void ctrlz(int signum);

/* 
* Structure for the storage of a job:
* -----------------------------------
*  pid: number that indentifies a job.
*  status: it can be Executed, Stopped, Finalized.
*  command_line: command name and his arguments.
*/
struct info_process
{
    pid_t pid;
    char status;
    char command_line[COMMAND_LINE_SIZE];
};

// Allocates memory for the job list in execution.
static struct info_process jobs_list[N_JOBS];

// Allocates memory for the minishell information.
static struct info_process minishell;

// Allocates memory for the default foreground.
static struct info_process foreground;

// Allocates memory for the number of active jobs in the minishell.
static int active_jobs = 1;

/*
* Function: Main:
* ---------------
* Primary function of the minishell, here starts the minishell execution.
*
*  argc: number of arguments introduced.
*  argv: char array of the arguments, the name of the executed file is stored 
*        in position 0.
*
*  returns: exit_success if it was executed correctly.
*/
int main(int argc, char **argv)
{
    // Sets the necessary values to recognize the minishell.
    minishell.pid = getpid();
    minishell.status = EXECUTED;
    strcpy(minishell.command_line, argv[0]);

    // Sets the foreground default values (no active job).
    foreground.pid = FOREGROUND;
    foreground.status = EXECUTED;
    foreground.command_line[0] = '\0';

    // Sets action (reaper) for signal SIGCHILD.
    signal(SIGCHLD, reaper);

    // Sets action (ctrlc) for signal SIGINT.
    signal(SIGINT, ctrlc);

    // Sets action (ctrlz) for signal SIGTSTP.
    signal(SIGTSTP, ctrlz);

    //Initialize the foreground when there is no active job.
    jobs_list[FOREGROUND].pid = foreground.pid;
    jobs_list[FOREGROUND].status = foreground.status;
    strcpy(jobs_list[FOREGROUND].command_line, foreground.command_line);

    // Allocates memory for the input command line.
    char *line = (char *)malloc(sizeof(char) * COMMAND_LINE_SIZE);

    // Checks if malloc was done correctly.
    if (line)
    {
        // Read and execute the line.
        while (read_line(line))
        {
            execute_line(line);
        }
        // Liberates memory used by the line.
        free(line);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

/*
* Function: print_prompt:
* -----------------------
* This function is used to print the prompt.
*
*  returns: exit success or exit failure.
*/
int print_prompt(){
    char *cwd = malloc(sizeof(char) * COMMAND_LINE_SIZE);
    if (cwd)
    {
        // Gets the current work directory.
        getcwd(cwd, COMMAND_LINE_SIZE);
        printf("%s%s",cwd,PROMPT);
        free(cwd);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

/*
* Function: read_line:
* --------------------
* Prints the prompt and reads the input introduced in stdin by the user.
*
*  line: pointer where the input introduced by stdin will be stored.
*
*  returns: pointer to input introduced.
*/
char *read_line(char *line)
{
    // Allocates memory for the prompt and check if it has been able to do it.
    char *prompt = malloc(sizeof(char) * COMMAND_LINE_SIZE);
    if (prompt)
    {
        // Gets the current work directory.
        getcwd(prompt, COMMAND_LINE_SIZE);

#ifdef  USE_READLINE
        // Prints the prompt and reads the input from the user.
        strcat(prompt, PROMPT);
        char *ptr = readline(prompt);

        // If the input from the user is ctrl+D then exit the minishell.
        if(!ptr){
            printf("\r");
            exit(0);
        }
        // If the input is not empty save it into history.
        if (ptr && *ptr)
        {
            add_history(ptr);
        }
        // Copies input to line.
        strcpy(line,ptr);
#else
        // Prints the prompt and the separator.
        printf("%s%s", prompt, PROMPT);

        // Reads input introduced in stdin by the user.
        char *ptr = fgets(line, COMMAND_LINE_SIZE, stdin);

        // Searches and clears the character '\n'.
        if(strchr(line, '\n'))
        {
        char *n = strchr(line, '\n');
        *(n) = '\0';
        }
        // If the ptr is null it means that the command is Ctrl+Letter.
        if (!ptr)
        {
            // Places the console cursor at start of line.
            printf("\r");

            // Exits the minishell when reaching end of file in stdin.
            if (feof(stdin))
            {
                exit(0);
            }
            else
            {
                // To not allow that ctrl+C exits from the shell.
                ptr = line;

                // This is to avoid the "command not found" error.
                ptr[0] = 0;
            }
        }
#endif
        // Frees the memory for prompt and cleans stdin.
        free(prompt);
        fflush(stdin);

        // Returns the command line.
        return line;
    }
    return NULL;
}

/*
* Function: execute_line:
* -----------------------
* Runs the different functions that will prepare and execute the command line
* introduced by the user.
*
*  line: pointer where the input introduced by stdin is stored.
*
*  returns: exit_failure if it has failed or exit_success if it was executed
*           correctly.
*/
int execute_line(char *line)
{
    // Allocates memory for the array of pointers to arguments.
    char **args = malloc(sizeof(char *) * ARGS_SIZE);

    // Checks if it has been allocated correctly.
    if (args)
    {
        // Obtains the arguments and if there are no arguments then skip.
        if (parse_args(args, line))
        {
            // Allocates memory for the char array command and checks it.
            char *command = malloc(sizeof(char) * COMMAND_LINE_SIZE);
            if (command)
            {
                // Groups the line with all tokens.
                int i = 0;
                strcat(command, args[i]);
                i++;
                while (args[i])
                {
                    strcat(command, " ");
                    strcat(command, args[i]);
                    i++;
                }

                // Checks if it is an internal command, if not continue.
                if (check_internal(args))
                {
                    // Checks if it is a background command.
                    int bkg = is_background(args);

                    // Creates a new thread and returns the son's pid.
                    pid_t pid = fork();

                    // If it is the father process then execute this.
                    if (pid > 0)
                    {
                        // If it is a background job then add it to jobs_list.
                        if (bkg)
                        {
                            jobs_list_add(pid, EXECUTED, command);
                        }
                        else
                        {
                            // Sets values for the foreground job.
                            jobs_list[FOREGROUND].pid = pid;
                            jobs_list[FOREGROUND].status = EXECUTED;
                            strcpy(jobs_list[FOREGROUND].command_line, command);

                            // Waits until the foreground job is finished.
                            while (jobs_list[FOREGROUND].pid)
                            {
                                pause();
                            }
                            // Resets values for the foreground job.
                            jobs_list[FOREGROUND].pid = foreground.pid;
                            jobs_list[FOREGROUND].status = foreground.status;
                            strcpy(jobs_list[FOREGROUND].command_line, command);
                        }
                    }
                    // If it is the son process then execute this.
                    else if (pid == 0)
                    {
                        // Sets ignore for SIGTSTP and SIGINT.
                        signal(SIGTSTP, SIG_IGN); 
                        signal(SIGINT, SIG_IGN);  

                        // Sets standard action for SIGCHILD.
                        signal(SIGCHLD, SIG_DFL);

                        // Looks for redirection in the command line.
                        is_output_redirection(args);

                        // Executes the command introduced using args.
                        if (execvp(args[0], args))
                        {
                            // If there is an error then shows it and exits.
                            fprintf(stderr,"%s: no se encontró la orden.\n",
                                args[0]);
                            exit(EXIT_FAILURE);
                        }
                        // Exits once the command has been executed.
                        exit(EXIT_SUCCESS);
                    }
                    else
                    {
                        // Exits if an error happens when creating the son.
                        perror("fork");
                        exit(EXIT_FAILURE);
                    }
                    // Liberates memory for the command.
                    free(command);
                }
            }
        }
        // Liberates the memory for the arguments.
        free(args);
    }
    return EXIT_SUCCESS;
}

/*
* Function: parse_args:
* ---------------------
* Divides the input line into differents segments into tokens that are divided
* by blank spaces " " and elimintates the comments that are after "#".
*
*  args: pointer to the pointers for all tokens obtained from the line.
*  line: pointer where the input introduced by stdin will be stored.
*
*  returns: the number of tokens obtained from line.
*/
int parse_args(char **args, char *line)
{
    // Counter for the tokens and pointer for each token.
    int ntoken = 0;
    char *token;

    // Checks if line is empty or not.
    if (line)
    {
        // Swaps all the tabs with blanks.
        while (strchr(line, '\t'))
        {
            token = strchr(line, '\t');
            *(token) = ' ';
        }
        // Gets the first token and saves it in args.
        token = strtok(line, " ");
        args[ntoken] = token;

        // Loop until obtaining a token that is NULL or a comment.
        while (args[ntoken])
        {
            // If there is a token that starts with "#" then it is a comment.
            if (*(token) == '#')
            {
                // Stops the search for tokens and adds the sentinel at args.
                args[ntoken] = NULL;
            }
            else
            {
                // It obtains the next token and moves by 1 the pointer args.
                ntoken++;
                token = strtok(NULL, " ");

                // Saves the obtained token in args.
                args[ntoken] = token;
            }
        }
    }
    return ntoken;
}

/*
* Function: check_internal:
* -------------------------
* Checks whether the command is internal or not. If it is internal, executes 
* the command and returns exit success. Otherwise, returns exit failure.
*  
*  args: pointer to the pointers for all tokens obteined from the line.
*
*  returns: exit success or exit failure.
*/
int check_internal(char **args)
{
    // Internal commands.
    const char cd[] = "cd";
    const char export[] = "export";
    const char source[] = "source";
    const char jobs[] = "jobs";
    const char ex[] = "exit";
    const char fg[] = "fg";
    const char bg[] = "bg";

    //Checks if it is an internal command, updates return value and calls it.
    if (!strcmp(args[0], cd))
    {
        internal_cd(args);
        return EXIT_SUCCESS;
    }
    else if (!strcmp(args[0], export))
    {
        internal_export(args);
        return EXIT_SUCCESS;
    }
    else if (!strcmp(args[0], source))
    {
        internal_source(args);
        return EXIT_SUCCESS;
    }
    else if (!strcmp(args[0], jobs))
    {
        internal_jobs(args);
        return EXIT_SUCCESS;
    }
    else if (!strcmp(args[0], ex))
    {
        exit(0);
    }
    else if (!strcmp(args[0], fg))
    {
        internal_fg(args);
        return EXIT_SUCCESS;
    }
    else if (!strcmp(args[0], bg))
    {
        internal_bg(args);
        return EXIT_SUCCESS;
    }
    // Returns if it was an internal command.
    return EXIT_FAILURE;
}

/*
* Function: internal_cd:
* ----------------------
* Changes the working directory for the one introduced as parameter. If there 
* are no arguments introduced it will go to the user home. Also it will accept 
* directories with blank spaces thanks to the auxiliary function.
*
*  args: pointer to the pointers for all tokens obtained from the line.
*
*  returns: exit success.
*/
int internal_cd(char **args)
{
    // Checks if there was an argument, if not, goes to the HOME.
    if (args[1])
    {
        // Allocates memory for the path introduced as argument.
        char *path = (char *)malloc(sizeof(char) * COMMAND_LINE_SIZE);

        // Copies the first argument to the path.
        strcpy(path, args[1]);

        // Creates the path including the characters c adding blanks.
        for (int i = 2; i < ARGS_SIZE && args[i] != NULL; i++)
        {
            strcat(path, " ");
            strcat(path, args[i]);
        }
        // Checks if there are blanks and adds them again.
        aux_internal_cd(path, '\"');
        aux_internal_cd(path, '\'');
        aux_internal_cd(path, '\\');

        // Changes the working directory and checks if it was successful.
        if (chdir(path))
        {
            // Prints the error in stderr.
            perror("chdir");
        }
        // Liberates memory for the path.
        free(path);
    }
    else
    {
        // Changes the working directory and checks if it was successful.
        if (chdir(getenv("HOME")))
        {
            // Prints the error in stderr.
            perror("chdir");
        }
    }
    return EXIT_SUCCESS;
}

/*
* Function: aux_internal_cd:
* --------------------------
* Checks if there are blank spaces identified with the character c and if so, 
* it unifies the path and elimintates all characters c from the path adding 
* blank spaces between diferent tokens.
*
*  path: pointer to the string char used to store the path.
*  c: char used as identifier as a space or union.
*
*  returns: exit success or exit failure.
*/
int aux_internal_cd(char *path, char c)
{
    // Checks if there is any character c in the path.
    if (strchr(path, c))
    {
        // Allocates memory for an auxiliary variable for the path.
        char *auxpath = (char *)malloc(sizeof(char) * COMMAND_LINE_SIZE);

        // Gets the first part from the path without the character c.
        char *aux = strtok(path, &c);

        // Cleans the auxiliary path.
        strcpy(auxpath, "");

        //While there are characters c in the string path.
        while (aux)
        {
            // Adds aux to the new path and gets the next part of it.
            strcat(auxpath, aux);
            aux = strtok(NULL, &c);
        }
        // Copies the content of auxpath to the path and eliminates it.
        strcpy(path, auxpath);
        free(auxpath);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

/*
* Function: internal_export:
* --------------------------
* Changes an env variable indicated in the args with the new value.
*  
*  args: pointer to the pointers for all tokens obteined from the line.
*
*  returns: exit success or exit failure
*/
int internal_export(char **args)
{
    // Checks if it has the arguments correctly.
    if (args[1] && !args[2])
    {
        // Divides the arg 1 using the = as separator.
        strtok(args[1], "=");
        char *token = strtok(NULL, "=");

        // Checks if the estructure NAME=value was introduced correctly.
        if (args[1] && token)
        {
            // Changes the values of the env variable.
            setenv(args[1], token, 1);
            return EXIT_SUCCESS;
        }
    }
    fprintf(stderr, "Error de sintaxis. Uso: export Nombre=Valor\n");
    return EXIT_FAILURE;
}

/*
* Function: internal_source:
* --------------------------
* Allows the execution of multiple predefined commands contained in a script
* file. 
*
*  args: pointer to the pointers for all tokens obtained from the line.
*
*  returns: exit success or exit failure
*/
int internal_source(char **args)
{
    // Allocates memory for the line.
    char *text = (char *)malloc(sizeof(char) * COMMAND_LINE_SIZE);
    if (text)
    {
        // Open a file in reading mode.
        FILE *fp = fopen(args[1], "r");
        if (fp)
        {
            // Obtain the lines one by one until reaches the end of file.
            while (fgets(text, COMMAND_LINE_SIZE, fp))
            {
                // Cleans the buffer after each reading iteration.
                fflush(fp);

                // Search and clears the character '\n'.
                if(strchr(text, '\n'))
                {
                    char *n = strchr(text, '\n');
                    *(n) = '\0';
                }
                execute_line(text);
            }
            // Closes the file and frees the memory ocupied by line.
            fclose(fp);
            free(text);
            return EXIT_SUCCESS;
        }
        else
        {
            // If there was a problem we notify it.
            fprintf(stderr, "El archivo no existe o no se puede abrir.\n");

            // Frees the allocated memory if an error occured aswell.
            free(text);
        }
    }
    return EXIT_FAILURE;
}

/*
* Function: internal_jobs:
* ------------------------
* Prints all active jobs in background with their pid, state, and command line.
*  
*  args: pointer to the arguments line.
*  
*  returns: exit success
*/
int internal_jobs(char **args)
{

    // Traverses the jobs_list and prints each job.
    int ind = 1;
    while (ind < active_jobs)
    {
        printf("[%d] %d\t%c\t%s\n", ind, jobs_list[ind].pid,
               jobs_list[ind].status, jobs_list[ind].command_line);
        ind++;
    }
    return EXIT_SUCCESS;
}

/*
* Function: internal_fg:
* ----------------------
* Moves a job to the foreground and pauses until it ends.
* 
*  args: pointer to the arguments form the command.
*
*  returns: exit success or exit failure.
*/
int internal_fg(char **args)
{
    // If the command was correctly introduced.
    if (args[1])
    {
        // Gets the index for the job and checks if it is valid.
        int job = (int)*(args[1]) - 48;
        if (job > 0 && job < active_jobs)
        {
            // If the job is stopped then sends continue signal to it.
            if (jobs_list[job].status == STOPPED)
            {
                kill(jobs_list[job].pid, SIGCONT);
            }
            // Updates foreground information with the job.
            jobs_list[FOREGROUND].pid = jobs_list[job].pid;
            jobs_list[FOREGROUND].status = jobs_list[job].status;
            strcpy(jobs_list[FOREGROUND].command_line,
                   jobs_list[job].command_line);

            // Removes the old job from the job_list.
            jobs_list_remove(job);

            // If it contains the char '&' it is removed.
            char *pos = strchr(jobs_list[FOREGROUND].command_line, '&');
            if (pos)
            {
                *(pos - 1) = '\0';
            }
            // Prints the command line.
            printf("%s\n", jobs_list[FOREGROUND].command_line);

            // Waits for the job to finish.
            while (jobs_list[FOREGROUND].pid)
            {
                pause();
            }
            return EXIT_SUCCESS;
        }
        fprintf(stderr, "El trabajo %d no existe.\n", job);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "La sintaxis es erronea, fg n_job\n");
    return EXIT_FAILURE;
}

/*
* Function: internal_bg:
* ----------------------
* Continues any job stopped in background. 
* 
*  args: pointer to the arguments of the command.
*
*  returns: exit success or exit failure.
*/
int internal_bg(char **args)
{
    // Checks if the command was introduced correctly.
    if (args[1])
    {
        // Gets the index for the job and checks if it is valid.
        int job = (int)*(args[1]) - 48;
        if (job > 0 && job < active_jobs)
        {
            // Checks if the job is stopped.
            if (jobs_list[job].status == STOPPED)
            {
                // Adds " &\0" to the command line and updates the job.
                strcat(jobs_list[job].command_line, " &\0");
                jobs_list[job].status = EXECUTED;

                // Sends the signal to continue the job.
                kill(jobs_list[job].pid, SIGCONT);
                ;
                return EXIT_SUCCESS;
            }
            fprintf(stderr, "El trabajo %d ya está en 2º plano.\n", job);
            return EXIT_FAILURE;
        }
        fprintf(stderr, "El trabajo %d no existe.\n", job);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "La sintaxis es errónea, bg n_job.\n");
    return EXIT_FAILURE;
}

/*
* Function: jobs_list_add:
* ------------------------
* Adds a new job to the last position of the jobs_list and updates active_jobs. 
* 
*  pid: the pid of the process to add.
*  status: the status of the process to add.
*  command_line: the command_line of the process to add.
* 
*  returns: exit success or exit failure.
*/
int jobs_list_add(pid_t pid, char status, char *command_line)
{
    // If jobs_list is not full then.
    if (active_jobs < N_JOBS)
    {
        // Adds the new job.
        jobs_list[active_jobs].pid = pid;
        jobs_list[active_jobs].status = status;
        strcpy(jobs_list[active_jobs].command_line, command_line);

        // Updates the active jobs.
        active_jobs++;
        return EXIT_SUCCESS;
    }
    else
    {
        fprintf(stderr, "No se pueden añadir mas trabajos a la lista.\n");
        return EXIT_FAILURE;
    }
}

/*
* Function: jobs_list_find:
* -------------------------
* Finds and returns the position of the job in the array jobs_list.
*
*  pid: pid from the process to find.
*
*  returns: the position of the process, else -1.
*/
int jobs_list_find(pid_t pid)
{
    int position = 0;

    // Search for the job with the same pid as the one introduced.
    while (position < N_JOBS && pid != jobs_list[position].pid)
    {
        position++;
    }
    // If it was not found then returns -1.
    if (position == N_JOBS)
    {
        return -1;
    }
    return position;
}

/*
* Function: jobs_list_remove:
* ---------------------------
* Removes a job from the list and adds the last job active in his positon.
*
*  position: position of the job to remove.
*
*  returns: exit success or exit failure
*/
int jobs_list_remove(int position)
{
    // Checks for a valid position.
    if (0 < position && position < N_JOBS)
    {
        // Gets the info of the last active job.
        pid_t pid_last = jobs_list[active_jobs - 1].pid;
        char status_last = jobs_list[active_jobs - 1].status;
        char *command_line_last = jobs_list[active_jobs - 1].command_line;

        // Overwrites the job of the specified position with the last job.
        jobs_list[position].pid = pid_last;
        jobs_list[position].status = status_last;
        strcpy(jobs_list[position].command_line, command_line_last);

        // Updates the active jobs.
        active_jobs--;
        return EXIT_SUCCESS;
    }
    else
    {
        // Prints error.
        fprintf(stderr, "La posición introducida es errónea.\n");
        return EXIT_FAILURE;
    }
}

/*
* Function: is_background:
* ------------------------
* Checks if is a background process.
*
*  args: arguments from the command. 
*
*  returns: exit success or exit failure.
*/
int is_background(char **args)
{
    int ind = 0;

    // Search for the last argument.
    while (args[ind + 1])
    {
        ind++;
    }
    // If the last argument contains '&' returns exit failure.
    if (!strcmp(args[ind], "&"))
    {
        args[ind] = NULL;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/*
* Function: is_output_redirection:
* --------------------------------
* Checks if there is '>' in the arguments and if so changes it with NULL 
* and obtains the file name in the argument after the '>' where the output
* of the command will be saved. 
*
*  args: char pointer of the tokens from de command line.
*
*  returns: true(0) if there is redirection, false(1) if not.
*/
int is_output_redirection(char **args)
{
    // Traverses the arguments until the token NULL.
    int ind = 0;
    while (args[ind])
    {
        // If it finds the token that contains '>' and the next token != NULL.
        if (!strcmp(args[ind], ">") && args[ind + 1])
        {
            args[ind] = NULL;

            // Opens the file and links it with stdout.
            int fd = open(args[ind + 1],
                          O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            dup2(fd, 1);
            close(fd);

            return 1;
        }
        ind++;
    }
    return 0;
}

/*
* Function: reaper:
* -----------------
* Executed when a son terminates and then updates jobs_list. 
*
*  signum: number of the signal.
*
*  returns: void.
*/
void reaper(int signum)
{
    // Variables for the ending job.
    int status;
    pid_t pid;

    // Checks if a job has ended.
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        // If it is a foreground job.
        if (pid == jobs_list[FOREGROUND].pid)
        {
            // Sets the job_list[foreground] as it was before.
            jobs_list[FOREGROUND].pid = foreground.pid;
            jobs_list[FOREGROUND].status = foreground.status;
            strcpy(jobs_list[FOREGROUND].command_line, foreground.command_line);
        }
        // If it is a background job.
        else
        {
            // Looks for his position in the list.
            int pos = jobs_list_find(pid);
            printf("\nTerminado PID %d (%s) en jobs_list[%d] con status %d\n",
                   pid, jobs_list[pos].command_line, pos, status);

            // Remove the job from the list.
            jobs_list_remove(pos);
        }
    }
    // Sets the signal SIGCHLD to the reaper function.
    signal(SIGCHLD, reaper);
}

/*
* Function: ctrlc:
* ----------------
* Executed when a Ctrl+C is pressed. 
*
*  signum: number of the signal.
*
*  returns: void.
*/
void ctrlc(int signum)
{

    printf("\n");

    #ifdef USE_READLINE
    print_prompt();
    #endif
    // Check if there is a job in foreground.
    if (jobs_list[FOREGROUND].pid > foreground.pid)
    {
        // Checks if it is not the minishell.
        if (strcmp(jobs_list[FOREGROUND].command_line, minishell.command_line))
        {
            // If it is not the minishell then send SIGTERM to the job.
            kill(jobs_list[FOREGROUND].pid, SIGTERM);
        }
    }
    // Sets SIGINT to the function ctrlc.
    signal(SIGINT, ctrlc);
}

/*
* Function ctrlz:
* ---------------
* Executed when Ctrl+Z is pressed. This function stops the foreground job
* and allows the user to input new commands.
*
*  signum: number of the signal.
*
*  returns: void
*/
void ctrlz(int signum)
{

    printf("\n");

    #ifdef USE_READLINE
    print_prompt();
    #endif

    // Check if there is a foreground job.
    if (jobs_list[FOREGROUND].pid != foreground.pid)
    {
        // Checks if the foreground is not the minishell.
        if (strcmp(jobs_list[FOREGROUND].command_line, minishell.command_line))
        {
            // Sends the signal to stop to the foreground job.
            kill(jobs_list[FOREGROUND].pid, SIGSTOP);

            // Updates the stopped job and adds it to the jobs queue.
            jobs_list[FOREGROUND].status = STOPPED;
            jobs_list_add(jobs_list[FOREGROUND].pid,
                          jobs_list[FOREGROUND].status,
                          jobs_list[FOREGROUND].command_line);

            // Updates the foreground with the default foreground properties.
            jobs_list[FOREGROUND].pid = foreground.pid;
            jobs_list[FOREGROUND].status = foreground.status;
            strcpy(jobs_list[FOREGROUND].command_line, foreground.command_line);
        }
    }
    // Sets SIGSTP to the function ctrlz.
    signal(SIGTSTP, ctrlz);
}