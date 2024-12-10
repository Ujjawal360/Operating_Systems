#define _DEFAULT_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 128

extern char **environ;  // Provides access to environment variables

pid_t foreground_pid = -1;  // Stores the PID of the foreground process

// Function declarations
bool is_builtin(char *cmd);
void run_builtin_commands(char *args[]);
void run_command(char *args[], bool is_background);

// Retrieves the shell prompt with the current working directory
char *get_prompt() {
    static char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        strcat(cwd, "> ");
        return cwd;
    } else {
        perror("getcwd error");
        exit(1);
    }
}

// Signal handler for SIGINT (Ctrl-C) to prevent shell termination
void handle_sigint(int sig) {
    printf("\nCtrl-C detected. Returning to shell prompt...\n");
    printf("%s", get_prompt());
    fflush(stdout);
}

// Signal handler for SIGALRM to terminate long-running foreground processes
void handle_sigalrm(int sig) {
    if (foreground_pid > 0) {
        printf("\nForeground process timed out. Terminating PID: %d\n", foreground_pid);
        kill(foreground_pid, SIGKILL);
        foreground_pid = -1;
    }
}

// Tokenizes the command line into individual arguments
void tokenize_input(char *input, char *args[]) {
    int index = 0;
    char *token = strtok(input, " \t\r\n");
    while (token != NULL && index < MAX_ARGS - 1) {
        args[index++] = token;
        token = strtok(NULL, " \t\r\n");
    }
    args[index] = NULL;
}

// Checks if a command is a built-in command
bool is_builtin(char *cmd) {
    return strcmp(cmd, "cd") == 0 || strcmp(cmd, "pwd") == 0 ||
           strcmp(cmd, "echo") == 0 || strcmp(cmd, "env") == 0 ||
           strcmp(cmd, "setenv") == 0 || strcmp(cmd, "exit") == 0;
}

// Executes built-in commands
void run_builtin_commands(char *args[]) {
    if (strcmp(args[0], "cd") == 0) {
        if (chdir(args[1]) != 0) perror("cd error");
    } else if (strcmp(args[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) printf("%s\n", cwd);
        else perror("pwd error");
    } else if (strcmp(args[0], "echo") == 0) {
        for (int i = 1; args[i] != NULL; i++) {
            if (args[i][0] == '$') {
                char *env_val = getenv(args[i] + 1);
                printf("%s ", env_val ? env_val : "");
            } else {
                printf("%s ", args[i]);
            }
        }
        printf("\n");
    } else if (strcmp(args[0], "env") == 0) {
        for (int i = 0; environ[i] != NULL; i++) printf("%s\n", environ[i]);
    } else if (strcmp(args[0], "setenv") == 0) {
        if (args[1] && args[2]) {
            setenv(args[1], args[2], 1);
        } else {
            printf("Usage: setenv <VAR> <VALUE>\n");
        }
    } else if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }
}

// Handles output redirection with `>`
void handle_output_redirection(char *args[]) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open error");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
            break;
        }
    }
}

// Handles input redirection with `<`
void handle_input_redirection(char *args[]) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            int fd = open(args[i + 1], O_RDONLY);
            if (fd < 0) {
                perror("open error");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
            break;
        }
    }
}

// Handles piping with `|`
bool handle_pipe(char *args[]) {
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "|") == 0) {
            args[i] = NULL;

            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("pipe error");
                return false;
            }

            pid_t pid1 = fork();
            if (pid1 == 0) {
                dup2(pipefd[1], STDOUT_FILENO);  // Redirect stdout to pipe write end
                close(pipefd[0]);  // Close unused read end
                close(pipefd[1]);
                execvp(args[0], args);
                perror("execvp error");
                exit(1);
            } else if (pid1 < 0) {
                perror("fork error");
                return false;
            }

            pid_t pid2 = fork();
            if (pid2 == 0) {
                dup2(pipefd[0], STDIN_FILENO);  // Redirect stdin to pipe read end
                close(pipefd[1]);  // Close unused write end
                close(pipefd[0]);
                execvp(args[i + 1], &args[i + 1]);
                perror("execvp error");
                exit(1);
            } else if (pid2 < 0) {
                perror("fork error");
                return false;
            }

            close(pipefd[0]);
            close(pipefd[1]);
            waitpid(pid1, NULL, 0);  // Wait for the first process
            waitpid(pid2, NULL, 0);  // Wait for the second process
            return true;
        }
        i++;
    }
    return false;
}

// Runs external commands
void run_command(char *args[], bool is_background) {
    if (handle_pipe(args)) return;

    pid_t pid = fork();
    if (pid == 0) {
        signal(SIGINT, SIG_DFL);  // Restore default SIGINT behavior in child
        handle_output_redirection(args);  // Handle output redirection
        handle_input_redirection(args);  // Handle input redirection
        execvp(args[0], args);  // Execute the command
        perror("execvp error");
        exit(1);
    } else if (pid < 0) {
        perror("fork error");
    } else {
        if (!is_background) {
            foreground_pid = pid;
            alarm(10);  // Set a 10-second timer for long-running processes
            waitpid(pid, NULL, 0);  // Wait for the child process
            alarm(0);  // Cancel the timer
            foreground_pid = -1;  // Reset foreground process PID
        } else {
            printf("Background process started with PID: %d\n", pid);
        }
    }
}

// Main loop to interact with the shell
int main() {
    char input[MAX_CMD_LEN];
    char *args[MAX_ARGS];

    signal(SIGINT, handle_sigint);
    signal(SIGALRM, handle_sigalrm);

    while (true) {
        printf("%s", get_prompt());
        fflush(stdout);

        if (fgets(input, MAX_CMD_LEN, stdin) == NULL) break;

        input[strlen(input) - 1] = '\0';  // Remove trailing newline
        tokenize_input(input, args);

        bool is_background = (strcmp(args[0], "&") == 0);
        if (is_builtin(args[0])) {
            run_builtin_commands(args);
        } else {
            run_command(args, is_background);
        }
    }
    return 0;
}
