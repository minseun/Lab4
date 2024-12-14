#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#define PORT 3490
#define MAXDATASIZE 100
#define TIMEOUT 5 // 초 단위로 타임아웃 시간 설정

int main(int argc, char *argv[]) {
    int csock, numbytes;
    char buf[MAXDATASIZE];
    struct sockaddr_in serv_addr;
    int len;
    fd_set read_fds;
    struct timeval tv;
    int retval;

    if (argc != 3) {
        fprintf(stderr, "Usage : tcp_client <SERVER IP> <ECHO STRING>\n");
        exit(1);
    }

    // 소켓 생성
    if ((csock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // 서버 주소 설정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(PORT);

    // 서버와 연결
    if (connect(csock, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        close(csock);
        exit(1);
    }

    // 서버로 보낼 메시지 준비
    memset(buf, 0, MAXDATASIZE);
    strcpy(buf, argv[2]);
    len = strlen(buf);

    // 메시지 전송
    if (send(csock, buf, len, 0) != len) {
        fprintf(stderr, "send failed...\n");
        close(csock);
        exit(1);
    }

    // select 설정
    FD_ZERO(&read_fds); // 읽을 파일 디스크립터 초기화
    FD_SET(csock, &read_fds); // 소켓 파일 디스크립터 추가

    tv.tv_sec = TIMEOUT;  // 타임아웃 시간 설정
    tv.tv_usec = 0;

    // select를 사용하여 읽을 데이터가 있는지 확인
    retval = select(csock + 1, &read_fds, NULL, NULL, &tv);

    if (retval == -1) {
        perror("select()");
        close(csock);
        exit(1);
    } else if (retval == 0) {
        printf("Timeout occurred! No data received in %d seconds.\n", TIMEOUT);
        close(csock);
        exit(1);
    } else {
        // 데이터가 준비되면 읽기
        if (FD_ISSET(csock, &read_fds)) {
            memset(buf, 0, MAXDATASIZE);
            if ((numbytes = recv(csock, buf, MAXDATASIZE, 0)) == -1) {
                perror("recv");
                close(csock);
                exit(1);
            }
            printf("Received: %s\n", buf);
        }
    }

    // 소켓 닫기
    close(csock);
    return 0;
}
