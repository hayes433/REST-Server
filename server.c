#include "server.h"

int main(int argc, char const *argv[])
{
	// Define local variables
	int listenfd = 0, connection_fd = 0, port = 9000;
    struct sockaddr_in sin; 

    STRINGSIZE = 256;
	
	// Set port
	if(argc == 2)
	{
		port = atoi(argv[1]);
	}

	//create socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd == -1)
	{
		printf("Error Oppening socket\n");
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

    	printf("port number %d\n", ntohs(sin.sin_port));
    	inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
    	printf("Ip %s\n", ipstr);

    
    while(1)
	{
		//wait on connection
		connection_fd = accept(listenfd, (struct sockaddr*)NULL, NULL);
		//read client
		if(connection_fd > 0)
		{
			if (pthread_create((pthread_t *) malloc(sizeof(pthread_t)), NULL, process_connection, (void *) &connection_fd)) 
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
int extract_callback(char** params, char* callback)
{
	// Return error if params or callback are null
	if(!params || !callback)
		return 0;
	int i = 0;
	char c;
	char temp[STRINGSIZE];
	memset(temp, '\0', STRINGSIZE);
	strncpy(temp, *params, 8);
	if(strcmp(temp, "callback") == 0)
	{
		// Need to copy callback param and consume it off the params list
		*params += 9;
		c = **params;
		while(c && c != '&')
		{
			callback[i++] = c;
			c = *(++(*params));
		}
		++(*params);
	}
	else
	{
		callback[i]='\0';
	}
	return i;
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
			json[i++] = '"';	// need closing quote
			json[i++] = ':';	// need colon separator
		}
		else if(*c == '&')
		{
			// End of field value
			json[i++] = ',';	// need comma separator
			json[i++] = '"';	// need opening quote
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
	// Local variables
	char* params = malloc(STRINGSIZE * sizeof(char));
	char* params_pointer = params;
	char method[STRINGSIZE];
	char endpoint[STRINGSIZE];
	char callback[STRINGSIZE];
    char JSON[STRINGSIZE];
    char response[STRINGSIZE];
    char header[STRINGSIZE];
    int size = 0, message_length = 0, error = 0;
    char* buffer = malloc(1000*sizeof(char));

    // Convert the passed in client id to an int
    int id = *((int*) client_id);

    // Read entire message from client
	message_length = read(id, buffer, 1000);

	// Extract endpoint and its paramaters from message
	extract_endpoint_params(buffer, method, endpoint, params);

	// Check for callback function
	extract_callback(&params, callback);

	// Decode tax info 	
	size = decode_params(params, JSON);

	// Set standard header
	strcpy(header, "HTTP/1.0 200 OK\r\nContent-type:application/json\r\n\r\n");
	// Select Resource
	if(strcmp(endpoint, "taxes") == 0)
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
	else
	{
		// Handle unsupported resource
		// 404 Not Found
		strcpy(header, "HTTP/1.0 404 Not Found\r\n");
	}
	// Write to client the length of the JSON
	if(callback[0] && error)
	{
		// An error occured
		strcat(header, callback);
		strcpy(response, header);
		strcat(response, "()");
		size = strlen(response);
	}
	else if(callback[0])
	{
		strcat(header, callback);
		strcat(header, "(");
		strcat(header, response);
		strcpy(response, header);
		strcat(response, ")");
		size = strlen(response);
	}
	else
	{
		strcat(header, response);
		strcpy(response, header);
		size = strlen(response);
	}
	write(id, response, size);

	// TODO: Log event
	printf("%s\n", response);
	printf("*************************************************************\n");
	
	// Clean up
	close(id);
	free(buffer);
	free(params_pointer);
	return (void *) 0;
}