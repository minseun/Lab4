#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 20
#define NUMITEMS 30

typedef struct {
    int item[BUFFER_SIZE];
    int totalitems;
    int in, out;

    pthread_mutex_t mutex;
    pthread_cond_t full;
    pthread_cond_t empty;

} buffer_t;

buffer_t bb = {
    {0}, 0, 0, 0,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_COND_INITIALIZER,
    PTHREAD_COND_INITIALIZER
};

volatile int running = 1;

int produce_item() {
    int item = (int)(100.0 * rand() / (RAND_MAX + 1.0));
    sleep((unsigned long)(5.0 * rand() / (RAND_MAX + 1.0)));
    printf("produce_item: item = %d\n", item);
    return item;
}

int insert_item(int item) {
    int status;

    status = pthread_mutex_lock(&bb.mutex);
    if (status != 0)
        return status;

    while (bb.totalitems >= BUFFER_SIZE && status == 0)
        status = pthread_cond_wait(&bb.empty, &bb.mutex);

    if (status != 0) {
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }

    bb.item[bb.in] = item;
    bb.in = (bb.in + 1) % BUFFER_SIZE;
    bb.totalitems++;

    if ((status = pthread_cond_signal(&bb.full))) {
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }
    return pthread_mutex_unlock(&bb.mutex);
}

int consume_item(int item) {
    sleep((unsigned long)(5.0 * rand() / (RAND_MAX + 1.0)));
    printf("\t\tconsume_item: item = %d\n", item);
    return 0;
}

int remove_item(int *temp) {
    int status;

    status = pthread_mutex_lock(&bb.mutex);
    if (status != 0)
        return status;

    while (bb.totalitems <= 0 && status == 0)
        status = pthread_cond_wait(&bb.full, &bb.mutex);

    if (status != 0) {
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }

    *temp = bb.item[bb.out];
    bb.out = (bb.out + 1) % BUFFER_SIZE;
    bb.totalitems--;

    if ((status = pthread_cond_signal(&bb.empty))) {
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }
    return pthread_mutex_unlock(&bb.mutex);
}

void *producer(void *arg) {
    int item;

    while (running) {
        item = produce_item();
        insert_item(item);
    }
    return NULL;
}

void *consumer(void *arg) {
    int item;

    while (running) {
        remove_item(&item);
        consume_item(item);
    }
    return NULL;
}

int main() {
    int status;
    pthread_t producer_tid, consumer_tid;

    status = pthread_create(&producer_tid, NULL, producer, NULL);
    if (status != 0)
        perror("Create producer thread");

    status = pthread_create(&consumer_tid, NULL, consumer, NULL);
    if (status != 0)
        perror("Create consumer thread");

    printf("Press ENTER to stop the program...\n");
    getchar();

    running = 0;

    pthread_join(producer_tid, NULL);
    pthread_join(consumer_tid, NULL);

    return 0;
}