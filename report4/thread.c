#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_THREADS 5
#define BUF_SIZE 512

sigset_t sigset;

typedef struct {
    char msg[BUF_SIZE];
    int wr_check;
    pthread_mutex_t mutex;
    pthread_cond_t read;
} buffer;

int thread_flag[MAX_THREADS] = {0};

buffer bb = {"no message", 0,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_COND_INITIALIZER};

void input_msg() {
    int status, signal, i;

    while (1) {
        sigwait(&sigset, &signal);
        if (signal == SIGTSTP) {
            pthread_mutex_lock(&bb.mutex);  // 메시지 입력 전 뮤텍스 잠금

            printf("\ninput: ");
            fgets(bb.msg, BUF_SIZE, stdin);
            bb.wr_check = 1;

            for (i = 1; i < MAX_THREADS; i++) {
                thread_flag[i] = 1; 
            }

            // 모든 클라이언트 쓰레드에게 broadcast
            pthread_cond_broadcast(&bb.read);  // 조건 변수로 모든 클라이언트에게 메시지 방송

            pthread_mutex_unlock(&bb.mutex);  // 메시지 입력 후 뮤텍스 잠금 해제
        }
    }
}

void print_msg(void *arg) {
    int id = *(int *)arg;

    while (1) {
        pthread_mutex_lock(&bb.mutex);  // 메시지 출력 전 뮤텍스 잠금

        while (!thread_flag[id]) {
            pthread_cond_wait(&bb.read, &bb.mutex);  // 조건 변수를 사용해 메시지가 준비될 때까지 대기
        }

        if (bb.wr_check) {  // 메시지가 입력된 경우
            printf("Thread %d: %s", id, bb.msg);
            thread_flag[id] = 0;
        }

        pthread_mutex_unlock(&bb.mutex);  // 메시지 출력 후 뮤텍스 잠금 해제
        sleep(1);
    }
}

void *signal_thread(void *arg) {
    input_msg();
    return NULL;
}

void *msg_thread(void *arg) {
    print_msg(arg);
    return NULL;
}

int main() {
    int i, status;
    pthread_t tid[MAX_THREADS];
    int thread_ids[MAX_THREADS];

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGTSTP);
    status = pthread_sigmask(SIG_BLOCK, &sigset, NULL);
    if (status != 0) {
        fprintf(stderr, "Set signal mask failed\n");
        exit(1);
    }

    status = pthread_create(&tid[0], NULL, signal_thread, NULL);
    if (status != 0) {
        perror("Create signal thread");
        exit(1);
    }

    for (i = 1; i < MAX_THREADS; i++) {
        thread_ids[i] = i;
        status = pthread_create(&tid[i], NULL, msg_thread, &thread_ids[i]);
        if (status != 0) {
            perror("Create message thread");
            exit(1);
        }
    }

    for (i = 0; i < MAX_THREADS; i++) {
        pthread_join(tid[i], NULL);
    }

    return 0;
}
