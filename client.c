#include "rpc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>



int is_registered(char * function_name, char** registeredFuncs, int count);

int main(int argc, char *argv[]) {
    int exit_code = 0;

    // read the first line and the scans into variable and then init client
    char buffer[256];
    char addr[256];
    int port;

    fgets(buffer, sizeof(buffer),stdin);
    sscanf(buffer, "init %s %d", addr, &port);

    // the client will initstate a socket
    rpc_client *state = rpc_init_client(addr, port);
    if (state == NULL) {
        exit(EXIT_FAILURE);
    }

    //char new_buffer[5];
    char function_name[30];
    rpc_handler handler_cl;
    rpc_data request_data;
    while (fgets(buffer, sizeof(buffer),stdin)!= NULL){
        //size_t size = read(state->sockfd,new_buffer,sizeof(new_buffer)-1);
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strncmp(buffer, "find", 4) == 0){
            sscanf(buffer + 5, "%s", function_name);
            // record all the functions that passed into the client
            // call find after with find request
            rpc_handle *next = rpc_find(state, function_name);


            if (next == NULL) {
                fprintf(stderr, "ERROR: Function add2 does not exist\n");
                exit_code = 1;
                goto cleanup;
    }

        }else if (strncmp(buffer, "call", 4)== 0){

            char function_name1[30];

            sscanf(buffer + 5, "%s %s", function_name, function_name1);

            rpc_handle *next = rpc_find(state, function_name1);

            rpc_data randomd ={.data1 =1, .data2_len =1, .data2=NULL};

            //read(state->sockfd, function_name, sizeof(function_name));
            rpc_call(state,next,&randomd);

            // read the handler
            //if (read(state->sockfd, &handler_cl, sizeof(handler_cl)) < 0 ){
                //perror("reading from handler");
                //exit(EXIT_FAILURE);
            //}
            /*if (read(state->sockfd, &request_data.data1,sizeof(request_data.data1)) < 0){
                perror("problems reading...");
                exit(EXIT_FAILURE);
                if (request_data.data1 != NULL){
                    if (read(state->sockfd, &request_data.data2, sizeof(request_data.data2)) < 0){
                            request_data.data2 = &request_data.data1;
                            request_data.data2_len = sizeof(request_data.data1);
                    }else{
                        request_data.data2_len = sizeof(request_data.data1);
                   }
                }
            }*/

            //if (read(state->sockfd, &request_data.data1, sizeof(request_data.data1)) < 0) {
                //perror("problems reading...");
                //exit(EXIT_FAILURE);
            //}
            /*
            if (request_data.data1 != NULL) {
                if (read(state->sockfd, &request_data.data2, sizeof(request_data.data2)) < 0) {
                    request_data.data2 = &request_data.data1;
                    request_data.data2_len = sizeof(request_data.data1);
                } else {
                    request_data.data2_len = sizeof(request_data.data1);
                }
            }
            rpc_handle *next = rpc_find(state, function_name);
            if (next == NULL) {
                fprintf(stderr, "ERROR: Function %s does not exist\n", function_name);
                exit_code = 1;
                goto cleanup;
        }
            rpc_data* respond = rpc_call(state, next, &request_data);
            int n = write(state->sockfd, respond, sizeof(respond));
            rpc_data_free(respond);
            */

        }else if (strncmp(buffer, "close", 5) == 0){
            fprintf(stderr, "closing");
            rpc_close_client(state);
        }


    }
    
    rpc_handle *handle_add2 = rpc_find(state, "add2");
    /*;
    if (handle_add2 == NULL) {
        fprintf(stderr, "ERROR: Function add2 does not exist\n");
        exit_code = 1;
        goto cleanup;
    }

    for (int i = 0; i < 2; i++) {
        // prepare request 
        char left_operand = i;
        char right_operand = 100;
        rpc_data request_data = {
            .data1 = left_operand, .data2_len = 1, .data2 = &right_operand};

        // prepare data
        rpc_data *response_data = rpc_call(state, handle_add2, &request_data);
        if (response_data == NULL) {
            fprintf(stderr, "Function call of add2 failed\n");
            exit_code = 1;
            goto cleanup;
        }

        // Interpret response 
        assert(response_data->data2_len == 0);
        assert(response_data->data2 == NULL);
        printf("Result of adding %d and %d: %d\n", left_operand, right_operand,
               response_data->data1);
        rpc_data_free(response_data);
    }
*/
    
cleanup:
    if (handle_add2 != NULL) {
        free(handle_add2);
    }

    rpc_close_client(state);
    state = NULL;

    return exit_code;
}


