#ifndef WEBSERVER_H_
#define WEBSERVER_H_

//Direzione Flusso
#define REQUEST_IN 1
#define RESPONSE_OUT 2


//Metodi
struct state * newConnection();

int verify(char *buffer);

//Puntatore Header 
struct header{
char * n;
char * v;
};


//Stato Connessione
struct state{
  char flusso;

  char buffer[1000];
  struct header h[20];
  char numero_header;
  int offset;

  int authorized;
  int skip; 
 
  char *file_pointer;
  int fin;
  int content_length;
  int header_size;
  int body_size;
};
#endif
