#ifndef WEBSERVER_H_
#define WEBSERVER_H_

//Direzione Flusso
#define REQUEST_IN 1
#define RESPONSE_OUT 2

#define POLLFD_LENGTH 10

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
  
  char buffer[1000];
  struct header * http_headers_head;
  char numero_header;
  int offset;

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
struct http_state * new_http_connection(struct http_state * head);

int close_http_connection(struct http_state * connection)

int verify(char *buffer);
#endif
