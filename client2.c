#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

typedef struct {
    char name[30];
    rpc_handler handler;
} FunctionMap;

typedef struct {
    int data1;
    size_t data2_len;
    void *data2;
} rpc_data;

typedef rpc_data *(*rpc_handler)(rpc_data *);

int main() {
    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr);

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return -1;
    }

    // Connect to server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);
    if (connect(sockfd, (struct sockaddr *)&server_addr, server_len) < 0) {
        perror("Error connecting to server");
        return -1;
    }

    // Send rpc_find request to server
    char request[30] = "rpc_find";
    if (write(sockfd, request, sizeof(request)) < 0) {
        perror("Error writing to socket");
        return -1;
    }

    // Receive the number of handlers from server
    int number_handlers;
    if (read(sockfd, &number_handlers, sizeof(int)) < 0) {
        perror("Error reading from socket");
        return -1;
    }

    // Receive the function map from server
    FunctionMap *functionMap = malloc(sizeof(FunctionMap) * number_handlers);
    if (read(sockfd, functionMap, sizeof(FunctionMap) * number_handlers) < 0) {
        perror("Error reading from socket");
        return -1;
    }

    // Perform continuous communication with the server
    while (1) {
        // Prompt the user for the function name
        printf("Enter the function name (or 'quit' to exit): ");
        char functionName[30];
        scanf("%s", functionName);

        if (strcmp(functionName, "quit") == 0) {
            break;
        }

        // Find the function in the function map
        rpc_handler equationHandler = NULL;
        for (int i = 0; i < number_handlers; i++) {
            if (strcmp(functionName, functionMap[i].name) == 0) {
                equationHandler = functionMap[i].handler;
                break;
            }
        }

        if (equationHandler != NULL) {
            // Call the function with sample data
            rpc_data data = {.data1 = 1, .data2_len = 1, .data2 = &(data.data1)};
            rpc_data *result = equationHandler(&data);
            // Process the result if needed
        } else {
            printf("Function name not found\n");
        }
    }

    // Clean up
    close(sockfd);
    free(functionMap);

    return 0;
}
