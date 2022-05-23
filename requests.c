#include "helpers.h"

char *compute_get_request(int isDELETE, char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char* JWT_token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));


    if (query_params != NULL) {
        if (isDELETE)
            sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
        else
            sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        if (isDELETE)
            sprintf(line, "DELETE %s HTTP/1.1", url);
        else 
            sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (cookies != NULL) {
        strcpy(line, "Cookie:");
        for (int i = 0; i < cookies_count - 1; i++) {
            strcat(line, " ");
            strcat(line, cookies[i]);
            strcat(line, ";");
        }

        strcat(line, " ");
        strcat(line, cookies[cookies_count - 1]);
        compute_message(message, line);
    }

    if (JWT_token != NULL) {
        sprintf(line, "Authorization: Bearer %s", JWT_token);
        compute_message(message, line);
    }

    compute_message(message, "");
    return message;
}


char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count, char* JWT_token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));
    int len = 0;

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    memset(line, 0, LINELEN);
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    memset(line, 0, LINELEN);
    if (strcmp(content_type, "application/json") == 0) {
        strcat(body_data_buffer, body_data[0]);
    } else if (strcmp(content_type, "application/x-www-form-urlencoded") == 0) {    
        for (int i = 0; i < body_data_fields_count; i++) {
            strcat(body_data_buffer, body_data[i]);
            if (i != body_data_fields_count - 1) {
                strcat(body_data_buffer, "&");
            }
        }
    }

    len = strlen(body_data_buffer);

    memset(line, 0, LINELEN);
    sprintf(line, "Content-Length: %d", len);
    compute_message(message, line);

    memset(line, 0, LINELEN);
    if (cookies != NULL) {
        strcpy(line, "Cookie:");
        for (int i = 0; i < cookies_count - 1; i++) {
            strcat(line, " ");
            strcat(line, cookies[i]);
            strcat(line, ";");
        }

        strcat(line, " ");
        strcat(line, cookies[cookies_count - 1]);
        compute_message(message, line);
    }

    memset(line, 0, LINELEN);
    if (JWT_token != NULL) {
        sprintf(line, "Authorization: Bearer %s", JWT_token);
        compute_message(message, line);
    }

    compute_message(message, "");

    strcat(message, body_data_buffer);

    free(line);
    return message;
}
