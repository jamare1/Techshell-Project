/*
* Author: Jack Revelett
*
* Features:
* - Displays a prompt with the current working directory
* - Executes user commands using fork() and execvp()
* - Supports reading from files using <
* - Supports writing to files using >
* - Handles errors
* - Exits when the user types "exit"
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_INPUT_SIZE 1024  // Maximum size of user input
#define INITIAL_ARG_SIZE 10  // Start with space for 10 arguments, expand if needed

// Defines a struct to store the parsed command data
typedef struct{
    char** args;       // Dynamically allocated array for command arguments
    char* inputFile;   // Input redirection file
    char* outputFile;  // Output redirection file
} ShellCommand;

// Function prototypes
char* CommandPrompt();
ShellCommand ParseCommandLine(char* input);
void ExecuteCommand(ShellCommand command);

int main(){
    char* input;
    ShellCommand command;

    for(;;){
        // Get user input from the command line
        input = CommandPrompt();
        if(input == NULL){
            continue;  // Skips processing if there's no input
        }

        // Parse the command input
        command = ParseCommandLine(input);

        // Execute the parsed command
        ExecuteCommand(command);

        // Free dynamically allocated memory after execution is finished
        free(input);
        if(command.inputFile){
            free(command.inputFile);
        }
        if(command.outputFile){
            free(command.outputFile);
        }
        if(command.args){
            for(int i = 0; command.args[i] != NULL; i++){
                free(command.args[i]);
            }
            free(command.args);
        }
    }
    exit(0);
}


/*
 * Function: CommandPrompt
 * --------------------------
 * Displays the current working directory and prompts the user for input
 *
 * Parameters:
 *   None
 * 
 * Returns:
 *   A string containing the user's input
 */
char* CommandPrompt(){
    char* cwd = getcwd(NULL, 0);  // Get current working directory
    if(cwd){
        printf("%s$ ", cwd);  // Display the prompt with the current directory ($)
        free(cwd);  // Free allocated memory from getcwd()
    }
    else{
        perror("getcwd failed");
    }
    fflush(stdout);

    // Allocate memory for user input
    char* input = (char*)malloc(MAX_INPUT_SIZE);
    if(!input){
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Read user input from stdin
    if(fgets(input, MAX_INPUT_SIZE, stdin) == NULL){
        perror("Error reading input");
        free(input);
        return NULL;
    }

    // Remove trailing newline character from input
    input[strcspn(input, "\n")] = 0;

    return input;
}


/*
 * Function: ParseCommandLine
 * --------------------------
 * Parses the user input into a ShellCommand struct, extracting the command,
 * arguments, and input/output files
 *
 * Parameters:
 *   input - The raw user input string
 *
 * Returns:
 *   A ShellCommand struct containing parsed command data
 */
ShellCommand ParseCommandLine(char* input) {
    ShellCommand command;
    command.inputFile = NULL;
    command.outputFile = NULL;
    command.args = NULL;

    int capacity = INITIAL_ARG_SIZE;
    int count = 0;

    // Allocate memory for argument list
    command.args = (char**)malloc(capacity * sizeof(char*));
    if(!command.args){
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    char* token;
    char* rest = input;

    // Tokenize the input string
    while((token = strtok_r(rest, " ", &rest))){
        // Check for input redirection
        if(strcmp(token, "<") == 0){
            token = strtok_r(NULL, " ", &rest);
            if(token != NULL){
                command.inputFile = strdup(token);  // Store input file
            }
            else{
                fprintf(stderr, "Error: Expected filename after '<'\n");
                command.args[0] = NULL;
                return command;
            }
        }
        // Check for output redirection
        else if (strcmp(token, ">") == 0) {
            token = strtok_r(NULL, " ", &rest);
            if(token != NULL){
                command.outputFile = strdup(token);  // Store output file
            }
            else{
                fprintf(stderr, "Error: Expected filename after '>'\n");
                command.args[0] = NULL;
                return command;
            }
        } 
        else{
            // Resize argument list if needed
            if(count >= capacity - 1){
                capacity *= 2;
                command.args = (char**)realloc(command.args, capacity * sizeof(char*));
                if(!command.args){
                    perror("Memory reallocation failed");
                    exit(EXIT_FAILURE);
                }
            }
            command.args[count++] = strdup(token);
        }
    }

    if(count == 0){  
        command.args[0] = NULL;  // Ensure args[0] is NULL if no command is entered
    }
    else{
        command.args[count] = NULL; // Null-terminate the argument list
    }
    
    return command;
}


/*
 * Function: ExecuteCommand
 * ------------------------
 * Executes the parsed command using fork() and execvp()
 * Handles input/output redirection
 *
 * Parameters:
 *   command - A ShellCommand struct containing the parsed command data
 * 
 * Returns:
 *   None
 */
void ExecuteCommand(ShellCommand command){
    // Check for empty command
    if(command.args[0] == NULL){
        fprintf(stderr, "Error: No command entered\n");
        return;
    }

    // Handles 'exit' command
    if(strcmp(command.args[0], "exit") == 0){
        exit(0);
    }

    // Handle 'cd' command
    if(strcmp(command.args[0], "cd") == 0){
        if(command.args[1] == NULL){
            // If no directory is specified, go to the home directory
            const char *home = getenv("HOME");
            if(home == NULL){
                home = "/";
            }
            if(chdir(home) != 0){
                perror("cd failed");
            }
        }
        else{
            char path[MAX_INPUT_SIZE] = "";

            // Check if the argument starts with a quote
            if(command.args[1][0] == '"'){
                // Concatenate arguments inside the quotes
                strcat(path, command.args[1] + 1); // Skip the opening quote

                for(int i = 2; command.args[i] != NULL; i++){
                    strcat(path, " "); // Add spaces between words
                    strcat(path, command.args[i]);

                    // Check if this argument ends with a closing quote
                    int len = strlen(command.args[i]);
                    if(command.args[i][len - 1] == '"'){
                        path[strlen(path) - 1] = '\0'; // Remove closing quote
                        break;
                    }
                }
            }
            else{
                // Normal single-word directory
                strcpy(path, command.args[1]);
            }

            // Attempt to change directory
            if(chdir(path) != 0){
                perror("cd failed");
            }
        }
        return;  // Return after handling `cd`
    }


    // Fork a new process to execute external commands
    pid_t pid = fork();

    if(pid == -1){
        perror("Fork failed");
        return;
    }
    else if(pid == 0){ // Child process
        if(command.inputFile){
            if(strlen(command.inputFile) == 0){
                fprintf(stderr, "Error: No input filename specified\n");
                exit(EXIT_FAILURE);
            }
            int fd = open(command.inputFile, O_RDONLY);
            if(fd == -1){
                fprintf(stderr, "Error: Cannot open input file '%s': %s\n", command.inputFile, strerror(errno));
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if(command.outputFile){
            if(strlen(command.outputFile) == 0){  // Prevents empty filenames
                fprintf(stderr, "Error: No output filename specified\n");
                exit(EXIT_FAILURE);
            }

            int fd = open(command.outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(fd == -1){
                fprintf(stderr, "Error: Cannot open output file '%s': %s\n", command.outputFile, strerror(errno));
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Execute the command using execvp
        execvp(command.args[0], command.args);
        fprintf(stderr, "Error: Command '%s' not found\n", command.args[0]);
        exit(127);

    }
    else{ // Parent process waits for child to finish
        int status;
        waitpid(pid, &status, 0);
    }
}