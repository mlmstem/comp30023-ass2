
#define _POSIX_C_SOURCE 200112L
#include "rpc.h"
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include "server.c"
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>



int main(int argc, char *argv[]) {
    int exit_code = 0;

    // read the first line and scan into variables to initialize the client
    char buffer[256];
    char addr[256];
    int port;

    fgets(buffer, sizeof(buffer), stdin);
    sscanf(buffer, "init %s %d", addr, &port);

    // Initialize the client
    rpc_client *state = rpc_init_client(addr, port);
    if (state == NULL) {
        exit(EXIT_FAILURE);
    }

    char new_buffer[5];
    char function_name[30];
    rpc_handler handler_cl;
    rpc_data request_data;
    while (1) {
        size_t size = read(state->sockfd, new_buffer, sizeof(new_buffer) - 1);
        new_buffer[size] = '\0';
        if (strncmp(new_buffer, "find", 4) == 0) {
            size_t byte_read = read(state->sockfd, function_name, sizeof(function_name) - 1);
            function_name[byte_read] = '\0';

            // Call find after with find request
            rpc_handle *next = rpc_find(state, function_name);
            if (next == NULL) {
                fprintf(stderr, "ERROR: Function %s does not exist\n", function_name);
                exit_code = 1;
                goto cleanup;
            }
        } else if (strncmp(new_buffer, "call", 4) == 0) {
            read(state->sockfd, function_name, sizeof(function_name));

            // Read the handler
            if (read(state->sockfd, &handler_cl, sizeof(handler_cl)) < 0) {
                perror("reading handler");
                exit(EXIT_FAILURE);
            }

            // Read the request data
            if (read(state->sockfd, &request_data.data1, sizeof(request_data.data1)) < 0) {
                perror("problems reading...");
                exit(EXIT_FAILURE);
            }
            
            if (request_data.data1 != NULL) {
                if (read(state->sockfd, &request_data.data2, sizeof(request_data.data2)) < 0) {
                    request_data.data2 = &request_data.data1;
                    request_data.data2_len = sizeof(request_data.data1);
                } else {
                    request_data.data2_len = sizeof(request_data.data1);
                }
            }

            // Check if the function name exists in registered_func
            if (!is_function_registered(registered_func, function_name)) {
                fprintf(stderr, "ERROR: Function %s does not exist\n", function_name);
                exit_code = 1;
                goto cleanup;
            }

            // Call the remote function
            rpc_handle *next = rpc_find(state, function_name);
            rpc_data *respond = rpc_call(state, next, &request_data);
            int n = write(state->sockfd, respond, sizeof(rpc_data));
            rpc_data_free(respond);
        } else if (strncmp(new_buffer, "close", 5) == 0) {
            fprintf(stderr, "Closing");
            rpc_close_client(state);
            break;
        }
    }

cleanup:
    rpc_close_client(state);

    return exit_code;
}

rpc_client *rpc_init_client(char *addr, int port) {
    rpc_client *new_cl = (rpc_client *)malloc(sizeof(rpc_client));
    if (new_cl == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate memory for client\n");
        return NULL;
    }

    strcpy(new_cl->addr, addr);
    new_cl->port = port;
    new_cl->sockfd = -1;

    char port_c[10];
    sprintf(port_c, "%d", port);
    new_cl->server_state = getaddrinfo(addr, port_c, &new_cl->hints, &new_cl->res);

    if (new_cl->server_state != 0) {
        fprintf(stderr, "ERROR: Failed to resolve the address and port\n");
        free(new_cl);
        return NULL;
    }

    struct addrinfo *rp;
    for (rp = new_cl->res; rp != NULL; rp = rp->ai_next) {
        new_cl->sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (new_cl->sockfd == -1) {
            continue;
        }
        if (connect(new_cl->sockfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            break;
        }
    }

    if (rp == NULL) {
        fprintf(stderr, "ERROR: Failed to connect to the server\n");
        free(new_cl);
        return NULL;
    }

    return new_cl;
}
