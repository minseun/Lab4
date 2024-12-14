#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXRECVSTRING 255  // 수신할 최대 버퍼 크기

int main(int argc, char *argv[]) {
    int sock;                      // 소켓 디스크립터
    struct sockaddr_in bAddr;      // 수신할 주소 구조체
    unsigned int bPort;            // 수신할 포트
    char buf[MAXRECVSTRING + 1];   // 수신한 메시지를 담을 버퍼
    int bufLen;                    // 수신된 메시지 길이

    // 명령행 인자 확인
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Broadcast Port>\n", argv[0]);
        exit(1);
    }

    // 포트 번호 설정
    bPort = atoi(argv[1]);

    // UDP 소켓 생성
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    // 주소 구조체 초기화
    memset(&bAddr, 0, sizeof(bAddr));
    bAddr.sin_family = AF_INET;                 // IPv4
    bAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // 모든 인터페이스에서 수신
    bAddr.sin_port = htons(bPort);              // 입력받은 포트로 설정

    // 소켓에 주소 바인딩
    if (bind(sock, (struct sockaddr *) &bAddr, sizeof(bAddr)) < 0) {
        perror("bind() failed");
        exit(1);
    }

    // 메시지 수신
    if ((bufLen = recvfrom(sock, buf, MAXRECVSTRING, 0, NULL, 0)) < 0) {
        perror("recvfrom() failed");
        exit(1);
    }

    // 수신된 메시지 출력
    buf[bufLen] = '\0';  // 문자열 종료 문자 추가
    printf("Received: %s\n", buf);

    // 소켓 종료
    close(sock);

    return 0;
}
