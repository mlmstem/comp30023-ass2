#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void read_arguments_from_line(char* line) {
    char command[10];
    char arg1[100];
    char arg2[100];
    
    // Tokenize the line using a delimiter (e.g., space)
    char* token = strtok(line, " \t\n");
    if (token == NULL) {
        printf("No input\n");
        return;
    }
    
    // Extract the command
    strcpy(command, token);
    
    // Extract the arguments if available
    token = strtok(NULL, " \t\n");
    if (token != NULL) {
        strcpy(arg1, token);
        
        token = strtok(NULL, " \t\n");
        if (token != NULL) {
            strcpy(arg2, token);
        }
    }
    
    // Process the command and arguments
    if (strcmp(command, "init") == 0) {
        // Call rpc_init_server with arg1 as port
        int port = atoi(arg1);
        rpc_init_server(port);
    } else if (strcmp(command, "register") == 0) {
        // Call rpc_register with arg1 as name and arg2 as handler
        rpc_register(arg1, arg2);
    } else if (strcmp(command, "serve") == 0) {
        // Call rpc_serve_all
        rpc_serve_all();
    } else {
        printf("Invalid command\n");
    }
}

int main() {
    char buffer[256];
    
    printf("Enter a command: ");
    fgets(buffer, sizeof(buffer), stdin);
    
    // Remove the trailing newline character from fgets
    buffer[strcspn(buffer, "\n")] = '\0';
    
    // Process the line
    read_arguments_from_line(buffer);
    
    return 0;
}
