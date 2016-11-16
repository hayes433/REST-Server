#include "server.h"

int main(int argc, char const *argv[])
{
    // Define local variables
    int listenfd = 0, port = 9000;
    struct sockaddr_in sin; 

    STRINGSIZE = 256;
    FILESIZE = 2096;
    BUFFERSIZE = 1000;
    
    // Set port
    if(argc == 2)
    {
        port = atoi(argv[1]);
    }

    //create socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        printf("Error Opening socket\n");
        return -1;
    }

    // Initialize socket params
    memset(&sin, '0', sizeof(sin));
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_family = AF_INET;

    // bind socket
    bind(listenfd, (struct sockaddr*)&sin, sizeof(sin)); 
    socklen_t len = sizeof(sin);

    listen(listenfd, 10); 
    char ipstr[INET6_ADDRSTRLEN];
    socklen_t addrlen = sizeof(sin);
    struct sockaddr_in *s = (struct sockaddr_in *)&sin;
    if (getsockname(listenfd, (struct sockaddr *)&sin,&len) == -1)
        printf("getsockname");
    else
    {
        printf("port number %d\n", ntohs(sin.sin_port));
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
        printf("Ip %s\n", ipstr);
    }
    
    while(1)
    {
        int* connection_fd = malloc(sizeof(int));

        //wait on connection
        *connection_fd = accept(listenfd, (struct sockaddr*)NULL, NULL);

        //read client
        if(connection_fd > 0)
        {
            if (pthread_create((pthread_t *) malloc(sizeof(pthread_t)), NULL, process_connection, (void *) connection_fd)) 
            {
                printf("WARN\n");
            }
        }
    }
    return 0;
}
int extract_endpoint_params(char* buf, char* method, char* endpoint, char* params)
{;
    char* token1;
    char* token2;
    // Get first line of request format will be (if it has params)
    // method resource?params HTTP
    char* line = strtok(buf, "\n");
    strtok(line, " ");
    line = strtok(NULL, " ");
    // Copy the method part of request
    strcpy(method, buf);
    if((token1 = strtok(line, "?")))
    {
        strcpy(endpoint, token1+1);
    }
    if((token2 = strtok(NULL, "?")))
    {
        strcpy(params, token2);
        return 1;
    }
    else
    {
        return 0;
    }
        
}

int decode_params(char* params, char* json)
{
    int i = 2;
    char* c = params;
    // No Params
    if(!*c)
        return 0;
    // Start first two characters of JSON
    json[0] = '{';
    json[1] = '"';


    while(*c)
    {
        if(*c == '=')
        {
            // End of field name
            json[i++] = '"';    // need closing quote
            json[i++] = ':';    // need colon separator
        }
        else if(*c == '&')
        {
            // End of field value
            json[i++] = ',';    // need comma separator
            json[i++] = '"';    // need opening quote
        }
        else
        {
            // Normal character
            json[i++] = *c;
        }
        c++;
    }

    // Closing brace of JSON and null character
    json[i++] = '}';
    json[i] = 0;
    return i;
}

