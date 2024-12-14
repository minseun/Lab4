#include <stdio.h>      
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <stdlib.h>    
#include <string.h>     
#include <unistd.h>   

#define MAXRECVSTRING 255 

int main(int argc, char *argv[]) {
    int sock;                       
    struct sockaddr_in mAddr;       
    char *mIP;                     
    unsigned int mPort;             
    char buf[MAXRECVSTRING + 1];   
    int bufLen;                    
    struct ip_mreq mReq;           

    /* 명령행 인자 확인 */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Multicast IP> <Port>\n", argv[0]);
        exit(1);
    }
    mIP = argv[1];
    mPort = atoi(argv[2]);

    /* 소켓 생성 */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("소켓 생성 실패");
        exit(1);
    }

    /* 멀티캐스트 주소 구조체 설정 */
    memset(&mAddr, 0, sizeof(mAddr));
    mAddr.sin_family = AF_INET;
    mAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    mAddr.sin_port = htons(mPort);

    /* 소켓 바인딩 */
    if (bind(sock, (struct sockaddr *)&mAddr, sizeof(mAddr)) < 0) {
        perror("bind() 실패");
        exit(1);
    }

    /* 멀티캐스트 그룹 가입 */
    mReq.imr_multiaddr.s_addr = inet_addr(mIP); 
    mReq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&mReq, sizeof(mReq)) < 0) {
        perror("멀티캐스트 그룹 가입 실패");
        exit(1);
    }

    /* 멀티캐스트 메시지 수신 */
    memset(buf, 0, MAXRECVSTRING);
    if ((bufLen = recvfrom(sock, buf, MAXRECVSTRING, 0, NULL, 0)) < 0) {
        perror("수신 실패");
        exit(1);
    }

    printf("Received: %s\n", buf); 
    /* 소켓 종료 */
    close(sock);
    return 0;
}
