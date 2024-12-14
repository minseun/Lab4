#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 5000      /* 서버 포트 */
#define MAXDATASIZE 100 /* 버퍼 크기 */

int main(int argc, char *argv[]) {
    int csock;             // 클라이언트 소켓
    char buf[MAXDATASIZE]; // 메시지를 저장할 버퍼
    struct sockaddr_in to_addr;   // 서버 주소
    struct sockaddr_in from_addr; // 서버로부터 응답을 받을 주소
    int from_len;           // 응답 길이
    int len;                // 메시지 길이
    int recv_len;           // 수신한 메시지 길이

    // 명령행 인자 확인
    if (argc != 3) {
        fprintf(stderr, "usage: udp_client <SERVER IP> <ECHO STRING>\n");
        exit(1);
    }

    // 클라이언트 소켓 생성
    if ((csock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // 서버 주소 설정
    memset(&to_addr, 0, sizeof(to_addr));
    to_addr.sin_family = AF_INET;
    to_addr.sin_addr.s_addr = inet_addr(argv[1]);  // 서버 IP 주소
    to_addr.sin_port = htons(PORT);                // 서버 포트 설정

    // 메시지 송신
    memset(buf, 0, MAXDATASIZE);
    strcpy(buf, argv[2]); // 사용자가 입력한 메시지를 buf에 저장
    len = strlen(buf);    // 메시지 길이

    // 서버로 메시지 전송
    if (sendto(csock, buf, len, 0, (struct sockaddr *)&to_addr, sizeof(to_addr)) != len) {
        fprintf(stderr, "send failed...\n");
        exit(1);
    }

    // 응답 수신 준비
    memset(buf, 0, MAXDATASIZE);  // 버퍼 초기화
    from_len = sizeof(from_addr);

    // 서버로부터 메시지 수신
    if ((recv_len = recvfrom(csock, buf, MAXDATASIZE, 0, (struct sockaddr *)&from_addr, &from_len)) == -1) {
        perror("recv");
        exit(1);
    }

    // 수신한 메시지 출력
    printf("Received: %s\n", buf);

    // 소켓 종료
    close(csock);
    return 0;
}
