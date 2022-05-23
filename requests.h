#ifndef _REQUESTS_
#define _REQUESTS_

/* 	Computes and returns a GET request string (query_params
*	and cookies can be set to NULL if not needed),
*	
*	Alternatively, if param <isDELETE> is not 0,
*	it computes a DELETE request
*/ 
char *compute_get_request(int isDELETE, char *host, char *url, char *query_params,
							char **cookies, int cookies_count, char *auth_token);


/*
*	Computes and returns a POST request string (cookies can be NULL if not needed)
*/
char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
							int body_data_fields_count, char** cookies, int cookies_count, char *auth_token);

#endif
