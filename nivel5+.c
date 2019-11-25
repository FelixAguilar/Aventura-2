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
#define PROMPT "> $:"
#define N_JOBS 64
#define FOREGROUND 0
#define EXECUTED 'E'
#define STOPPED 'D'
#define FINALIZED 'F'

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

/* 
* Structure for the storage of a process:
* ---------------------------------------
*  pid: Number that indentifies a process.
*  status: It can be Executed, Stopped, Finalized.
*  command_line: Comand name and his arguments.
*/
struct info_process
{
    pid_t pid;
    char status;
    char command_line[COMMAND_LINE_SIZE];
};

// Function headers.
char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int aux_internal_cd(char *path, char c);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);
int jobs_list_add(pid_t pid, char status, char *command_line);
int jobs_list_find(pid_t pid);
int jobs_list_remove(int pos);
int is_background(char **args);
void reaper(int signum);
void ctrlc(int signum);
void ctrlz(int signum);
int internal_fg(char **args);
int internal_bg(char **args);

// Allocates memory for the job list.
static struct info_process jobs_list[N_JOBS];

// Allocates memory for the name of the process.
static struct info_process minishell;

// Allocates memory for the default foreground.
static struct info_process foreground;

// Allocates memory for the active jobs in the minishell, (at least 1).
static int active_jobs = 1;

