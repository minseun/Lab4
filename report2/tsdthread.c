#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define NUM_THREADS 3
pthread_key_t tsd_key;

typedef struct tsd_data {
    int thread;
    int key_data;
} tsd_data;

void *tsd_thread(void *arg) {
    tsd_data *data = (tsd_data *)arg;
    tsd_data *result;
    int status;

    status = pthread_setspecific(tsd_key, data);
    if (status != 0) {
        fprintf(stderr, "Set tsd key: %d", status);
        exit(1);
    }

    result = (tsd_data *)pthread_getspecific(tsd_key);
    printf("Thread %d: key_data is %d\n", result->thread, result->key_data);
    return arg;
}

int main(int argc, char *argv[]) {
    pthread_t tid[NUM_THREADS];
    int i;
    tsd_data data[NUM_THREADS];
    void *result;
    int status;

    if (argc < 4) {
        fprintf(stderr, "Usage: tsdthread number1 number2 number3\n");
        exit(1);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        
        data[i].thread = i;
        data[i].key_data = atoi(argv[i + 1]);
    }

    status = pthread_key_create(&tsd_key, NULL);
    if (status != 0) {
        fprintf(stderr, "Create key: %d", status);
        exit(1);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        status = pthread_create(&tid[i], NULL, tsd_thread, (void *)&data[i]);
        if (status != 0) {
            fprintf(stderr, "Create thread %d: %d", i, status);
            exit(1);
        }
    }

    for (i = 0; i < NUM_THREADS; i++) {
        status = pthread_join(tid[i], &result);
        if (status != 0) {
            fprintf(stderr, "Join thread %d: %d", i, status);
            exit(1);
        }
    }

    status = pthread_key_delete(tsd_key);
    if (status != 0)
        perror("Destroy key");

    pthread_exit(result);
}
