#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 3490
#define BUF_SIZE 128

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Server IP> <Message>\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_in server_addr;
    char buf[BUF_SIZE];

    // 소켓 생성
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    // 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // 서버와 연결
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect() failed");
        exit(1);
    }

    // 메시지 전송
    snprintf(buf, sizeof(buf), "%s", argv[2]);
    send(sockfd, buf, strlen(buf), 0);

    // 서버로부터 응답 받기
    int len = recv(sockfd, buf, BUF_SIZE, 0);
    if (len > 0) {
        buf[len] = '\0';
        printf("서버로부터 받은 메시지: %s\n", buf);
    } else {
        perror("recv() failed");
    }

    // 소켓 닫기
    close(sockfd);
    return 0;
}