void *process_connection(void* client_id)
{
    // Convert the passed in client id to an int
    int id = *((int*) client_id);

    // Local variables
    char* params = malloc(STRINGSIZE * sizeof(char));
    char* file = malloc(FILESIZE * sizeof(char));
    char* buffer = malloc(BUFFERSIZE * sizeof(char));
    char* params_pointer = params;
    char method[STRINGSIZE];
    char endpoint[STRINGSIZE];
    char response[STRINGSIZE];
    char JSON[STRINGSIZE];
    char header[STRINGSIZE];
    int size = 0, message_length = 0, error = 0, is_file = 0;
    FILE *f;

    // Read entire message from client
    message_length = read(id, buffer, BUFFERSIZE);

    // Extract endpoint and its paramaters from message
    extract_endpoint_params(buffer, method, endpoint, params);

    // Decode tax info     
    size = decode_params(params, JSON);

    // Set standard header
    strcpy(header, "HTTP/1.0 200 OK\r\nContent-type:application/json\r\n\r\n");
    
    // Select Resource
    if(strcmp(endpoint, "taxestimator/taxes") == 0)
    {
        if(strcmp(method, "GET") == 0)
        {
            // Process taxes
            error = !process_taxes(JSON, response);
            if(error)
            {
                // 500 Internal Server Error
                memset(response, '\0', STRINGSIZE); // Erase response
                strcpy(header, "HTTP/1.0 500 Internal Server Error\r\n");
            }
        }
        else 
        {
            // Handle unsupported method
            // 405 Method Not Allowed
            strcpy(header, "HTTP/1.0 405 Method Not Allowed\r\n");
        }
    }
    else if(strcmp(endpoint, "taxestimator/") == 0)
    {
        if(strcmp(method, "GET") == 0)
        {
            is_file = 1;
            strcpy(header, "HTTP/1.0 200 OK\r\nContent-type:text/html\r\n\r\n");
            f = fopen("../../Clients/TaxEstimatorWebsite/TaxEstimator.html", "r");
        }
        else 
        {
            // Handle unsupported method
            // 405 Method Not Allowed
            strcpy(header, "HTTP/1.0 405 Method Not Allowed\r\n");
        }
    }
    else if (strcmp(endpoint, "taxestimator") == 0)
    {
        if(strcmp(method, "GET") == 0)
        {
            strcpy(header, "HTTP/1.1 302 Found\r\nLocation:taxestimator/\r\n\r\n");
        }
        else 
        {
            // Handle unsupported method
            // 405 Method Not Allowed
            strcpy(header, "HTTP/1.0 405 Method Not Allowed\r\n");
        }
    }
    else if(strcmp(endpoint, "taxestimator/TaxEstimator.js") == 0)
    {
        if(strcmp(method, "GET") == 0)
        {
            is_file = 1;
            strcpy(header, "HTTP/1.0 200 OK\r\nContent-type:text/javascript\r\n\r\n");
            f = fopen("../../Clients/TaxEstimatorWebsite/TaxEstimator.js", "r");
        }
        else 
        {
            // Handle unsupported method
            // 405 Method Not Allowed
            strcpy(header, "HTTP/1.0 405 Method Not Allowed\r\n");
        }
    }
    else if(strcmp(endpoint, "taxestimator/TaxEstimator.css") == 0)
    {
        if(strcmp(method, "GET") == 0)
        {
            is_file = 1;
            strcpy(header, "HTTP/1.0 200 OK\r\nContent-type:text/css\r\n\r\n");
            f = fopen("../../Clients/TaxEstimatorWebsite/TaxEstimator.css", "r");
        }
        else 
        {
            // Handle unsupported method
            // 405 Method Not Allowed
            strcpy(header, "HTTP/1.0 405 Method Not Allowed\r\n");
        }
    }
    else if(strcmp(endpoint, "taxestimator/favicon.png") == 0)
    {
        if(strcmp(method, "GET") == 0)
        {
            is_file = 1;
            strcpy(header, "HTTP/1.0 200 OK\r\nContent-type:image/png\r\n\r\n");
            f = fopen("../../Clients/TaxEstimatorWebsite/favicon.png", "r");
        }
        else 
        {
            // Handle unsupported method
            // 405 Method Not Allowed
            strcpy(header, "HTTP/1.0 405 Method Not Allowed\r\n");
        }
    }
    else
    {
        // Handle unsupported resource
        // 404 Not Found
        strcpy(header, "HTTP/1.0 404 Not Found\r\n");
    }

    if(is_file)
    {
        write(id, header, strlen(header));
        while((size = fread(file, 1, FILESIZE*sizeof(char), f)) > 0)
        {
            write(id, file, size);
        }
        fclose(f);
    } 
    else
    {
        strcat(header, response);
        strcpy(response, header);
        size = strlen(response);
        write(id, response, size);
    }

    // TODO: Log event
    if (is_file)
    {
        printf("%s, %s\n", endpoint, method);
    }
    else
    {
        printf("%s, %s, %s\n", endpoint, method, response);
    }
    printf("*************************************************************\n");
    
    // Clean up
    close(id);
    free(buffer);
    free(params_pointer);
    free(client_id);
    return (void *) 0;
}