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
    int lisres = listen(server_fd, 10);
    if (lisres < 0)
    {
        printf("error in listening");
        return 0;
    }
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    if (ctx == NULL)
    {
        printf("error1");
        exit(1);
    }
    SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM);
    SSL_CTX_check_private_key(ctx);    
while(1){
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0)
    {
        printf("error2");
    }
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_fd);
    int res = SSL_accept(ssl);
    if (res <= 0)
    {
        printf("error3");
        continue;
    }
        char f[4096] = {0};
        int d = SSL_read(ssl, f,sizeof(f)-1);
        if (d < 0)
        {
            printf("error\n");
            break;
        }
        char buffer[]="HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n"
                "\r\n"
                "<html><body>"
                "<h1 style='color: blue;'>Hello from my secure C server!</h1>"
                "<p>I am speaking HTTP over TLS!</p>"
                "</body></html>\r\n";
        SSL_write(ssl, buffer,strlen(buffer));
       
    }
    return 0;
}
