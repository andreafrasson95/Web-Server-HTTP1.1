#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <fcntl.h>
//#include <sys/sendfile.h>

#include "webserver.h"

int numero;

struct sockaddr_in indirizzo;
struct sockaddr_in indirizzo_remoto;


int primiduepunti;
char *request;
char *response;
char *request_line;
char *method,*uri,*http_ver,*scheme;


int main(){

 int socket_main,connessione,t,lunghezza,p,yes,z,j,i;

 lunghezza=sizeof(struct sockaddr_in);
 yes=1;

 socket_main=socket(AF_INET,SOCK_STREAM,0);
 if(socket_main==-1){
    perror("Socket Fallita");
    return 1;
 }

 if(setsockopt(socket_main, SOL_SOCKET, SO_REUSEADDR , &yes ,sizeof(int))==-1){
    perror("Sockopt");
    return 1;
 }

 indirizzo.sin_family=AF_INET;
 indirizzo.sin_port=htons(80);
 indirizzo.sin_addr.s_addr=0;

 t=bind(socket_main,(struct sockaddr*)&indirizzo,sizeof(struct sockaddr_in));
 if(t==-1){
    perror("Bind Fallita");
    return 1;
 }

 t=listen(socket_main,10);
 if(t==-1){
    perror("Listen Fallita");
    return 1;
 }


 struct pollfd * sockets=malloc(sizeof(struct pollfd)*10);
 struct state * controllo[10];

 sockets[0].fd=socket_main;
 sockets[0].events=POLLIN;
 numero=1;



 while(1){

  p=poll(sockets,numero,-1);
  if(p==-1){
     perror("Poll Fallita");
     return 1;
  }
  //CONTROLLO SE CI SONO CONNESSIONI NUOVE

  if((sockets[0].revents) & POLLIN){
    printf("NEW CONN Connessione Numero: %d\n",numero);
    connessione=accept(socket_main,(struct sockaddr *)&indirizzo_remoto,&lunghezza);
    if(connessione==-1){
       perror("Accept Fallita");
       return 1;
    }
    sockets[numero].fd=connessione;
    sockets[numero].events=POLLIN;
   
    fcntl(connessione,F_SETFL,fcntl(connessione,F_GETFL,NULL) | O_NONBLOCK);   
 
    controllo[numero]=(struct state *)malloc(sizeof(struct state));
    controllo[numero]->flusso=REQUEST_IN;
    numero++;   
  }

  //CONTROLLO SE HO DATA DA LEGGERE
  for(int count=1;count<numero;count++){
   if((sockets[count].revents)){ //Evento nel socket Count
    printf("HO POLLATO %d CON STATO %d \n",sockets[count].revents,controllo[count]->flusso);
    switch(controllo[count]->flusso){

     case(REQUEST_IN):{
   
      if(sockets[count].revents & POLLHUP){
        printf("Ha resettato");      
        close(sockets[count].fd);
        numero--;
        break;
      }
      if(sockets[count].revents & POLLIN){
        printf("DATA IN Connessione: %d\n\n",count);   
        request=controllo[count]->buffer;
        controllo[count]->h[0].n=request;
        controllo[count]->h[0].v=controllo[count]->h[0].n;
        request_line=request;
        for(i=controllo[count]->offset,j=controllo[count]->numero_header;(t=read(sockets[count].fd,request+i,1))>0;i++){
          	if (( i>1) && (request[i]=='\n') && (request[i-1]=='\r')){
	        	primiduepunti=1;
		        request[i-1]=0;
	        	if(controllo[count]->h[j].n[0]==0) break;
	        	controllo[count]->h[++j].n=request+i+1;               
	        	}
        	if (primiduepunti && (request[i]==':')){
         		controllo[count]->h[j].v = request+i+1;
	        	request[i]=0;
	        	primiduepunti=0;
        	}
       }
       controllo[count]->offset=i;
       controllo[count]->numero_header=j;
      
       if (t==-1 && errno==EAGAIN) { 
         break;
       }

       if (t==0){
           printf("Chiudo Connessione\n");
           free(controllo[count]);
           shutdown(sockets[count].fd,SHUT_RD);
           close(sockets[count].fd);
           int t;
           for(t=count;t<numero;t++){ 
             sockets[count]=sockets[count+1];
             controllo[count]=controllo[count+1];
           }
           numero--;
           break;
       }


     
       controllo[count]->offset=0; 
       controllo[count]->flusso=RESPONSE_OUT;
       sockets[count].events=POLLOUT; 
       //STAMPA HEADER E CONTROLLI	
       printf("Request Line: %s\n",request_line);
       for(i=1;i<j;i++){
           printf("%s ===> %s\n",controllo[count]->h[i].n,controllo[count]->h[i].v);
       }   
	

       method = request_line;
       for(i=0;request_line[i]!=' '&& request_line[i];i++){};
       if (request_line[i]!=0) { request_line[i]=0; i++;}
       uri = request_line + i;
       controllo[count]->file_pointer=uri+1;
       for(;request_line[i]!=' '&& request_line[i];i++){};
       if (request_line[i]!=0) { request_line[i]=0; i++;}
       http_ver = request_line +i;

       //printf("\nMethod = %s, URI = %s, Http-Version = %s\n", method, uri, http_ver);
       
       /**********Controllo Header Vari*****************/
       controllo[count]->authorized=0; 
       for(i=1;i<j;i++){
          if(strcmp(controllo[count]->h[i].n,"Authorization")==0){
                  scheme=controllo[count]->h[i].v;
                  int t;
                  for (t=1;t<strlen(controllo[count]->h[i].v);t++)
                          if(controllo[count]->h[i].v[t]==' ')break;
                  controllo[count]->h[i].v[t]='\0';
                  char *autenticazione;
                  autenticazione=&(controllo[count]->h[i].v[t+1]);
                  controllo[count]->authorized=verify(autenticazione);
                  //printf("Scheme: %s Login: %s\n",scheme,autenticazione);
                  /*      
                  if (controllo[count]->authorized==0){
                    response=controllo[count]->buffer;
                    sprintf(response,"HTTP/1.1 401 UNAUTHORIZED\r\nContent-Length: 13\r\nWWW-Authenticate: Basic Realm=\"Merda\"\r\n\r\nMa Dove Vai?!");
                    controllo[count]->header_size=strlen(response);
                    controllo[count]->body_size=0;
                    break;
                  }*/
          }
          if(strcmp(controllo[count]->h[i].n,"If-None-Match")==0 && strcmp(controllo[count]->h[i].v," \"ciccio\"")==0){
                 response=controllo[count]->buffer;
                 sprintf(response,"HTTP/1.1 304 NOT MODIFIED\r\n\r\n");
                 controllo[count]->header_size=strlen(response);
                 controllo[count]->body_size=0;
                 controllo[count]->skip=1;
          }       
       }
       if (controllo[count]->authorized==0){
                 response=controllo[count]->buffer;
                 sprintf(response,"HTTP/1.1 401 UNAUTHORIZED\r\nContent-Length: 13\r\nWWW-Authenticate: Basic Realm=\"Merda\"\r\n\r\nMa Dove Vai?!");
                 controllo[count]->header_size=strlen(response);
                 controllo[count]->body_size=0;
                 break;
       }
      }
	 }break;

     case(RESPONSE_OUT):{
      if(sockets[count].revents & POLLOUT){
       response=controllo[count]->buffer;
       write(1,"DATA OUT\n",9);
       //PROCESSAZIONE FILE
        //PRIMA VOLTA QUI	
       if( (controllo[count]->offset==0) && (controllo[count]->authorized==1) && (controllo[count]->skip==0) ){
         	
        if( !(access(controllo[count]->file_pointer, F_OK))){ 

            printf("File %s Non Aperto\n",controllo[count]->file_pointer);
            sprintf(response,"HTTP/1.1 404 Not Found\r\nServer: Frassi_WebServer\r\nContent-Length: 16\r\n\r\nFile non trovato"); 
            controllo[count]->header_size=strlen(response);
            controllo[count]->body_size=0;
        }  
        else{
           controllo[count]->fin=open(controllo[count]->file_pointer,O_RDONLY);
           struct stat info;
           stat(controllo[count]->file_pointer,&info);
           controllo[count]->content_length=info.st_size;            

           sprintf(response,"HTTP/1.1 200 OK\r\nServer: Frassi_WebServer\r\nEtag: \"ciccio\"\r\nConnection: keep-alive\r\nContent-Length: %d\r\n\r\n",controllo[count]->content_length);
           controllo[count]->header_size=strlen(response);	  
           controllo[count]->body_size=controllo[count]->content_length;
           
        }
         	
       }
         //Riprendo dagli Header
        
       if(controllo[count]->offset<controllo[count]->header_size){
         t=write(sockets[count].fd,response+controllo[count]->offset,controllo[count]->header_size-controllo[count]->offset);
         if(t==-1 && errno==EAGAIN)
           break;

         controllo[count]->offset+=t;
         
         if(controllo[count]->offset<controllo[count]->header_size) break;
         controllo[count]->skip=0;
       }
         //Riprendo dal Body
         
       if( controllo[count]->body_size>0){
          
         controllo[count]->offset=0; 
         t=sendfile(sockets[count].fd,controllo[count]->fin,&controllo[count]->offset,controllo[count]->body_size-controllo[count]->offset);

         if(t==-1 && errno==EAGAIN)
            break;

         controllo[count]->offset+=t;  
          
       }        
       if(controllo[count]->offset<controllo[count]->body_size) break;
     
       controllo[count]->offset=0;
       controllo[count]->numero_header=0;
       sockets[count].events=POLLIN;
       controllo[count]->flusso=REQUEST_IN;
      }//Fine If POLLOUT
     }//Fine Case RESPONSE_OUT
    }//Fine Switch
   }//Fine IF Revents
  }//Fine For
 }//Fine While
}//Fine Main



unsigned char decode(char c){
        int o;
        switch(c){
                case 'A'...'Z': o=c-'A';break;
                case 'a'...'z': o=c-'a'+26;break;
                case '0'...'9': o=c-'0'+52;break;
                case '+'      : o=62;break;
                case '/'      : o=63;break;
                case '='      : o=0;
        }
return o;
}

int verify(char *buffer){
        int i,j;
        j=0;
        char decoded[100];
        for (i=0;i<strlen(buffer);i+=4){
                decoded[j++]=((decode(buffer[i]))<<2 | ((decode(buffer[i+1]))>>4));
                decoded[j++]=((decode(buffer[i+1]))<<4 |((decode(buffer[i+2]))>>2));
                decoded[j++]=((decode(buffer[i+2]))<<6 | ((decode(buffer[i+3]))));
        }
    if(buffer[i+3]=='=') j--;
        if(buffer[i+2]=='=') j--;
        decoded[j]=0;
        printf("Stringa: %s \n",decoded);
        if(strcmp(decoded,"Andrea:Frasso")==0) return 1; else return 0;

}



