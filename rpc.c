#define _POSIX_C_SOURCE 200112L
#include "rpc.h"
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#define MAX_NUM 100


int create_listening_socket(char* service);
void handle_client_request(void* ptr);
rpc_data* echo_2(rpc_data* x);
rpc_data* sleep(rpc_data*x);
rpc_data* add2_2(rpc_data*x);
rpc_data* bad_null(rpc_data*x);
rpc_data* bad_data2_2(rpc_data*x);
rpc_data* bad_data2_1(rpc_data*x);
rpc_data *add2_i8(rpc_data *);

typedef struct {
    char name[30];
    rpc_handler handler;
} FunctionMap;

struct rpc_server {
    /* Add variable(s) for server state */
    int port;
    char name [256];
    int server_state; // 0 1 2 on non-inititate, listen, accepted
    int cur_client;
    int sockfd;
    struct addrinfo hints, *res;
    FunctionMap* map;
    int number_handlers;
};

rpc_server *rpc_init_server(int port) {
/*Called before rpc_register. Use this for whatever you need. It should return a pointer to a struct (that you
define) containing server state information on success and NULL on failure.*/

rpc_server * new_server = (rpc_server*)malloc(sizeof(rpc_server));
if (new_server != NULL){
    new_server->server_state = 1;
    new_server->port = port;
    return new_server;
}
    return NULL;
}

int rpc_register(rpc_server *srv, char *name, rpc_handler handler) {

/*At the server, let the subsystem know what function to call when an incoming request is received.
It should return a non-negative number on success (possibly an ID for this handler, but a constant is fine), and -1
on failure. If any of the arguments is NULL then -1 should be returned.*/

    if (srv == NULL || name == NULL || handler == NULL){
        return -1;
    }
    rpc_handler cur;
    if (strcmp(name, "add2") == 0) {
        cur = add2_i8;
        
    } else if (strcmp(name, "echo2") == 0) {
        cur = echo_2;
    
    } else if (strcmp(name, "add2_2") == 0) {
        cur = add2_2;
        
    }else if (strcmp(name, "bad_null") == 0) {
        cur = bad_null;
        
    }else if (strcmp(name, "bad_data2_2") == 0) {
        cur = bad_data2_2;
        
    }else if (strcmp(name, "bad_data2_1") == 0) {
        cur = bad_data2_1;
        
    }else if (strcmp(name, "sleep") == 0) {
        cur = sleep;
        
    }
    

    // create the socket;
   /* srv->sockid = socket(AF_INET6,SOCK_STREAM, 0);
    if(srv->sockid < 0){
        perror("Error");
        return -1;
   }*/

   /*memset(&srv->hints,0, sizeof(srv->hints));
   srv->hints.ai_family = AF_INET6;
   srv->hints.ai_socktype = SOCK_STREAM;
   srv->hints.ai_flags = AI_PASSIVE;

   char port_n[10];
   sprintf(port_n, "%d",srv->port);
   srv->server_state = getaddrinfo(NULL, port_n, &srv->hints, &srv->res);
   int listenfd = socket(srv->res->ai_family,srv->res->ai_socktype,srv->res->ai_protocol);
   srv->listenfd = listenfd;
   // setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&srv->res,sizeof(srv->res));


   // bind and listen 

    bind(listenfd, srv->res->ai_addr,srv->res->ai_addrlen);

    // allow max of 10 client 
    listen(listenfd, 10);*/
   

   // record all the handlers in the servers 

   srv->map = (FunctionMap*)realloc(srv->map, (srv->number_handlers + 1) * sizeof(FunctionMap));

   if (srv->map == NULL)
      return -1;

    strcpy(srv->map[srv->number_handlers].name,name);
    srv->map[srv->number_handlers].handler = cur;
    srv->number_handlers++;

    return srv->number_handlers;

}


void rpc_serve_all(rpc_server *srv) {

/*This function will wait for incoming requests for any of the registered functions, or rpc_find, on the port specified
in rpc_init_server of any interface. If it is a function call request, it will call the requested function, send a
reply to the caller, and resume waiting for new requests. If it is rpc_find, it will reply to the caller saying whether
the name was found or not, or possibly an error code.
This function will not usually return. It should only return if srv is NULL or you’re handling SIGINT (not a requirement).*/


    if (srv == NULL) {
        return;
    }

    srv->sockfd = create_listening_socket("service");
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    // argument read from client side



    if (listen(srv->sockfd, 10) < 0) {
        perror("Error listening ");
        return;
    }

    while (1) {
        
        srv->cur_client = accept(srv->sockfd, (struct sockaddr*)&client_addr, &client_len);
        if (srv->cur_client< 0) {
            perror("Error accepting connection");
            return;
        }

        /*rpc_data* response = malloc(sizeof(rpc_data));
        memset(response, 0, sizeof(rpc_data));
        if (read(client_sockfd, response, sizeof(rpc_data)) < 0) {
            perror("Error reading from socket");
            return;
        }*/
        pthread_t client_thread;

        if (pthread_create(client_thread, NULL, handle_client_request, (void*)srv) < 0){
            perror("error creating thread");
            close(srv->cur_client);
            exit(EXIT_FAILURE);
        }


        

        /**
        rpc_data* handled_response;
        for (int i = 0; i < srv->number_handlers; i++){
            rpc_handler cur_handler = srv->handlers[i];
            handled_response = cur_handler(response);
            if (write(client_sockfd, handled_response, sizeof(rpc_data)) < 0) {
                perror("Error writing to socket");
                return;
        }
        }
        */

        close(srv->cur_client);

        }
        //free(response);
}

