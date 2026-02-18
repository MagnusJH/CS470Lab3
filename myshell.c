#define MAX_LINE 1024  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

void parse_input(char *line, char *args[], int max_args);
 
int main() {
    char line[MAX_LINE];
    while (1) { 
        printf("myshell> ");
        fflush(stdout);      
        if (!fgets(line, sizeof(line), stdin))
            break;   
 
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0)
            continue;

        // Parse the input into tokens
        int N = 10;
        char *tokens[N];

        // tokenize the string
        parse_input(line, tokens, N);

        // exit program
        if (strcmp(tokens[0], "exit") == 0) {
            break;
        }

        // change directory
        if (strcmp(tokens[0], "cd") == 0) {
            if (chdir(tokens[1]) == -1) {
                // go to home directory if just cd was given
                if (!tokens[1]) {
                    chdir(getenv("HOME"));
                    printf("Directory changed successfully.\n");
                } else {
                    // print error if path doesn't exist
                    printf("Failed to change directory to \"%s\"\n", tokens[1]);
                }
            } else {
                printf("Directory changed successfully.\n");
            }
            continue;
        }

        // execute commands in the child processes
        int status;
        int fn;
        pid_t pid;
        pid = fork();

        if (pid == -1) {
            perror("Fork Failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {

            // input redirection
            int i = 0;
            while (tokens[i] != NULL && i < 9) {
                if (strcmp(tokens[i], "<") == 0) {
                    // open file
                    fn = open(tokens[i+1], O_RDONLY);
                    if (fn == -1) {
                        tokens[0] = NULL;
                        perror("File Error");
                        break;
                    }
                    tokens[i] = NULL;

                    // redirect input with the file
                    dup2(fn, STDIN_FILENO);
                    close(fn);

                } else if (strcmp(tokens[i], ">") == 0) {
                    // overwrite or create file
                    fn = open(tokens[i+1], O_TRUNC | O_CREAT | O_WRONLY, 0644);
                    if (fn == -1) {
                        tokens[0] = NULL;
                        perror("File Error");
                        break;
                    }
                    tokens[i] = NULL;

                    // redirect standard output
                    dup2(fn, STDOUT_FILENO);
                    close(fn);

                } else if (strcmp(tokens[i], ">>") == 0) {
                    // open file in append mode
                    fn = open(tokens[i+1], O_APPEND | O_CREAT | O_WRONLY, 0644);
                    if (fn == -1) {
                        tokens[0] = NULL;
                        perror("File Error");
                        break;
                    }
                    tokens[i] = NULL;

                    // redirect output
                    dup2(fn, STDOUT_FILENO);
                    close(fn);
                }
                i++;
            }
            
            // execute command
            execvp(tokens[0], tokens);

            // if it returned there was an error
            perror("Command Failed");
            exit(1);
        } else {

            // wait for child to finish executing
            waitpid(pid, &status, 0);
        }
    }

        // ============================================
        // TODO: Students implement the following:
        //
        // 1. Parse the input line into tokens
        //    - Split by whitespace
        //    - Handle special characters (<, >, >>)
        //
        // 2. Check for built-in commands
        //    - "cd [dir]" - change directory
        //    - "exit" - exit the shell
        //
        // 3. Execute external commands
        //    - fork() to create child process
        //    - execvp() to run the command
        //    - waitpid() to wait for completion
        //
        // 4. Handle redirection
        //    - < for input redirection
        //    - > for output redirection
        //    - >> for append redirection
        // ============================================

    printf("\nGoodbye!\n");
    return 0;
}

// command parser
void parse_input(char *line, char *args[], int max_args) {
    int i = 0;
    char *p = line;

    while (*p && i < max_args - 1) {

        // Skip leading spaces
        while (*p == ' ')
            p++;

        if (*p == '\0')
            break;

        // If token starts with quote
        if (*p == '"') {
            p++;  // skip opening quote
            args[i++] = p;

            // Find closing quote
            while (*p && *p != '"')
                p++;

            if (*p == '"') {
                *p = '\0';  // terminate token
                p++;        // move past closing quote
            }
        }
        else {
            args[i++] = p;

            while (*p && *p != ' ')
                p++;

            if (*p) {
                *p = '\0';
                p++;
            }
        }
    }

    args[i] = NULL;
}