/*
* Function: Main
* --------------
* Here starts the execution.
*
*  argc: number of arguments introduced.
*  argv: char array of the arguments, in 0 is the name of the executed file.
*
*  returns: 0 if it was executed correctly.
*/
int main(int argc, char **argv)
{
    // Sets all values for the minishell process and default foreground.
    minishell.pid = getpid();
    minishell.status = EXECUTED;
    strcpy(minishell.command_line, argv[0]);
    foreground.pid = FOREGROUND;
    foreground.status = EXECUTED;
    foreground.command_line[0] = '\0';

    // Sets action (reaper) for signal SIGCHILD.
    signal(SIGCHLD, reaper);

    // Sets action (ctrlc) for signal SIGINT.
    signal(SIGINT, ctrlc);

    // Sets action (ctrlz) for signal SIGTSTP.
    signal(SIGTSTP, ctrlz);

    //Inicialize the fields for foreground process.
    jobs_list[FOREGROUND].pid = foreground.pid;
    jobs_list[FOREGROUND].status = foreground.status;
    strcpy(jobs_list[FOREGROUND].command_line, foreground.command_line);

    // Allocates memory for the input command line.
    char *line = (char *)malloc(sizeof(char) * COMMAND_LINE_SIZE);

    // If there is enough memory for line then execute the loop.
    if (line)
    {
        // Read line and execute it.
        while (read_line(line))
        {
            execute_line(line);
        }
    }
    // Liberates memory and returns exit success.
    free(line);
    return EXIT_SUCCESS;
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
char *read_line(char *line)
{
    // Allocates memory for the prompt and check if it has been able to do it.
    char *prompt = malloc(sizeof(char) * COMMAND_LINE_SIZE);
    if (prompt)
    {
        // Gets the current work directory.
        getcwd(prompt, COMMAND_LINE_SIZE);
        // Prints the prompt and the separator.
        printf("%s %s ", prompt, PROMPT);

        // Reads input introduced in stdin by the user.
        char *ptr = fgets(line, COMMAND_LINE_SIZE, stdin);

        // If the ptr is null, then process if it is an Ctrl+Letter.
        if (!ptr)
        {
            printf("\r");

            // If stdin is end of file then exits from the file.
            if (feof(stdin))
            {
                exit(0);
            }
            else
            {
                // To not allow that ctrl+C exits from the shell.
                ptr = line;

                // This is to avoid the error "command not found".
                ptr[0] = '\0';
            }
        }
        // Frees the memory for prompt and cleans stdin.
        free(prompt);
        fflush(stdin);

        // Checks and cleans the character "\n" at the end of the string line.
        char *n = strchr(line, '\n');
        *n = '\0';

        // Returns the command line.
        return ptr;
    }
}

/*
* Function: execute_line:
* -----------------------
* runs the different functions that will prepare and execute the command line
* introduced by the user.
*
*  line: pointer where will be stored the input introduced by stdin.
*
*  returns: -1 if it has failed or 0 if it was executed correctly.
*/
int execute_line(char *line)
{
    // Allocates memory for the pointers to arguments.
    char **args = malloc(sizeof(char *) * ARGS_SIZE);

    // Checks if it has been done correctly.
    if (args)
    {
        strcpy(jobs_list[FOREGROUND].command_line, line);

        // Obteins the arguments and if there is no arguments then skip.
        if (parse_args(args, line))
        {
            // Checks if it is an internal command if not then execute this.
            if (check_internal(args))
            {
                // Checks if it a background command.
                int bkg = is_background(args);

                // Creates a new thread and obteins his pid.
                pid_t pid = fork();

                // If it is the father then execute this.
                if (pid > 0)
                {
                    // (Temporal)
                    printf("[execute_line()→ PID padre: %d (%s)]", minishell.pid, minishell.command_line);
                    printf("\n[execute_line()→ PID hijo: %d (%s)]\n", pid, jobs_list[FOREGROUND].command_line);

                    // If is a background process then add it to the jobs_list.
                    if (bkg)
                    {
                        jobs_list_add(pid, EXECUTED, jobs_list[FOREGROUND].command_line);
                    }
                    else
                    {
                        // Sets values for the foreground process.
                        jobs_list[FOREGROUND].pid = pid;
                        jobs_list[FOREGROUND].status = EXECUTED;

                        // Waits until all son are finished.
                        while (jobs_list[FOREGROUND].pid)
                        {
                            pause();
                        }
                    }
                }
                // If it is the son then execute this.
                else if (pid == 0)
                {

                    // If it is a background process, ignore SIGTSTP.
                    if (bkg)
                    {
                        signal(SIGTSTP, SIG_IGN);
                    }
                    else
                    {
                        signal(SIGTSTP, SIG_DFL);
                    }
                    // Sets standard action for SIGCHILD.
                    signal(SIGCHLD, SIG_DFL);

                    // Sets ignore for SIGINT.
                    signal(SIGINT, SIG_IGN);

                    // Executes the command introduced using args.
                    if (execvp(args[0], args))
                    {
                        // if there is an error then shows it and exits.
                        perror(args[0]);
                        exit(EXIT_FAILURE);
                    }
                    // Once it has been executed the command then exit.
                    exit(EXIT_SUCCESS);
                }
                else
                {
                    // If an error happens creating the son, error and exit.
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
            }
        }
        // Sets values for the foreground process.
        jobs_list[FOREGROUND].pid = foreground.pid;
        jobs_list[FOREGROUND].status = foreground.status;
        strcpy(jobs_list[FOREGROUND].command_line, foreground.command_line);

        // liberates the memory for the arguments.
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
*  args: pointer to the pointers for all tokens obteined from the line.
*  line: pointer where will be stored the input introduced by stdin.
*
*  returns: the number of tokens obteined from line.
*/
int parse_args(char **args, char *line)
{
    // Count for the tokens and pointer to for each token.
    int ntoken = 0;
    char *token;

    // Checks if line is empty or not.
    if (line)
    {
        // Changes all the tabs with blanks.
        while (strchr(line, '\t'))
        {
            token = strchr(line, '\t');
            *(token) = ' ';
        }
        // Gets the first token and saves it in args.
        token = strtok(line, " ");
        args[ntoken] = token;

        // Loop until obteining a token that is NULL or a comment.
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
*  returns: if it is an internal command or not.
*/
int check_internal(char **args)
{
    // Internal commands.
    const char cd[] = "cd";
    const char export[] = "export";
    const char source[] = "source";
    const char jobs[] = "jobs";
    const char fg[] = "fg";
    const char bg[] = "bg";
    const char ex[] = "exit";

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
    else if (!strcmp(args[0], ex))
    {
        exit(0);
    }
    // returns if it was an internal command.
    return EXIT_FAILURE;
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
int internal_cd(char **args)
{
    // Allocates memory to show the working directory (temporal).
    char *pwd = (char *)malloc(sizeof(char) * COMMAND_LINE_SIZE);

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
        // If there was blanks indicated by any character and treats them.
        aux_internal_cd(path, '\"');
        aux_internal_cd(path, '\'');
        aux_internal_cd(path, '\\');

        // Changes the working directory and checks if it was successful.
        if (chdir(path))
        {
            // Prints the error in stderr.
            perror("chdir");
        }
        else
        {
            // To show how it changes. (temporal).
            getcwd(pwd, COMMAND_LINE_SIZE);
            printf("[internal_cd()→ %s]\n", pwd);
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
        else
        {
            // To show how it changes. (temporal).
            getcwd(pwd, COMMAND_LINE_SIZE);
            printf("[internal_cd()→ %s]\n", pwd);
        }
    }
    // Liberates memory for the pwd (temporal).
    free(pwd);
    return EXIT_SUCCESS;
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
int aux_internal_cd(char *path, char c)
{
    // Checks if there is any character c in the path.
    if (strchr(path, c))
    {
        // Allocates for an auxiliary variable for the path.
        char *auxpath = (char *)malloc(sizeof(char) * COMMAND_LINE_SIZE);

        // Gets the first part from the path with out the character c.
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
*  returns: 0 is it was executed correctly, -1 if an error has been produced.
*/
int internal_export(char **args)
{
    // Checks if it have the arguments correctly.
    if (args[1] && !args[2])
    {
        // Divides the arg 1 using the = as separator.
        strtok(args[1], "=");
        char *token = strtok(NULL, "=");

        // Checks if the estructure NAME=value was introduced correctly.
        if (args[1] && token)
        {
            // Prints values introduced and the current one (temporal).
            printf("[internal_export()→ nombre: %s]\n", args[1]);
            printf("[internal_export()→ valor: %s]\n", token);
            printf("[internal_export()→ antiguo valor: %s]\n", getenv(args[1]));

            // Changes the values of the env variable.
            setenv(args[1], token, 1);

            // Prints the new value of the env variable (temporal).
            printf("[internal_export()→ nuevo valor: %s]\n", getenv(args[1]));
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
*  returns: EXIT_SUCCES if executed or EXIT_FAILURE if there was a problem.
*/
int internal_source(char **args)
{
    // Allocates memory for the line.
    char *line = (char *)malloc(sizeof(char) * COMMAND_LINE_SIZE);
    if (line)
    {
        // Open a file in reading mode.
        FILE *fp = fopen(args[1], "r");
        if (fp)
        {
            // Obtain the lines one by one until reaches the end of file.
            while (fgets(line, COMMAND_LINE_SIZE, fp))
            {
                execute_line(line);

                // After each execution cleans the buffer.
                fflush(fp);
            }
            // Closes the file and frees the memory ocupied by line.
            fclose(fp);
            free(line);
            return EXIT_SUCCESS;
        }
        else
        {
            // If there was a problem we notify it.
            fprintf(stderr, "El archivo no existe o no se puede abrir.\n");

            // Frees the allocated memory if an error occured aswell.
            free(line);
        }
    }
    return EXIT_FAILURE;
}

/*
* Function: internal_jobs:
* ------------------------
*  
*/
int internal_jobs(char **args)
{
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
* Function: jobs_list_add:
* ------------------------
* Adds a new job to the last position of the jobs_list and updates active_jobs. 
* 
*  pid: the pid of the process to add.
*  status: the status of the process to add.
*  command_line: the command_line of the process to add.
* 
*  returns: success if done correctly else faileture.
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

    // Search for the job with the same pid as introduced.
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
* Removes a job from the list and in his positon add the last job active.
*
*  position: position of the job to remove.
*
*  returns: exit success if done correctly else exit failure.
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
        fprintf(stderr, "La posisicion introducida es erronea.\n");
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
*  returns: exit success if it is a foreground process, else exit failure.
*/
int is_background(char **args)
{
    int ind = 0;

    // Search for the last argument.
    while (args[ind + 1])
    {
        ind++;
    }
    // if the last argument conteins '&' then returns exit failture.
    if (!strcmp(args[ind], "&"))
    {
        args[ind] = NULL;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/*
* Function: reaper:
* -----------------
* Executed when a son is terminated. Updates jobs_list. 
*
*  signum: number of the signal.
*
*  returns: void.
*/
void reaper(int signum)
{
    // Variables for the ending process.
    int status;
    pid_t pid;

    // Checks if a process has ended.
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        // If is a foreground process.
        if (pid == jobs_list[FOREGROUND].pid)
        {
            printf("[reaper()→ Proceso hijo %d en foreground (%s) finalizado"
                   " con exit code %d]\n",
                   pid, jobs_list[FOREGROUND].command_line, WEXITSTATUS(status));

            // Sets the job_list[foreground] as it was before.
            jobs_list[FOREGROUND].pid = foreground.pid;
            jobs_list[FOREGROUND].status = foreground.status;
            strcpy(jobs_list[FOREGROUND].command_line, foreground.command_line);
        }
        else
        {
            int pos = jobs_list_find(pid);

            // If was finished with exit.
            if (WIFEXITED(status))
            {
                printf("\n[reaper()→ Proceso hijo %d en background (%s) finalizado"
                       " con exit code %d]\n",
                       pid, jobs_list[pos].command_line, WEXITSTATUS(status));
            }
            // If was finished with a signal.
            else if (WIFSIGNALED(status))
            {
                printf("[reaper()→ Proceso hijo %d en background (%s) finalizado"
                       " por la señal %d]\n",
                       pid, jobs_list[pos].command_line, WTERMSIG(status));
            }

            printf("Terminado PID %d (%s) en jobs_list[%d] con status %d\n",
                   pid, jobs_list[pos].command_line, pos, status);

            jobs_list_remove(pos);
        }
    }
    // Sets again the signal SIGCHLD to the reaper function.
    signal(SIGCHLD, reaper);
}

/*
* Function: ctrlc:
* ----------------
* Executed when a Ctrl+C is presed. 
*
*  signum: number of the signal.
*
*  returns: void.
*/
void ctrlc(int signum)
{
    printf("\n[ctrlc() → Soy el proceso con PID %d (%s), el proceso en "
           "foreground es %d(%s)]\n",
           getpid(), minishell.command_line, jobs_list[FOREGROUND].pid,
           jobs_list[FOREGROUND].command_line);

    // Checks if it the foreground is not the minishell.
    if (jobs_list[FOREGROUND].pid > minishell.pid)
    {
        // Checks if it is the minishell.
        if (strcmp(jobs_list[FOREGROUND].command_line, minishell.command_line))
        {
            // If it is not the minishell then send SIGTERM to the process.
            kill(jobs_list[FOREGROUND].pid, SIGTERM);

            printf("[ctrlc() → Señal 15 enviada a %d (%s) por %d (%s)]\n",
                   jobs_list[FOREGROUND].pid, jobs_list[FOREGROUND].command_line,
                   getpid(), minishell.command_line);
        }
        else
        {
            // Prints error.
            printf("[ctrlc() → Señal 15 no enviada debido a que el "
                   "proceso en foreground es el shell]\n");
        }
    }
    else
    {
        // Prints error.
        printf("[ctrlc() → Señal no enviada por %d (%s) debido a "
               "que no hay proceso en foreground]\n",
               getpid(), minishell.command_line);
    }

    // Sets again SIGINT to the function ctrlc.
    signal(SIGINT, ctrlc);
}

/*
* Function ctrlz:
* ---------------
* Executed when is presed Ctrl+Z. This function stops the foreground process
* and allows the user to input new commands.
*
*  signum: number of the signal.
*
*  returns: void
*/
void ctrlz(int signum)
{
    printf("\n[ctrlz()→ Soy el proceso con PID %d, el proceso en foreground es"
           " %d (%s)]\n",
           getpid(), jobs_list[FOREGROUND].pid,
           jobs_list[FOREGROUND].command_line);

    // Check if there is a foreground process.
    if (jobs_list[FOREGROUND].pid != foreground.pid)
    {

        // Checks if is a son that the foreground process is not a minishell.
        if (strcmp(jobs_list[FOREGROUND].command_line, minishell.command_line))
        {

            // Sends the signal to stop to the foreground process.
            kill(jobs_list[FOREGROUND].pid, SIGTSTP);

            // Updates the process stopped and adds it to the jobs queue.
            jobs_list[FOREGROUND].status = STOPPED;
            jobs_list_add(jobs_list[FOREGROUND].pid, jobs_list[FOREGROUND].status,
                          jobs_list[FOREGROUND].command_line);

            // Updates the foreground with the default foreground properties.
            jobs_list[FOREGROUND].pid = foreground.pid;
            jobs_list[FOREGROUND].status = foreground.status;
            strcpy(jobs_list[FOREGROUND].command_line, foreground.command_line);

            printf("[ctrlz()→ Señal %d (SIGTSTP) enviada a %d (%s) por %d (%s)]\n", signum,
                   jobs_list[FOREGROUND].pid,
                   jobs_list[FOREGROUND].command_line,
                   getpid(), minishell.command_line);
        }
        else
        {
            // Prints error.
            printf("[ctrlz()→ Señal %d (SIGTSTP) no enviada debido a que el "
                   "proceso en foreground es el shell.]\n",
                   signum);
        }
    }
    else
    {
        // Prints error.
        printf("[ctrlz()→ Señal %d (SIGTSTP) no enviada debido a que no hay"
               " proceso en foreground.\n",
               signum);
    }
    signal(SIGTSTP, ctrlz);
}

int internal_fg(char **args)
{
    if (args[1])
    {
        int job = (int)*(args[1]) - 48;
        if (job > 0 && job < active_jobs)
        {
            if (jobs_list[job].status == STOPPED)
            {
                kill(jobs_list[job].pid, SIGCONT);
            }
            jobs_list[FOREGROUND].pid = jobs_list[job].pid;
            jobs_list[FOREGROUND].status = jobs_list[job].status;
            strcpy(jobs_list[FOREGROUND].command_line, jobs_list[job].command_line);
            jobs_list_remove(job);
            char *pos = strchr(jobs_list[FOREGROUND].command_line, '&');
            if (pos)
            {
                *(pos - 1) = '\0';
            }
            printf("%s\n", jobs_list[FOREGROUND].command_line);

            while (jobs_list[FOREGROUND].pid)
            {
                pause();
            }
            return EXIT_SUCCESS;
        }
        fprintf(stderr, "El trabajo %d no existe.", job);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "La sintaxis es erronea, fg n_job");
    return EXIT_FAILURE;
}

int internal_bg(char **args)
{
    if (args[1])
    {
        int job = (int)*(args[1]) - 48;
        if (job > 0 && job < active_jobs)
        {
            if (jobs_list[job].status != EXECUTED)
            {
                strcat(jobs_list[job].command_line, " &\0");
                jobs_list[job].status = EXECUTED;
                kill(jobs_list[job].pid, SIGCONT);
                printf("[internal_bg→ señal %d enviada a %d (%s)]\n", SIGCONT, jobs_list[job].pid, jobs_list[job].command_line);
                return EXIT_SUCCESS;
            }
            fprintf(stderr, "El trabajo %d ya se esta en 2º plano.", job);
            return EXIT_FAILURE;
        }
        fprintf(stderr, "El trabajo %d no existe.", job);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "La sintaxis es erronea, bg n_job");
    return EXIT_FAILURE;
}