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
void* handle_client_request(void* ptr);
/**
rpc_data* echo_2(rpc_data* x);
rpc_data* sleep_h(rpc_data*x);
rpc_data* add2_2(rpc_data*x);
rpc_data* bad_null(rpc_data*x);
rpc_data* bad_data2_2(rpc_data*x);
rpc_data* bad_data2_1(rpc_data*x);
rpc_data *add2_i8(rpc_data *);
*/

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
//fprintf(stdout,"rpc_init_server: instance 0, port %d\n", port);
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
    
    //fprintf(stdout, "rpc_register: instance 0, %s (handler) as %s\n", name, name);

    if (srv == NULL || name == NULL || handler == NULL){
        return -1;
    }
    
    

   // record all the handlers in the servers 

   srv->map = (FunctionMap*)realloc(srv->map, (srv->number_handlers + 1) * sizeof(FunctionMap));

   if (srv->map == NULL)
      return -1;

    strcpy(srv->map[srv->number_handlers].name,name);
    srv->map[srv->number_handlers].handler = handler;
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
    /*Make sure the service name is passed in correctly so getaddressinfo not
    causing error*/

    //fprintf(stdout, "rpc_serve_all: instance 0\n");

    char service[20];
    sprintf(service, "%d", srv->port);

    srv->sockfd = create_listening_socket(service);
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    // argument read from client side



    if (listen(srv->sockfd, 10) < 0) {
        perror("Error listening ");
        return;
    }

    while (1) {
        
        // need debug from this line
        /* this will require a client to connect to server to enable the connection */
        srv->cur_client = accept(srv->sockfd, (struct sockaddr*)&client_addr, &client_len);
        if (srv->cur_client< 0) {
            perror("Error accepting connection");
            continue;
        }

        /*rpc_data* response = malloc(sizeof(rpc_data));
        memset(response, 0, sizeof(rpc_data));
        if (read(client_sockfd, response, sizeof(rpc_data)) < 0) {
            perror("Error reading from socket");
            return;
        }*/
        pthread_t client_thread;

        if (pthread_create(&client_thread, NULL, handle_client_request, (void*)srv) < 0){
            perror("error creating thread");
            close(srv->cur_client);
            exit(EXIT_SUCCESS);
        }

        pthread_detach(client_thread);


        //("the program continue to run \n");
        void* thread_result;
        pthread_join(client_thread, &thread_result);

        // Check if client has closed the connection
        if (thread_result == PTHREAD_CANCELED) {
            //printf("running stops\n");
            break;
        }else{
            continue;
        }

        }
        //free(response);
}

void* handle_client_request(void* ptr){

    rpc_server* srv = (rpc_server*)ptr;
    int count =0;
    while(1){
        count++;
        //printf("%d\n", count);
        char request [30];
        // sets the request to 0 to avoid the arguments passed into request memory has being cleared
        memset(request,0,sizeof(request));
        

        if (read(srv->cur_client, request, sizeof(request)) < 0) {
            // stops once no more request from the client 
            close(srv->cur_client);
            pthread_exit(NULL);
        }

        //printf("the request string is %s\n", request);

        if (strlen(request) == 0){
            break;
        }


        if(strcmp(request, "rpc_find") == 0){

            if (write(srv->cur_client,&srv->number_handlers,sizeof(srv->number_handlers)) < 0){
                perror("Error writing from socket");
                exit(EXIT_FAILURE);
            }

            if (write(srv->cur_client, srv->map, sizeof(FunctionMap)* srv->number_handlers) < 0) {
                perror("Error writting from socket");
                exit(EXIT_FAILURE);
            }
        }

        if(strcmp(request, "rpc_call") == 0){

            /*EDIT FROM HERE, SO THE SERVER NO LONGER SENDING THE FUNCTIONMAP ARRAY TO CLIENT 
            INSTEAD CALLING IT LOCALLY AND SEND RESULT SO CLIENT CAN PRINTF THE OUTPUT 
            */


           /* TO CALL THE FUNCTION, ONLY REQUIRES THE NAME AND DATA */

            //printf("first occurence\n");


            rpc_data* args = malloc(sizeof(rpc_data));
            
            int old = args->data1;
            int data2L = (int)args->data2_len;
            //printf("%d\n", old);
            if(read(srv->cur_client, args,sizeof(rpc_data)) < 0){
                perror("Eorror reading the rpc_data");
                exit(EXIT_FAILURE);
            }
            if (old == args->data1 && data2L == args->data2_len){
                break;
            }
            //printf("read first \n%d", args->data1);
            if(args->data2_len > 0){
                args->data2 = (void*)malloc(args->data2_len);
                if (read(srv->cur_client, args->data2, args->data2_len) < 0) {
                    perror("Error reading rpc_data2");
                    exit(EXIT_FAILURE);
            }
            }
            /*
            size_t name_length;
            if (read(srv->cur_client, &name_length, sizeof(name_length)) < 0) {
                perror("Error reading from socket");
                exit(EXIT_FAILURE);
            }*/

            char name[30];
            if(read(srv->cur_client, name, sizeof(name)) < 0){
                perror("Eorror reading from the name");
                exit(EXIT_FAILURE);
        }



            // potential segmentation faults
            // needs to malloc memory for data2 since its a void pointer
          

            /* retrive the handler and gets the result*/

            rpc_data* result;
            rpc_handler handler_call;
            for(int i = 0; i < srv->number_handlers; i++){
                if (strcmp(srv->map[i].name, name) == 0){

            // program failed at this stage caused an segmentation error
                handler_call = srv->map[i].handler;
            // 
                result = handler_call(args);

                //fprintf(stdout, "the result data 1 is %d\n", result->data1);
                break;

            }
        }

            // send the data over the socket (BAD ADDRESS ERROR ADDRESS NOT SENT)
            
            if (result == NULL){
                //design a speical data formate to represent NULL result
                rpc_data nullD = {.data1 = 666, .data2_len = 0, .data2 = NULL};
                if(write(srv->cur_client, &nullD,sizeof(rpc_data)) < 0){
                    perror("Eorror sendinig null");
                    exit(EXIT_FAILURE);
                }
                continue;

            }

            if ((result->data2_len == 0 && result->data2!= NULL) ||(result->data2_len != 0 && result->data2== NULL)){
                //design a speical data formate to represent NULL result
                rpc_data nullD = {.data1 = 666, .data2_len = 0, .data2 = NULL};
                if(write(srv->cur_client, &nullD,sizeof(rpc_data)) < 0){
                    perror("Eorror sendinig null");
                    exit(EXIT_FAILURE);
                }
                continue;

            }


            if(write(srv->cur_client, result,sizeof(rpc_data)) < 0){
                perror("Eorror sendinig rusult");
                exit(EXIT_FAILURE);
            } 

            if (result->data2_len > 0) {
                if (write(srv->cur_client, result->data2, result->data2_len) < 0) {
                    perror("Error writing data2");
                    exit(EXIT_FAILURE);
            }

            // make sure all the extra bytes in the server is cleared into a new buffer


}

            rpc_data_free(args);
        }
    }

    close(srv->cur_client);
    //pthread_exit(NULL);
    return NULL;
}