void handle_client_request(void* ptr){

    rpc_server* srv = (rpc_server*)ptr;

    char request[30];
    if (read(srv->cur_client, request, sizeof(request)) < 0) {
        perror("Error reading from socket");
        close(srv->cur_client);
        pthread_exit(NULL);
    }


    if(strcmp(request, "rpc_find") == 0){

        if (write(srv->cur_client,&srv->number_handlers,sizeof(srv->number_handlers)) < 0){
            perror("Error writing from socket");
            return;
        }

        if (write(srv->cur_client, srv->map, sizeof(srv->map)) < 0) {
            perror("Error writting from socket");
            return;
        }
    }

     if(strcmp(request, "rpc_call") == 0){

        if (write(srv->cur_client,&srv->number_handlers,sizeof(srv->number_handlers)) < 0){
            perror("Error writing from socket");
            return;
        }

        if (write(srv->cur_client, srv->map, sizeof(srv->map)) < 0) {
            perror("Error writting from socket");
            return;
        }
        rpc_data* args;
        rpc_data* args2;
        if(read(srv->cur_client, &args,sizeof(args)) < 0){
            perror("Eorror reading");
            exit(EXIT_FAILURE);
    }
        if(read(srv->cur_client, &args2,sizeof(args2)) < 0){
            perror("Eorror reading");
            exit(EXIT_FAILURE);
    }

        if(args2 != NULL){
            args2->data2 = (char*)args2->data2;
            printf("%s : arguments %d and %d\n", (char*)args2->data2, args->data1, *(int*)args->data2);
        }else{
            printf("null: called");
        }
  
    
}
}


rpc_data* echo_2(rpc_data* x){
    

    printf("calling echo2  data1 = %d and data2 sha256 = %p: \n", x->data1, (x->data2));
    printf("call of echo2 received data1 = %d, call of echo2 received data1 = %p \n",x->data1, (x->data2));

    const char* funcName = __func__;
    x->data2 = (void*)funcName;

    return x;
}
rpc_data* sleep(rpc_data*x){
    printf("calling sleep, with argument %d\n", x->data1);
    printf("call of sleep received result %d\n", x->data1);

    const char* funcName = __func__;
    x->data2 = (void*)funcName;

    return x;
}

rpc_data* add2_2(rpc_data* x){
    
    char n1 = x->data1;
    char n2 = ((char *)x->data2)[0];
    printf("calling add2_2, with arguments %d %d \n", n1, n2);
    int res = n1 + n2;

    /* Prepare response */
    rpc_data *out = malloc(sizeof(rpc_data));
    assert(out != NULL);
    out->data1 = res;
    out->data2_len = 0;
    const char* funcName = __func__;
    out->data2 = (void*)funcName;
    return out;
}

rpc_data* bad_null(rpc_data*x){

    char n1 = x->data1;
    char n2 = ((char *)x->data2)[0];

    printf("calling bad_null, with arguments %d %d...", n1, n2);

    printf("call of bad_null failed");
    
    return NULL;

}
rpc_data* bad_data2_1(rpc_data*x){


    char n1 = x->data1;
    char n2 = ((char *)x->data2)[0];

    printf("calling bad_data2_1, with arguments %d %d...", n1, n2);

    printf("call of bad_data2_1 failed");
    
    const char* funcName = __func__;
    x->data2 = (void*)funcName;
    return x;

}
rpc_data* bad_data2_2(rpc_data*x){

    char n1 = x->data1;
    char n2 = ((char *)x->data2)[0];

    printf("calling bad_data2_2, with arguments %d %d...", n1, n2);

    printf("call of bad_data2_2 failed");

    const char* funcName = __func__;
    x->data2 = (void*)funcName;
    
    return x;

}





struct rpc_client {
    /* Add variable(s) for client state */
    char addr[256];
    int port;
    int sockfd;
    int server_state;
    int socket_type;
    struct addrinfo hints,*res;
};

