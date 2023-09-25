#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef int (*FunctionPtr)(int, int);

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

int divide(int a, int b) {
    return a / b;
}

int main() {
    int sockfd, clientfd;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen;
    int port = 3000;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Bind socket
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(sockfd, 1) < 0) {
        perror("Failed to listen for connections");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);

    while (1) {
        // Accept connection
        clientLen = sizeof(clientAddr);
        clientfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientLen);
        if (clientfd < 0) {
            perror("Failed to accept connection");
            continue;
        }

        printf("Client connected: %s\n", inet_ntoa(clientAddr.sin_addr));

        // Send function pointers to the client
        FunctionPtr functions[] = { add, subtract, multiply, divide };
        int numFunctions = sizeof(functions) / sizeof(FunctionPtr);
        write(clientfd, &numFunctions, sizeof(numFunctions));
        for (int i = 0; i < numFunctions; i++) {
            write(clientfd, &functions[i], sizeof(FunctionPtr));
        }

        // Close connection
        close(clientfd);
    }

    // Close socket
    close(sockfd);

    return 0;
}
