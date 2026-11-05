#include <openssl/ssl.h>
#include <openssl/err.h>
#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include<sys/epoll.h>
#define MAX_EVENTS 100

struct custom_data {
    int fd;
    SSL *ssl;
};

int sendall(SSL *ssl, void *msg, int len)
{
    char *ptr = (char *)msg;
    int sent = 0;
    while (sent < len)
    {
        int temp = SSL_write(ssl, ptr + sent, len - sent);
        if (temp <= 0)
        {
            return -1;
        }
        sent += temp;
    }
    return 1;
}
int recvall(SSL *ssl, void *msg, int len)
{
    char *ptr = (char *)msg;
    int rcvd = 0;
    while (rcvd < len)
    {
        int temp = SSL_read(ssl, ptr + rcvd, len - rcvd);
        if (temp <= 0)
        {
            return -1;
        }
        rcvd += temp;
    }
   return 1;
}
int sendamsg(SSL *ssl, char *msg)
{
    uint32_t len = strlen(msg);
    uint32_t lenn = htonl(len);
    if (sendall(ssl, &lenn, sizeof(lenn)) < 0)
    {
        return -1;
    }
    if (sendall(ssl, msg, len) < 0)
        return -1;
    return 1;
}
int recvamsg(SSL *ssl, char *buffer)
{
    uint32_t len;
    if ((recvall(ssl, &len, sizeof(len))) < 0)
        return -1;
    uint32_t lenn = ntohl(len);
    if (lenn >= 2048)
        return -1;
    if ((recvall(ssl, buffer, lenn)) < 0)
        return -1;
    buffer[lenn] = '\0';
    return 1;
}
int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        printf("error in file descriptor");
        return 0;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    int bd = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bd < 0)
    {
        printf("error in binding");
        return 0;
    }
    int lisres = listen(server_fd, 2);
    if (lisres < 0)
    {
        printf("error in listening");
        return 0;
    }
    int epoll_fd=epoll_create1(0);
    
    struct custom_data *s_data = malloc(sizeof(struct custom_data));
    s_data->fd = server_fd;
    s_data->ssl = NULL;
    
    struct epoll_event event;
    event.events=EPOLLIN;
    event.data.ptr=s_data;
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,server_fd,&event);
    struct epoll_event events[MAX_EVENTS];
    while(1){
    int ready=epoll_wait(epoll_fd,events,MAX_EVENTS,-1);
    for(int i=0;i<ready;i++){
    struct custom_data *active_data = (struct custom_data *)events[i].data.ptr;
    if(active_data->fd==server_fd){
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0)
    {
        printf("error");
    }
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    if (ctx == NULL)
    {
        printf("error");
       exit(1);
    }
    SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM);
    SSL_CTX_check_private_key(ctx);
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_fd);
    int res = SSL_accept(ssl);
    if (res <= 0)
    {
        printf("error");
        exit(1);
    }
    struct custom_data *c_data = malloc(sizeof(struct custom_data));
    c_data->fd = client_fd;
    c_data->ssl = ssl;
    struct epoll_event client_event;
                client_event.events = EPOLLIN;
                client_event.data.ptr = c_data;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event);}
else{
    SSL *ssl = active_data->ssl;
    while (1)
    {
        char f[2048] = {0};
        int d = recvamsg(ssl, f);
        if (d < 0)
        {
            printf("error\n");
            break;
        }
        printf("client reqs:- %s", f);
        char buffer[2048];
        printf("> ");
        fgets(buffer, sizeof(buffer), stdin);
        if (strncmp(buffer, "quit", 4) == 0)
        {
            break;
        }
        d = sendamsg(ssl, buffer);
        if (d < 0)
        {
            printf("error\n");
            break;
        }
    }
  }
}
    }
    return 0;
}