/*
rpc_data* echo_2(rpc_data* x){
    

    printf("calling echo2  data1 = %d and data2 sha256 = %p: \n", x->data1, (x->data2));
    printf("call of echo2 received data1 = %d, call of echo2 received data1 = %p \n",x->data1, (x->data2));

    const char* funcName = __func__;
    x->data2 = (void*)funcName;

    return x;
}
rpc_data* sleep_h(rpc_data*x){
    printf("calling sleep, with argument %d\n", x->data1);
    printf("call of sleep received result %d\n", x->data1);

    const char* funcName = "sleep";
    x->data2 = (void*)funcName;

    return x;
}
*/

/*
rpc_data* add2_2(rpc_data* x){
    
    char n1 = x->data1;
    char n2 = ((char *)x->data2)[0];
    printf("calling add2_2, with arguments %d %d \n", n1, n2);
    int res = n1 + n2;

    //Prepare response 
    rpc_data *out = malloc(sizeof(rpc_data));
    assert(out != NULL);
    out->data1 = res;
    out->data2_len = 0;
    const char* funcName = __func__;
    out->data2 = (void*)funcName;
    return out;
}
*/

/*
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
rpc_data *add2_i8(rpc_data *in) {

    if (in->data2 == NULL || in->data2_len != 1) {
        return NULL;
    }

    char n1 = in->data1;
    char n2 = ((char *)in->data2)[0];


    printf("add2: arguments %d and %d\n", n1, n2);
    int res = n1 + n2;

    

    rpc_data *out = malloc(sizeof(rpc_data));
    assert(out != NULL);
    out->data1 = res;
    out->data2_len = 0;
    const char* funcName = __func__;
    out->data2 = (void*)funcName;
    return out;
}
*/




struct rpc_client {
    /* Add variable(s) for client state */
    char addr[256];
    int port;
    int sockfd;
    int server_state;
    int socket_type;
    struct addrinfo hints,*res;
    int closed;
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

    //fprintf(stdout, "rpc_init_client: instance 0, addr %s, port %d \n", addr, port);
    rpc_client* new_cl = (rpc_client*)malloc(sizeof(rpc_client));

