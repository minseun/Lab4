#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <signal.h>
#include <errno.h>

#define MAXBUFSIZE 255   // 최대 버퍼 크기
#define SERV_PORT 5000   // 서버 포트 번호
#define SLEEP_TIME 5     // 대기 시간 (초)

void UseIdleTime();             // 대기 시간 동안 처리할 작업 함수
void SocketHandler(int signalType); // SIGIO 시그널 처리 핸들러 함수

int sock;  // 소켓 디스크립터

int main(int argc, char *argv[]) {
    struct sockaddr_in serv_addr;
    unsigned short serv_port = SERV_PORT;  // 서버 포트
    struct sigaction handler;  // 시그널 핸들러 구조체

    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    // 서버 주소 구조체 설정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 모든 IP 주소에서의 연결 허용
    serv_addr.sin_port = htons(serv_port);  // 포트 설정

    // 소켓 바인딩
    if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind() failed");
        exit(1);
    }

    // SIGIO 핸들러 설정
    handler.sa_handler = SocketHandler;
    if (sigfillset(&handler.sa_mask) < 0) {
        perror("sigfillset() failed");
        exit(1);
    }
    handler.sa_flags = 0;

    // SIGIO 시그널 핸들러 연결
    if (sigaction(SIGIO, &handler, 0) < 0) {
        perror("sigaction() failed for SIGIO");
        exit(1);
    }

    // 소켓의 소유권을 현재 프로세스로 설정
    if (fcntl(sock, F_SETOWN, getpid()) < 0) {
        perror("Unable to set process owner to us");
        exit(1);
    }

    // 소켓을 논블로킹 및 비동기 모드로 설정
    if (fcntl(sock, F_SETFL, O_NONBLOCK | FASYNC) < 0) {
        perror("Unable to put client sock into nonblocking/async mode");
        exit(1);
    }

    // 메인 루프 (SIGIO 이벤트가 발생하지 않으면 idle 상태 유지)
    for (;;) {
        UseIdleTime();
    }

    return 0;
}

// SIGIO 이벤트가 없으면 대기 시간 동안 실행되는 함수
void UseIdleTime() {
    printf("SIGIO 관련 이벤트가 없으면 이 함수가 실행됨.\n");
    printf("소켓 관련되지 않은 일 처리 가능함.\n\n");
    sleep(SLEEP_TIME);  // SLEEP_TIME만큼 대기
}

// SIGIO 시그널을 처리하는 함수
void SocketHandler(int signalType) {
    struct sockaddr_in clnt_addr;
    unsigned int clntLen;
    int recvMsgSize;
    char buf[MAXBUFSIZE];

    do {
        clntLen = sizeof(clnt_addr);

        // 클라이언트로부터 메시지 수신
        if ((recvMsgSize = recvfrom(sock, buf, MAXBUFSIZE, 0, 
                                    (struct sockaddr *)&clnt_addr, &clntLen)) < 0) {
            // 논블로킹 모드에서는 EWOULDBLOCK 오류를 처리
            if (errno != EWOULDBLOCK) {
                perror("recvfrom() failed");
                exit(1);
            }
        } else {
            printf("Handling client %s\n", inet_ntoa(clnt_addr.sin_addr));

            // 클라이언트에게 메시지 송신
            if (sendto(sock, buf, recvMsgSize, 0, 
                        (struct sockaddr *)&clnt_addr, sizeof(clnt_addr)) != recvMsgSize) {
                perror("sendto() failed");
                exit(1);
            }
        }
    } while (recvMsgSize >= 0);  // 메시지가 계속해서 수신될 때까지
}
