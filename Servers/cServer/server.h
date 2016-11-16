/**************************** Import Libraries *****************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <pthread.h>
/**************************** Supported Endpoints **************************************/
#include "tax_endpoint.h"
/***************************** Import global variables *********************************/
#include "globals.h"
/**************************** Function Declarations ************************************/
int extract_callback(char**, char*);
int extract_endpoint_params(char*, char*, char*, char*);
int decode_params(char*, char*);
void *process_connection(void*);