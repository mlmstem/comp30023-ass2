#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

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

typedef struct {
    int port;
    FunctionMap *map;
    int number_handlers;
    int sockfd;
} rpc_server;

// Function handlers
rpc_data *handleFunction1(rpc_data *data) {
    printf("Executing Function 1\n");
    // Perform operations using data
    return NULL;
}

rpc_data *handleFunction2(rpc_data *data) {
    printf("Executing Function 2\n");
    // Perform operations using data
    return NULL;
}

rpc_data *handleFunction3(rpc_data *data) {
    printf("Executing Function 3\n");
    // Perform operations using data
    return NULL;
}

void *handle_client_request(void *arg) {
    int client_sockfd = *(int *)arg;

    // Read request from client
    char request[30];
    if (read(client_sockfd, request, sizeof(request)) < 0) {
        perror("Error reading from socket");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Check if it's an rpc_find request
    if (strcmp(request, "rpc_find") == 0) {
        // Send the number of handlers to the client
        if (write(client_sockfd, &(srv->number_handlers), sizeof(int)) < 0) {
            perror("Error writing to socket");
            close(client_sockfd);
            pthread_exit(NULL);
        }

        // Send the function map to the client
        if (write(client_sockfd, srv->map, sizeof(FunctionMap) * srv->number_handlers) < 0) {
            perror("Error writing to socket");
            close(client_sockfd);
            pthread_exit(NULL);
        }
    }

    close(client_sockfd);
    pthread_exit(NULL);
}

void rpc_serve_all(rpc_server *srv) {
    if (srv == NULL) {
        return;
    }

    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr);

    // Create socket
    srv->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv->sockfd < 0) {
        perror("Error creating socket");
        return;
    }

    // Bind socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(srv->port);
    if (bind(srv->sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        return;
    }

    // Listen for connections
    if (listen(srv->sockfd, 5) < 0) {
        perror("Error listening for connections");
        return;
    }

    while (1) {
        int client_sockfd = accept(srv->sockfd, (struct sockaddr *)&server_addr, &server_len);
        if (client_sockfd < 0) {
            perror("Error accepting connection");
            return;
        }

        // Create a new thread to handle the client request
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client_request, &client_sockfd) != 0) {
            perror("Error creating thread");
            close(client_sockfd);
            continue;
        }
    }
}

int main() {
    FunctionMap functionMap[] = {
        {"function1", handleFunction1},
        {"function2", handleFunction2},
        {"function3", handleFunction3}
    };

    rpc_server server;
    server.port = 8888;
    server.map = functionMap;
    server.number_handlers = sizeof(functionMap) / sizeof(functionMap[0]);

    rpc_serve_all(&server);

    return 0;
}
