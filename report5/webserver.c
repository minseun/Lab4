#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>        /* close, read, write */
#include <sys/types.h>     /* system call types */

#define PORT 3490      /* default port */
#define QLEN 10        /* request queue size */
#define BUF_SIZE 1024

void *handle_clnt(int sockfd);
void send_err(int sockfd);
void send_msg(int sockfd);

int main(int argc, char *argv[]) {
    int sockfd, new_fd;                       /* listen socket, new connection socket */
    struct sockaddr_in server_addr;           /* server's address structure */
    struct sockaddr_in client_addr;           /* client's address structure */
    int alen;                                 /* length of address */
    fd_set readfds, activefds;                /* the set of read descriptors */
    int i, maxfd = 0;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind() failed");
        exit(1);
    }

    if (listen(sockfd, QLEN) < 0) {
        fprintf(stderr, "listen failed\n");
        exit(1);
    }

    alen = sizeof(client_addr);

    /* Initialize the set of active sockets. */
    FD_ZERO(&activefds);
    FD_SET(sockfd, &activefds);
    maxfd = sockfd;

    fprintf(stderr, "Server up and running.\n");

    /* Main server loop - accept and handle requests */
    while (1) {
        printf("SERVER: Waiting for contact..., %d\n", maxfd);
        readfds = activefds;

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        /* Service all the sockets with input pending. */
        for (i = 0; i <= maxfd; i++) {
            if (FD_ISSET(i, &readfds)) {
                if (i == sockfd) { /* New connection */
                    if ((new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &alen)) < 0) {
                        fprintf(stderr, "accept failed\n");
                        exit(1);
                    }
                    FD_SET(new_fd, &activefds);
                    if (new_fd > maxfd) maxfd = new_fd;
                } else { /* Handle client */
                    printf("handle clnt\n");
                    handle_clnt(i);
                    close(i);
                    FD_CLR(i, &activefds);
                }
            }
        }
    }
    close(sockfd);
}

void *handle_clnt(int client_sock) {
    int str_len = 0;
    int recv = 0;
    char msg[BUF_SIZE];
    char method[10];

    if ((str_len = read(client_sock, &msg[recv], BUF_SIZE)) == -1) {
        printf("read() error!\n");
        exit(1);
    }
    recv += str_len;

    strcpy(method, strtok(msg, " "));
    if (strcmp(method, "GET") != 0)
        send_err(client_sock);
    else
        send_msg(client_sock);

    return NULL;
}

void send_err(int client_sock) {
    char protocol[] = "HTTP/1.1 400 Bad Request\r\n";
    char server[] = "Server:Netscape-Enterprise/6.0\r\n";
    char contenttype[] = "Content-Type:text/html\r\n";
    char html[] = "<html><head>BAD Connection</head><body><H1>Bad Request</H1></body></html>\r\n";
    char end[] = "\r\n";

    printf("send err\n");
    write(client_sock, protocol, strlen(protocol));
    write(client_sock, server, strlen(server));
    write(client_sock, contenttype, strlen(contenttype));
    write(client_sock, end, strlen(end));
    write(client_sock, html, strlen(html));
    fflush(fdopen(client_sock, "w"));
}

void send_msg(int client_sock) {
    char protocol[] = "HTTP/1.1 200 OK\r\n";
    char server[] = "Server:Netscape-Enterprise/6.0\r\n";
    char contentlength[] = "Content-Length: 72\r\n";
    char contenttype[] = "Content-Type:text/html\r\n";
    char html[] = "<html><head>Hello World</head><body><H1>Hello World</H1></body></html>\r\n";
    char end[] = "\r\n";

    printf("send msg, len=%d\n", (int)strlen(html));
    write(client_sock, protocol, strlen(protocol));
    write(client_sock, server, strlen(server));
    write(client_sock, contentlength, strlen(contentlength));
    write(client_sock, contenttype, strlen(contenttype));
    write(client_sock, end, strlen(end));
    write(client_sock, html, strlen(html));
    fflush(fdopen(client_sock, "w"));
}
