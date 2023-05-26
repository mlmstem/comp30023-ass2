#include "rpc.h"
#define _POSIX_C_SOURCE 200112L
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

rpc_data *add2_i8(rpc_data *);

//int create_listening_socket(char* service);

int main(int argc, char *argv[]) {
    int sockfd, clientsockfd, n, port;
	char buffer[256];
	struct sockaddr_in client_addr;
	socklen_t client_addr_size;
    rpc_server* state;
    /*
    int sockfd = create_listening_socket("service");

    if (listen(sockfd, 10) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
    client_addr_size = sizeof(client_addr);
    clientsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_size);
    if (clientsockfd < 0){
        perror("connection");
        exit(EXIT_FAILURE);
    }
    */
    while (fgets(buffer, sizeof(buffer),stdin)!= NULL){
        //char buffer[256];
        buffer[strcspn(buffer, "\n")] = '\0';
        // check the first command 
        if (strncmp(buffer, "init",4) == 0){
            int port;
            sscanf(buffer + 5, "%d", &port);
            state = rpc_init_server(port);
            if (state == NULL){
                printf("failed to init");
                exit(EXIT_FAILURE);
        }

        //state->sockfd = sockfd;
        rpc_handler new;

        }else if (strncmp(buffer, "register",8)==0){
            char name[256];
            char handler[256];
            sscanf(buffer + 9, "%s %s", name, handler);
            if (rpc_register(state, name, add2_i8) == -1){
                printf("FAILED register");
                exit(EXIT_FAILURE);
            }

        }else if (strncmp(buffer, "serve",5) == 0){
            rpc_serve_all(state);
        }

    }



  /*
    state = rpc_init_server(3000);
    if (state == NULL) {
        fprintf(stderr, "Failed to init\n");
        exit(EXIT_FAILURE);
    }

    if (rpc_register(state, "add2", add2_i8) == -1) {
        fprintf(stderr, "Failed to register add2\n");
        exit(EXIT_FAILURE);
    }

    rpc_serve_all(state);
    */
//close(sockfd);
//close(clientsockfd);
}

/* Adds 2 signed 8 bit numbers */
/* Uses data1 for left operand, data2 for right operand */


rpc_data *add2_i8(rpc_data *in) {
    // Check data2 
    if (in->data2 == NULL || in->data2_len != 1) {
        return NULL;
    }

    //Parse request 
    char n1 = in->data1;
    char n2 = ((char *)in->data2)[0];

    //Perform calculation 
    printf("add2: arguments %d and %d\n", n1, n2);
    int res = n1 + n2;

    
    // Prepare response 
    rpc_data *out = malloc(sizeof(rpc_data));
    assert(out != NULL);
    out->data1 = res;
    out->data2_len = 0;
    out->data2 = NULL;

    return out;
}



/* COPIED FROM PRACTICAL server.c  used to create listening socket*/
/*
int create_listening_socket(char* service) {
	int re, s, sockfd;
	struct addrinfo hints, *res;

	// Create address we're going to listen on (with given port number)
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET6;       // IPv4
	hints.ai_socktype = SOCK_STREAM; // Connection-mode byte streams
	hints.ai_flags = AI_PASSIVE;     // for bind, listen, accept
	// node (NULL means any interface), service (port), hints, res
	s = getaddrinfo(NULL, service, &hints, &res);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	// Create socket
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	re = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(int)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	// Bind address to the socket
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(res);

	return sockfd;
}*/




