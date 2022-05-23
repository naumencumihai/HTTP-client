#include "helpers.h"

int main(int argc, char *argv[]) {
    char *message;
    char *response;
    int sockfd = -1;
    int client_running = 1;
    int id, page_count;
    char buffer[BUFLEN];
    char username[USERPASSLEN], password[USERPASSLEN];
    char title[BOOKDETAILLEN], author[BOOKDETAILLEN];
    char publisher[BOOKDETAILLEN], genre[BOOKDETAILLEN];
    int check;
    char **cookies = NULL;
    char* JWT_token = NULL;

    do {
        check = 0;

        // Read command from stdin
        fgets(buffer, BUFLEN, stdin);

        // Remove new line
        buffer[strlen(buffer) - 1] = '\0';

        // Check for trailing non alphabetic characters
        if (isalpha(buffer[0]) == 0)
            continue;

        // Register
        if (strncmp(buffer, R_CMD, R_CMD_LEN) == 0) {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            DIE(sockfd < 0, "Connecting to server failed\n");

            printf("username=");
            fgets(username, USERPASSLEN, stdin);
            printf("password=");
            fgets(password, USERPASSLEN, stdin);

            // Remove new line chararcter
            username[strlen(username) - 1] = '\0';
            password[strlen(password) - 1] = '\0';

            // Format username and password as JSON
            int params_no = 2;

            char **params_names = (char **)calloc(params_no, sizeof(char *));
            params_names[0] = strdup("username");
            params_names[1] = strdup("password");

            char **params = (char **)calloc(params_no, sizeof(char *));
            params[0] = strdup(username);
            params[1] = strdup(password);

            char **body_data = (char **)calloc(1, sizeof(char *));
            int body_data_fields_count = 1;

            body_data[0] = format_params_to_JSON(params_no, params_names, params);

            // Compute message and send it to server
            message = compute_post_request(HOST, REGISTER, JSON,
                                            body_data, body_data_fields_count,
                                            NULL, 0, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Feedback to User
            if (strstr(response, "201 Created"))
                printf("\n[OK]:\t\tUser registered!\n\n");
            else print_error(response);

            // Free allocated memory
            free(params_names);
            free(params);
            if (body_data) {
                if (body_data[0])
                    free(body_data[0]);
                free(body_data);
            }
        }
        // Login
        else if (strncmp(buffer, LI_CMD, LI_CMD_LEN) == 0) {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            DIE(sockfd < 0, "Connecting to server failed\n");

            printf("username=");
            fgets(username, USERPASSLEN, stdin);
            printf("password=");
            fgets(password, USERPASSLEN, stdin);

             // Remove new line chararcter
            username[strlen(username) - 1] = '\0';
            password[strlen(password) - 1] = '\0';

            // Format username and password as JSON
            int params_no = 2;

            char **params_names = (char **)calloc(params_no, sizeof(char *));
            params_names[0] = strdup("username");
            params_names[1] = strdup("password");

            char **params = (char **)calloc(params_no, sizeof(char *));
            params[0] = strdup(username);
            params[1] = strdup(password);

            char **body_data = (char **)calloc(1, sizeof(char *));
            int body_data_fields_count = 1;

            body_data[0] = format_params_to_JSON(params_no, params_names, params);

            // Compute message and send it to server
            message = compute_post_request(HOST, LOGIN, JSON,
                                            body_data, body_data_fields_count,
                                            NULL, 0, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Get session cookie
            cookies = (char**)calloc(1, sizeof(char*));
            if (extract_cookie(response)) {
                cookies[0] = strdup(extract_cookie(response));
            }

            // Feedback to User
            if (strstr(response, "200 OK"))
                printf("\n[OK]:\t\tWelcome %s!\n\n", username);
            else print_error(response);

            // Free allocated memory
            free(params_names);
            free(params);
            if (body_data) {
                if (body_data[0])
                    free(body_data[0]);
                free(body_data);
            }
        }
        // Enter Library
        else if (strncmp(buffer, EL_CMD, EL_CMD_LEN) == 0) {
            // If current User received session cookies (if it is logged)
            if (cookies != NULL) {
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                DIE(sockfd < 0, "Connecting to server failed\n");

                // Compute message and send it to server
                message = compute_get_request(0, HOST, ACCESS, NULL, cookies, 1, NULL);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                if (strstr(response, "200 OK")) {
                    printf("\n[OK]:\t\tNow you have access to the library!\n\n");
                    JWT_token = extract_value(response, "token");
                }
                else print_error(response);
            }
            else print_not_logged();
        }
        // Get all books
        else if (strncmp(buffer, GBS_CMD, GBS_CMD_LEN) == 0) {
            // If current User received the JWT toke (if it aquired access to the library)
            if (JWT_token) {
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                DIE(sockfd < 0, "Connecting to server failed\n");

                message = compute_get_request(0, HOST, BOOKS, NULL, NULL, 0, JWT_token);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // Feedback to User
                if (strstr(response, "200 OK")) {
                    printf("\n[OK]:\t\tHere are all available books, %s!\n\n", username);

                    // Show books
                    extract_array(response);
                }
                else print_error(response); 
            } 
            else print_no_access();
        }
        // Get a book
        else if (strncmp(buffer, GB_CMD, GB_CMD_LEN) == 0) {
            // If current User received the JWT toke (if it aquired access to the library)
            if (JWT_token) {
                printf("id=");
                check = scanf("%d", &id);

                // Checks offered input
                if (check != 1) {
                    printf("\n[ERROR]:\tBad argument offered.\n"
                    "\t\tArgument <id> should be a number!\n\n");
                    scanf("%s", buffer);
                    continue;
                }
            
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                DIE(sockfd < 0, "Connecting to server failed\n");

                char* path = (char*)calloc(LINELEN, sizeof(char));
                sprintf(path, "%s/%d", BOOKS, id);

                message = compute_get_request(0, HOST, path, NULL, NULL, 0, JWT_token);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // Feedback to User
                if (strstr(response, "200 OK")) {
                    printf("\n[OK]:\t\tHere is the requested book, %s!\n\n", username);

                    // Show books
                    extract_array(response);
                }
                else print_error(response);

                free(path);
            } 
            else print_no_access();
        }
        // Add a book
        else if (strncmp(buffer, AB_CMD, AB_CMD_LEN) == 0) {
            // If current User received the JWT toke (if it aquired access to the library)
            if (JWT_token) {
                printf("title=");
                fgets(title, BOOKDETAILLEN, stdin);
                title[strlen(title) - 1] = '\0';
                printf("author=");
                fgets(author, BOOKDETAILLEN, stdin);
                author[strlen(author) - 1] = '\0';
                printf("genre=");
                fgets(genre, BOOKDETAILLEN, stdin);
                genre[strlen(genre) - 1] = '\0';
                printf("publisher=");
                fgets(publisher, BOOKDETAILLEN, stdin);
                publisher[strlen(publisher) - 1] = '\0';
                printf("page_count=");
                check += scanf("%d", &page_count);

                // Checks offered input
                if (check != 1) {
                    printf("\n[ERROR]:\tBad argument offered.\n"
                    "\t\tArgument <page_count> should be a number!\n\n");
                    scanf("%s", buffer);
                    continue;
                }

                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                DIE(sockfd < 0, "Connecting to server failed\n");

                char **body_data = (char **)calloc(1, sizeof(char *));
                int body_data_fields_count = 1;

                body_data[0] = format_book_params_to_JSON(title, author, genre, page_count, publisher);

                // Compute message and send it to server
                message = compute_post_request(HOST, BOOKS, JSON,
                                                body_data, body_data_fields_count,
                                                NULL, 0, JWT_token);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // Feedback to User
                if (strstr(response, "200 OK"))
                    printf("\n[OK]:\t\tBook \"%s\" by %s added to library!\n\n", title, author);
                else print_error(response);

                if (body_data) {
                    if (body_data[0])
                        free(body_data[0]);
                    free(body_data);
                }
            }
            else print_no_access();
        }
        // Delete a book
        else if (strncmp(buffer, DB_CMD, DB_CMD_LEN) == 0) {
            if (JWT_token) {
                printf("id=");
                check = scanf("%d", &id);
                // Checks offered input
                if (check != 1) {
                    printf("\n[ERROR]:\tBad argument offered.\n"
                    "\t\tArgument <id> should be a number!\n\n");
                    scanf("%s", buffer);
                    continue;
                }

                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                DIE(sockfd < 0, "Connecting to server failed\n");

                char* path = (char*)calloc(LINELEN, sizeof(char));
                sprintf(path, "%s/%d", BOOKS, id);

                // Compute message and send it to server
                message = compute_get_request(1, HOST, path, NULL, NULL, 0, JWT_token);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // Feedback to User
                if (strstr(response, "200 OK"))
                    printf("\n[OK]:\t\t Specified book removed from library!\n\n");
                else print_error(response);

                free(path);
            }
            else print_no_access();
        }
        // Logout
        else if (strncmp(buffer, LO_CMD, LO_CMD_LEN) == 0) {
            // If current User received session cookies (if it is logged)
            if (cookies != NULL) {
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                DIE(sockfd < 0, "Connecting to server failed\n");

                // Compute message and send it to server
                message = compute_get_request(0, HOST, LOGOUT, NULL, cookies, 1, NULL);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                if (strstr(response, "200 OK"))
                    printf("\n[OK]:\t\tGoodbye, %s!\n\n", username);
                else print_error(response);

                // Reset session cookies
                cookies = NULL;
                // Reset JWT token
                JWT_token = NULL;
            }
            else print_not_logged();
        }
        // Exit
        else if (strncmp(buffer, EXT_CMD, EXT_CMD_LEN) == 0) {
            printf("\n\t\tGoodbye!\n\n");
            client_running = 0;
        }
        // Wrong command
        else {
            printf("\n[ERROR]:\tCommand not recognized.\n\t\tAccepted commands include:\n\n"
            "\t\t[register]\t[login]\t\t[enter_library]\n"
            "\t\t[get_books]\t[get_book]\t[add_book]\n"
            "\t\t[delete_book]\t[logout]\t[exit]\n\n");  
        }
    } while(client_running);

    // Free memory for cookie/s and JWT token, if ever used
    if (cookies) {
        if (cookies[0])
            free(cookies[0]);
        free(cookies);
    }
    if (JWT_token)
        free(JWT_token);
    
    // Close server connection, if ever opened
    if (sockfd > 0)
        close_connection(sockfd);

    return 0;
}
