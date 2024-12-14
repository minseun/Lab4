#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

void *join_thread(void *arg) {
    pthread_exit(arg);
}

int main(int argc, char *argv[]) {
    pthread_t tid;
    intptr_t arg, status;
    void *result;

    if (argc < 2) {
        fprintf(stderr, "Usage: jointhread <number>\n");
        exit(1);
    }

    arg = (intptr_t)atoi(argv[1]);

    status = pthread_create(&tid, NULL, join_thread, (void *)arg);
    if (status != 0) {
        fprintf(stderr, "Create thread: %ld\n", (long int)status);  
        exit(1);
    }

    status = pthread_join(tid, &result);
    if (status != 0) {
        fprintf(stderr, "Join thread: %ld\n", (long int)status);  
        exit(1);
    }

    return (int)(intptr_t)result;
}
