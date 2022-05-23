#include "helpers.h"

char* format_params_to_JSON(int params_no, char** params_names, char** params) {
    JSON_Value *value = json_value_init_object();
    JSON_Object *object = json_value_get_object(value);

    for (int i = 0; i < params_no; i++) {
        json_object_set_string(object, params_names[i], params[i]);
    }

    char *serialized_string = json_serialize_to_string_pretty(value);
    char *result = strdup(serialized_string);

    json_free_serialized_string(serialized_string);
    json_value_free(value);

    return result;
}

char* format_book_params_to_JSON(char* title, char*  author,
                            char* genre, int page_count, 
                            char*  publisher) {
    
    JSON_Value *value = json_value_init_object();
    JSON_Object *object = json_value_get_object(value);

    
    json_object_set_string(object, "title", title);
    json_object_set_string(object, "author", author);
    json_object_set_string(object, "genre", genre);
    json_object_set_number(object, "page_count", page_count);
    json_object_set_string(object, "publisher", publisher);

    char *serialized_string = json_serialize_to_string_pretty(value);
    char *result = strdup(serialized_string);

    json_free_serialized_string(serialized_string);
    json_value_free(value);

    return result;
}

char* extract_response(char* response) {
    char* result = strstr(response, "\r\n\r\n");
    if (result[4] == '{' || result[4] =='[') {
        result += 4;
        return result;
    }
    return NULL;
}

char* extract_cookie(char* response) {
    char* cookie = strstr(response, "connect.sid");
    if (cookie) {
        cookie = strtok(cookie, ";");
        return cookie;
    }
    return NULL;
}

char* extract_value(char* response, char* key) {
    char* rsp = extract_response(response);

    JSON_Value *value = json_parse_string(rsp);
    JSON_Object *object = json_value_get_object(value);

    char* str_value = strdup(json_object_get_string(object, key));

    json_value_free(value);

    return str_value;
}

void extract_array(char* response) {
    char* rsp = extract_response(response);

    JSON_Value *value = json_parse_string(rsp);
    JSON_Array *array = json_value_get_array(value);
    JSON_Object *object; 

    int count = json_array_get_count(array);

    // If server response is one book
    if (count == 1) {
        object = json_array_get_object(array, 0);
        printf("TITLE\t\t\"%s\"\nAUTHOR\t\t%s\nPUBLISHER\t%s\nGENRE\t\t%s\nPAGES\t\t%g\n",
        json_object_get_string(object, "title"),
        json_object_get_string(object, "author"),
        json_object_get_string(object, "publisher"),
        json_object_get_string(object, "genre"),
        json_object_get_number(object, "page_count"));
        printf("\n");
    }
    // If server response is all books
    else {
        printf("ID\t\tTITLE\n");
        for (int i = 0; i < json_array_get_count(array); i++) {
            object = json_array_get_object(array, i);
            printf("%g\t\t%s\n",
            json_object_get_number(object, "id"),
            json_object_get_string(object, "title"));
        }
        printf("\n");
    }
    json_value_free(value);
}

void print_error(char* response) {
    printf("\n[ERROR]:\t");
    char* err = extract_value(response, "error");
    if (err) {
        printf("%s\n\n", err);
    } else {
        printf("Something went wrong!\n\n");
    }
    free(err);
}

void print_no_access(void) {
    printf("\n[ERROR]:\tYou don't have access to the library.\n"
    "\t\tEnter the library first by using the [enter_library] command!\n\n");
}

void print_not_logged(void) {
    printf("\n[ERROR]:\tYou are not logged in.\n"
    "\t\tLog in to the server first by using the [login] command!\n\n");
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void compute_message(char *message, const char *line)
{
    strcat(message, line);
    strcat(message, "\r\n");
}

int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag)
{
    struct sockaddr_in serv_addr;
    int sockfd = socket(ip_type, socket_type, flag);
    if (sockfd < 0)
        error("ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = ip_type;
    serv_addr.sin_port = htons(portno);
    inet_aton(host_ip, &serv_addr.sin_addr);

    /* connect the socket */
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    return sockfd;
}

void close_connection(int sockfd)
{
    close(sockfd);
}

void send_to_server(int sockfd, char *message)
{
    int bytes, sent = 0;
    int total = strlen(message);

    do
    {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0) {
            error("ERROR writing message to socket");
        }

        if (bytes == 0) {
            break;
        }

        sent += bytes;
    } while (sent < total);
}

char *receive_from_server(int sockfd)
{
    char response[BUFLEN];
    buffer buffer = buffer_init();
    int header_end = 0;
    int content_length = 0;

    do {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0){
            error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        buffer_add(&buffer, response, (size_t) bytes);
        
        header_end = buffer_find(&buffer, HEADER_TERMINATOR, HEADER_TERMINATOR_SIZE);

        if (header_end >= 0) {
            header_end += HEADER_TERMINATOR_SIZE;
            
            int content_length_start = buffer_find_insensitive(&buffer, CONTENT_LENGTH, CONTENT_LENGTH_SIZE);
            
            if (content_length_start < 0) {
                continue;           
            }

            content_length_start += CONTENT_LENGTH_SIZE;
            content_length = strtol(buffer.data + content_length_start, NULL, 10);
            break;
        }
    } while (1);
    size_t total = content_length + (size_t) header_end;
    
    while (buffer.size < total) {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0) {
            error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        buffer_add(&buffer, response, (size_t) bytes);
    }
    buffer_add(&buffer, "", 1);
    return buffer.data;
}

char *basic_extract_json_response(char *str)
{
    return strstr(str, "{\"");
}
