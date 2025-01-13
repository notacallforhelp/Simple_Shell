#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

// Function to handle input/output redirection
void handle_redirection(char **args) {
    int i = 0;
    while (args[i] != NULL) {
        // Handle output redirection
        if (strcmp(args[i], ">") == 0) {
            int fd = open(args[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        }
        // Handle input redirection
        else if (strcmp(args[i], "<") == 0) {
            int fd = open(args[i + 1], O_RDONLY);
            if (fd < 0) {
                perror("open");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
        }
        i++;
    }
}

// Function to execute a command
void execute(char **args, int background) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        handle_redirection(args);
        if (execvp(args[0], args) < 0) {
            perror("execvp");
            exit(1);
        }
    } else if (pid > 0) {
        // Parent process
        if (!background) {
            waitpid(pid, NULL, 0);  // Wait for the process if it's not in the background
        } else {
            printf("Process running in background with PID %d\n", pid);
        }
    } else {
        perror("fork");
    }
}

// Function to split the input command into arguments
char **parse_command(char *input) {
    char **args = malloc(64 * sizeof(char *));
    char *token;
    int i = 0;

    token = strtok(input, " \n");
    while (token != NULL) {
        args[i] = token;
        i++;
        token = strtok(NULL, " \n");
    }
    args[i] = NULL;
    return args;
}

// Main shell loop
int main() {
    char input[1024];
    char **args;
    int background;

    while (1) {
        printf("shell> ");
        fflush(stdout);
        
        if (!fgets(input, 1024, stdin)) {
            break;  // EOF (Ctrl+D)
        }

        // Check if the command should run in the background
        background = 0;
        if (strchr(input, '&')) {
            background = 1;
            *strchr(input, '&') = '\0';  // Remove '&' from the command
        }

        args = parse_command(input);

        if (args[0] == NULL) {
            continue;  // Empty command
        }

        if (strcmp(args[0], "exit") == 0) {
            free(args);
            break;  // Exit the shell
        }

        execute(args, background);
        free(args);
    }

    return 0;
}
