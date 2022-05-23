#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "buffer.h"
#include "requests.h"
#include "parson.h"

#ifndef _HELPERS_
#define _HELPERS_

#define HEADER_TERMINATOR "\r\n\r\n"
#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)

#define BUFLEN 			8192
#define LINELEN 		1000
#define USERPASSLEN 	200
#define BOOKDETAILLEN 	200

// Host and port macros
#define HOST 		"34.241.4.235"
#define PORT 		8080

// Paths macros
#define REGISTER 	"/api/v1/tema/auth/register"
#define LOGIN 		"/api/v1/tema/auth/login"
#define ACCESS 		"/api/v1/tema/library/access"
#define BOOKS 		"/api/v1/tema/library/books"
#define LOGOUT 		"/api/v1/tema/auth/logout"
#define JSON 		"application/json"

// Accepted commands, and their length macros
#define R_CMD 		"register"
#define LI_CMD 		"login"
#define EL_CMD 		"enter_library"
#define GBS_CMD 	"get_books"
#define GB_CMD 		"get_book"
#define AB_CMD 		"add_book"
#define DB_CMD 		"delete_book"
#define LO_CMD 		"logout"
#define EXT_CMD 	"exit"

#define R_CMD_LEN	8
#define LI_CMD_LEN	5
#define EL_CMD_LEN	13
#define GBS_CMD_LEN	9
#define GB_CMD_LEN	8
#define AB_CMD_LEN	8
#define DB_CMD_LEN	11
#define LO_CMD_LEN	6
#define EXT_CMD_LEN	4

// macro used for treating errors
#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

// --------- My helper functions ----------

/*
*   Formats parameters as JSON,
*   Works only if parameters are strings.
*/
char* format_params_to_JSON(int params_no, char** params_names, char** params);

/*
*   Formats book parameters as JSON.
*   Works for book details.
*/
char* format_book_params_to_JSON(char* title, char*  author,
                            char* genre, int page_count, 
                            char*  publisher);

/*
*   Extracts server response.
*/
char* extract_response(char* response);

/* 
*   Extracts session cookie from server response.
*/
char* extract_cookie(char* response);

/* 
*   Extracts value from server response,
*   Works only if server returns one value, not an Array.
*/
char* extract_value(char* response, char* key);

/* 
*   Extracts and prints JSON array values.
*   Works for both [get_books] and [get_book] commands.
*/
void extract_array(char* response);

/*
*   Prints error response from server
*/
void print_error(char* response);

/*
*   Prints that user doesn't have access to library
*/
void print_no_access(void);

/*
*   Prints that user isn't logged in
*/
void print_not_logged(void);

// ----------------------------------------

// shows the current error
void error(const char *msg);

// adds a line to a string message
void compute_message(char *message, const char *line);

// opens a connection with server host_ip on port portno, returns a socket
int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag);

// closes a server connection on socket sockfd
void close_connection(int sockfd);

// send a message to a server
void send_to_server(int sockfd, char *message);

// receives and returns the message from a server
char *receive_from_server(int sockfd);

// extracts and returns a JSON from a server response
char *basic_extract_json_response(char *str);

#endif
