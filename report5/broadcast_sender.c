#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define bIP "255.255.255.255" 

int main(int argc, char *argv[]) {
    int sock;                       
    struct sockaddr_in bAddr;      
    unsigned short bPort;            
    char *buf;                    
    int broadcastPermission;         
    unsigned int bufLen;            

    // 명령행 인자 확인
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <포트번호> <보낼 문자열>\n", argv[0]);
        exit(1);
    }

    // 포트 번호와 전송할 메시지를 입력받음
    bPort = atoi(argv[1]);       
    buf = argv[2];               

    // UDP 소켓 생성
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    // 브로드캐스트 권한 설정
    broadcastPermission = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&broadcastPermission, sizeof(broadcastPermission)) < 0) {
        perror("setsockopt() failed");
        exit(1);
    }

    // 브로드캐스트 주소 설정
    memset(&bAddr, 0, sizeof(bAddr));  
    bAddr.sin_family = AF_INET;         
    bAddr.sin_addr.s_addr = inet_addr(bIP); 
    bAddr.sin_port = htons(bPort);    

    bufLen = strlen(buf);  // 메시지 길이 계산

    // 메시지 전송
    if (sendto(sock, buf, bufLen, 0, (struct sockaddr *)&bAddr, sizeof(bAddr)) != bufLen) {
        perror("전송된 바이트 수 틀림");
        exit(1);
    }

    // 소켓 종료
    close(sock);
    return 0;
}