struct rpc_handle {
    /* Add variable(s) for handle */
    rpc_client* client;
    char name[20];
};

rpc_client *rpc_init_client(char *addr, int port) {


/*Called before rpc_find or rpc_call. Use this for whatever you need. The string addr and integer port are the
text-based IP address and numeric port number passed in on the command line.
The function should return a non-NULL pointer to a struct (that you define) containing client state information on
success and NULL on failure.*/

    rpc_client* new_cl = (rpc_client*)malloc(sizeof(rpc_client));

    strcpy(new_cl->addr, addr);
    new_cl->port = port;
    new_cl->sockfd = -1;

    char port_c[10];
    sprintf(port_c, "%d",port);
    new_cl->server_state = getaddrinfo(addr,port_c,&new_cl->hints,&new_cl->res);

    if (new_cl->server_state != 0) {
        // Failed to resolve the address and port
        free(new_cl);
        return NULL;
    }

    struct addrinfo* rp;
    for(rp = new_cl->res; rp!= NULL; rp = rp->ai_next){
        new_cl->sockfd = socket(rp->ai_family,rp->ai_socktype,rp->ai_protocol);
        if (new_cl->sockfd == -1) continue;
        if (connect(new_cl->sockfd, rp->ai_addr, rp->ai_addrlen)!= -1) break;
    }


    return new_cl;
}

rpc_handle *rpc_find(rpc_client *cl, char *name) {
/*At the client, tell the subsystem what details are required to place a call. The return value is a handle (not handler)
for the remote procedure, which is passed to the following function.
If name is not registered, it should return NULL. If any of the arguments are NULL then NULL should be returned. If
the find operation fails, it returns NULL*/

    char request[] ="rpc_find";

    if (write(cl->sockfd, request,sizeof(request)) < 0){
        perror("Error reading");
        exit(EXIT_FAILURE);
    }


    int register_num;
    FunctionMap* map_r;

    rpc_handle* handle1;

    if (read(cl->sockfd, &register_num,sizeof(register_num)) < 0){
        perror("Error reading");
        exit(EXIT_FAILURE);
    }
    map_r = (FunctionMap*)malloc(sizeof(FunctionMap) * register_num);

    if(read(cl->sockfd, &map_r,sizeof(map_r)) < 0){
        perror("Eorror reading");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < register_num; i++){
        if(strcmp(map_r[i].name, name) == 0){
            strcpy(handle1->name, name);
            return handle1; 
        }
    }


    return NULL;
}

rpc_data *rpc_call(rpc_client *cl, rpc_handle *h, rpc_data *payload) {
/*This function causes the subsystem to run the remote procedure, and returns the value.
If the call fails, it returns NULL. NULL should be returned if any of the arguments are NULL. If this returns a
non-NULL value, then it should dynamically allocate (by malloc) both the rpc_data structure and its data2
field. The client will free these by rpc_data_free (defined below).*/
    if(payload == NULL || payload->data1 == NULL || payload->data2 == NULL){
        return NULL;
    }


    /*same code implementation as rpc_find*/


    char request[] ="rpc_call";

    if (write(cl->sockfd, request,sizeof(request)) < 0){
        perror("Error reading");
        exit(EXIT_FAILURE);
    }


    int register_num;
    FunctionMap* map_r;
    rpc_handler handler_call;



    if(read(cl->sockfd, &register_num,sizeof(register_num)) < 0){
        perror("Error reading");
        exit(EXIT_FAILURE);
    }
    map_r = (FunctionMap*)malloc(sizeof(FunctionMap) * register_num);

    if(read(cl->sockfd, &map_r,sizeof(map_r)) < 0){
        perror("Eorror reading");
        exit(EXIT_FAILURE);
    }

    if(write(cl->sockfd, payload,sizeof(payload)) < 0){
        perror("Eorror reading");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < register_num; i++){
        if (strcmp(map_r[i].name, h->name) == 0){
            handler_call = map_r[i].handler;
            rpc_data* result = handler_call(payload);
            if(write(cl->sockfd,result->data2,sizeof(result->data2))<0){
                perror("Eorror reading");
                exit(EXIT_FAILURE);
                }
            
        }
    }

    // write back to server to communicate the data argument



    return NULL;
}

void rpc_close_client(rpc_client *cl) {
    if (cl == NULL) {
        return;
    }
    if (cl->sockfd != -1)
        close(cl->sockfd);

    free(cl);

}



void rpc_data_free(rpc_data *data) {
    if (data == NULL) {
        return;
    }
    if (data->data2 != NULL) {
        free(data->data2);
    }
    free(data);
}


/* COPIED FROM PRACTICAL server.c  used to create listening socket*/
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

	// Reuse port if possible
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
}







