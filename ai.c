#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

typedef struct {
    char method[256];
    char params[256];
} rpc_data;

typedef struct {
    int port;
    int sockfd;
    struct sockaddr_in server_addr;
} rpc_server;

typedef struct {
    char addr[256];
    int port;
    int sockfd;
    struct sockaddr_in server_addr;
} rpc_client;

typedef struct {
    rpc_client* client;
    char name[256];
} rpc_handle;

rpc_data* rpc_handler(rpc_data* request) {
    // Example handler implementation
    rpc_data* response = (rpc_data*)malloc(sizeof(rpc_data));
    
    if (strcmp(request->method, "add") == 0) {
        int sum = atoi(request->params);
        sum += 10;
        sprintf(response->params, "%d", sum);
    } else if (strcmp(request->method, "multiply") == 0) {
        int product = atoi(request->params);
        product *= 5;
        sprintf(response->params, "%d", product);
    } else {
        sprintf(response->params, "Unknown method");
    }
    
    return response;
}

rpc_server* rpc_init_server(int port) {
    rpc_server* srv = (rpc_server*)malloc(sizeof(rpc_server));
    srv->port = port;
    srv->sockfd = -1;
    return srv;
}

int rpc_register(rpc_server* srv, const char* name, rpc_data* (*handler)(rpc_data*)) {
    if (srv == NULL) {
        return -1;
    }
    
    // Open a socket
    srv->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv->sockfd < 0) {
        perror("Error opening socket");
        return -1;
    }
    
    // Bind the socket to the given port
    memset(&(srv->server_addr), 0, sizeof(srv->server_addr));
    srv->server_addr.sin_family = AF_INET;
    srv->server_addr.sin_addr.s_addr = INADDR_ANY;
    srv->server_addr.sin_port = htons(srv->port);
    
    if (bind(srv->sockfd, (struct sockaddr*)&(srv->server_addr), sizeof(srv->server_addr)) < 0) {
        perror("Error binding");
        return -1;
    }
    
    // Listen for incoming connections
    listen(srv->sockfd, 5);
    
    printf("Server listening on port %d\n", srv->port);
    
    return 0;
}

void rpc_serve_all(rpc_server* srv) {
    if (srv == NULL) {
        return;
    }
    
    while (1) {
        // Accept client connection
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sockfd = accept(srv->sockfd, (struct sockaddr*)&client_addr, &client_len);
        if (client_sockfd < 0) {
            perror("Error accepting connection");
            return;
        }
        
        // Read request from client
        rpc_data request;
        memset(&request, 0, sizeof(request));
        if (read(client_sockfd, &request, sizeof(request)) < 0) {
            perror("Error reading from socket");
            return;
        }
        
        // Handle the request
        rpc_data* response = rpc_handler(&request);
        
        // Send response back to client
        if (write(client_sockfd, response, sizeof(rpc_data)) < 0) {
            perror("Error writing to socket");
            return;
        }
        
        // Clean up
        free(response);
        close(client_sockfd);
    }
}

rpc_client* rpc_init_client(const char* addr, int port) {
    rpc_client* cl = (rpc_client*)malloc(sizeof(rpc_client));
    strcpy(cl->addr, addr);
    cl->port = port;
    cl->sockfd = -1;
    return cl;
}

void rpc_close_client(rpc_client* cl) {
    if (cl == NULL) {
        return;
    }
    
    if (cl->sockfd != -1) {
        close(cl->sockfd);
    }
    
    free(cl);
}

rpc_handle* rpc_find(rpc_client* cl, const char* name) {
    if (cl == NULL) {
        return NULL;
    }
    
    rpc_handle* handle = (rpc_handle*)malloc(sizeof(rpc_handle));
    handle->client = cl;
    strcpy(handle->name, name);
    
    return handle;
}

rpc_data* rpc_call(rpc_client* cl, rpc_handle* h, const rpc_data* data) {
    if (cl == NULL || h == NULL) {
        return NULL;
    }
    
    // Create a socket
    h->client->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (h->client->sockfd < 0) {
        perror("Error opening socket");
        return NULL;
    }
    
    // Set server address
    memset(&(h->client->server_addr), 0, sizeof(h->client->server_addr));
    h->client->server_addr.sin_family = AF_INET;
    h->client->server_addr.sin_addr.s_addr = inet_addr(h->client->addr);
    h->client->server_addr.sin_port = htons(h->client->port);
    
    // Connect to the server
    if (connect(h->client->sockfd, (struct sockaddr*)&(h->client->server_addr), sizeof(h->client->server_addr)) < 0) {
        perror("Error connecting");
        return NULL;
    }
    
    // Send request to the server
    if (write(h->client->sockfd, data, sizeof(rpc_data)) < 0) {
        perror("Error writing to socket");
        return NULL;
    }
    
    // Read response from the server
    rpc_data* response = (rpc_data*)malloc(sizeof(rpc_data));
    memset(response, 0, sizeof(rpc_data));
    if (read(h->client->sockfd, response, sizeof(rpc_data)) < 0) {
        perror("Error reading from socket");
        free(response);
        return NULL;
    }
    
    // Clean up
    close(h->client->sockfd);
    h->client->sockfd = -1;
    
    return response;
}

int main() {
    // Initialize server
    rpc_server* srv = rpc_init_server(1234);
    
    // Register handler functions
    rpc_register(srv, "add", rpc_handler);
    rpc_register(srv, "multiply", rpc_handler);
    
    // Start serving requests
    rpc_serve_all(srv);
    
    // Clean up server
    free(srv);
    
    // Initialize client
    rpc_client* cl = rpc_init_client("127.0.0.1", 1234);
    
    // Find RPC handle
    rpc_handle* handle = rpc_find(cl, "add");
    
    // Prepare request data
    rpc_data request;
    strcpy(request.method, "add");
    strcpy(request.params, "5");
    
    // Make RPC call
    rpc_data* response = rpc_call(cl, handle, &request);
    
    if (response != NULL) {
        printf("Response: %s\n", response->params);
        free(response);
    }
    
    // Clean up client
    rpc_close_client(cl);
    
    return 0;
}
