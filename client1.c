#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef int (*FunctionPtr)(int, int);

int main() {
    int sockfd, port;
    char serverIP[256];
    struct sockaddr_in serverAddr;
    int numFunctions;
    FunctionPtr *functions;

    // Read server IP and port from user
    printf("Enter server IP: ");
    scanf("%s", serverIP);
    printf("Enter server port: ");
    scanf("%d", &port);

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    serverAddr.sin_port = htons(port);
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }

    // Receive function pointers from the server
    read(sockfd, &numFunctions, sizeof(numFunctions));
    functions = (FunctionPtr *)malloc(numFunctions * sizeof(FunctionPtr));
    for (int i = 0; i < numFunctions; i++) {
        read(sockfd, &functions[i], sizeof(FunctionPtr));
    }

    // Call the received functions
    int a = 10, b = 5;
    for (int i = 0; i < numFunctions; i++) {
        int result = functions[i](a, b);
        printf("Function %d result: %d\n", i + 1, result);
    }

    // Clean up
    free(functions);
    close(sockfd);

    return 0;
}
