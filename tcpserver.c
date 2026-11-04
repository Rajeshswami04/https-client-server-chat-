#include<stdio.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include <string.h>


int main(){
  int server_fd=socket(AF_INET,SOCK_STREAM,0);
  if(server_fd<0){printf("error in file descriptor"); return 0;}
   struct sockaddr_in server_addr;
   memset(&server_addr,0,sizeof(server_addr));
   server_addr.sin_family=AF_INET;
   server_addr.sin_port=htons(8080);
   inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr);
   int bd=bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr));
   if(bd<0){printf("error in binding"); return 0;}
   int lisres=listen(server_fd,2);
   if(lisres<0){printf("error in listening"); return 0;}
   struct sockaddr_in client_addr;
   socklen_t client_len=sizeof(client_addr);
   int client_fd =accept(server_fd,(struct sockaddr*)&client_addr,&client_len);
   if(client_fd<0){printf("error"); return 0; }
   char * msg="hello from server";
   ssize_t d=send(client_fd,msg,strlen(msg),0);
   if(d<0){printf("error"); return 0;}
   return 0;
}
