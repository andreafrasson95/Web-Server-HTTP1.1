#ifndef WEBSERVER_H_
#define WEBSERVER_H_

//Direzione Flusso
#define REQUEST_IN 1
#define RESPONSE_OUT 2

#define POLLFD_LENGTH 10

#define BUFFSIZE 10000

struct request_line{
  char * method;
  char * uri;
  char *http_vers;
};

//Puntatore Header 
struct header{
char * name;
char * value;
struct header * next;
};


//Stato Connessione
struct http_state{
  char flusso;
  
  int fd;
  int pollfd_index;  
  
  char buffer[BUFFSIZE];
  struct request_line req_line;
  struct header * http_headers_head;
  struct header * http_headers_tail;
  long int offset;

  int authorized;
  int skip; 
 
  char *file_pointer;
  int fin;
  int content_length;
  int header_size;
  int body_size;
  
  struct http_state * previous;
  struct http_state * next;
};

//Metodi
int create_socket(unsigned short port);

int get_free_index(struct pollfd * pollfd);

struct http_state * get_http_connection(int fd);

struct http_state * new_http_connection();

int parse_request_line(struct http_state * connection);

int parse_http_header(struct http_state * connection);

int check_http_authentication(struct http_state * connection);

int check_etag(struct http_state * connection);

int get_http_resource(struct http_state * connection);

int close_http_connection(struct http_state * connection);

void clearing_headers(struct http_state * connection);
int verify(char *buffer);
#endif
