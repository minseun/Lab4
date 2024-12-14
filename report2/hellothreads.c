#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h> 

#define NUM_THREADS 3

void *hello_thread(void *arg) {
    intptr_t i = (intptr_t)arg;
    printf("Thread %ld: Hello, World!\n", i);
    return arg;
}

int main() {
    pthread_t tid[NUM_THREADS];
    int i, status;

    for (i = 0; i < NUM_THREADS; i++) {
        status = pthread_create(&tid[i], NULL, hello_thread, (void *)(intptr_t)i);
        if (status != 0) {
            fprintf(stderr, "Create thread %d: %d\n", i, status);
            exit(1);
        }
    }

    pthread_exit(NULL);
}