    strcpy(new_cl->addr, addr);
    new_cl->port = port;
    new_cl->sockfd = -1;
    new_cl->closed = 0;

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

/*this function needs debug*/
rpc_handle *rpc_find(rpc_client *cl, char *name) {
/*At the client, tell the subsystem what details are required to place a call. The return value is a handle (not handler)
for the remote procedure, which is passed to the following function.
If name is not registered, it should return NULL. If any of the arguments are NULL then NULL should be returned. If
the find operation fails, it returns NULL*/


// change sizeof(request) to strlen(request) to ensure the string is properly sent
    if (cl->closed){
        return NULL;
    }

    char request[] ="rpc_find";
// Invalid read of size 4
    if (write(cl->sockfd, request,strlen(request) + 1) < 0){
        perror("Error writinig request (rpc_find)");
        exit(EXIT_FAILURE);
    }

    //fprintf(stdout, "rpc_find: instance 0, %s\n", name);
    int register_num;
    FunctionMap* map_r;

    rpc_handle* handle1 = (rpc_handle*)malloc(sizeof(rpc_handle));
    handle1->client = (rpc_client*)malloc(sizeof(rpc_client));
    handle1->client->sockfd = cl->sockfd;

    // error happens from first read

    if (read(cl->sockfd, &register_num,sizeof(register_num)) < 0){
        perror("Error reading");
        exit(EXIT_FAILURE);
    }
    map_r = (FunctionMap*)malloc(sizeof(FunctionMap) * register_num);

    if(read(cl->sockfd, map_r,sizeof(FunctionMap) * register_num) < 0){
        perror("Eorror reading");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < register_num; i++){
        if(strcmp(map_r[i].name, name) == 0){
            //fprintf(stdout, "rpc_find: instance 0, returned handle for function %s\n", name);
            strcpy(handle1->name, name);
            return handle1; 
        }
    }

    //fprintf(stdout, "rpc_find: instance 0, wasn't able to find function %s\n", name);


    return NULL;
}

rpc_data *rpc_call(rpc_client *cl, rpc_handle *h, rpc_data *payload) {
/*This function causes the subsystem to run the remote procedure, and returns the value.
If the call fails, it returns NULL. NULL should be returned if any of the arguments are NULL. If this returns a
non-NULL value, then it should dynamically allocate (by malloc) both the rpc_data structure and its data2
field. The client will free these by rpc_data_free (defined below).*/
    if(payload == NULL || h == NULL || cl == NULL || cl->closed){
        return NULL;
    }


    /*same code implementation as rpc_find*/

    if (payload->data1 == 0 && payload->data2 == NULL){
        return NULL;
    }
    if (h->name == NULL){
        return NULL;
    }


    char request[] ="rpc_call";

    //error cuase bakc this line

    if (write(cl->sockfd, request,strlen(request) + 1) < 0){
        perror("Error writing (request rpc_call)");
        exit(EXIT_FAILURE);
    }

   


/* EDIT THIS CODE SO THE FUNCTIONS ALWAYS CALLED ON THE SERVER END INSTEAD OF CLIENT END
ONLY THE RESULT WILL BE SEND BACK TO THE CLIENT END*/

// NEED EDIT FROM THIS LINE  (SEND THE DATA TO THE SERVER AND LET SERVER CALL THE FUNCTION)


// SEND THE DATA TO THE CLIENT -> RECEIVES THE RESULT (MALLOC FOR BOTH RPC_DATA AND DATA2)

    // send the initial arguments to the server so the server can prints the arguments

    // data sent was incorrect -> needs further debugging
    if(write(cl->sockfd, payload,sizeof(rpc_data)) < 0){
        perror("Eorror reading");
        exit(EXIT_FAILURE);
    }
    if (payload->data2 != NULL){
        // try it with data->data2len
        if(write(cl->sockfd, payload->data2,payload->data2_len) < 0){
            perror("Eorror reading");
            exit(EXIT_FAILURE);
    }
    }

    /*size_t name_length = strlen(h->name) + 1; 
     // Calculate the length of the string

    if (write(cl->sockfd, &name_length, sizeof(name_length)) < 0) {
        perror("Error writing to socket");
        exit(EXIT_FAILURE);
    }   */
    /*send over data len first let the server allocate memory*/

    //printf("the sending string is :%s\n", h->name);

    if(write(cl->sockfd, h->name,strlen(h->name)+1) < 0){
        perror("Eorror wrting name to the socket");
        exit(EXIT_FAILURE);
    }
    rpc_data* result = (rpc_data*)malloc(sizeof(rpc_data));
    

    if (read(cl->sockfd, result, sizeof(rpc_data)) < 0) {
        return NULL;
        perror("Error reading from socket");
        exit(EXIT_FAILURE);
    }
    // go through null check firtst

    if (result->data1 == 666 && result->data2 == NULL){
        return NULL;
    }

    if (result->data2_len > 0){
        result->data2 = malloc(result->data2_len);
        if (read(cl->sockfd, result->data2, result->data2_len) < 0) {
            perror("Error reading from socket");
            exit(EXIT_FAILURE);
    }
    }   


    // prints the result



    /*end of the call, remove all the extra bytes in client's buffer*/

                
    return result;
}


void rpc_close_client(rpc_client *cl) {
    if (cl == NULL) {
        return;
    }
    cl->server_state = -1;
    cl->closed =1;

    if (cl->res != NULL) {
        freeaddrinfo(cl->res);
        cl->res = NULL; // Set to NULL to avoid double-freeing
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
