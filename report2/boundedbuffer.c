#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
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

buffer_t bb = { {0}, 0, 0, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER };

int produce_item() {
    int item = (int)(100.0 * rand() / (RAND_MAX + 1.0));
    sleep((unsigned long)(5.0 * rand() / (RAND_MAX + 1.0)));
    printf("produce_item: item=%d\n", item);
    return item;
}

int insert_item(int item) {
    int status = pthread_mutex_lock(&bb.mutex);
    if (status != 0) return status;

    while (bb.totalitems >= BUFFER_SIZE) {
        status = pthread_cond_wait(&bb.empty, &bb.mutex);
        if (status != 0) {
            pthread_mutex_unlock(&bb.mutex);
            return status;
        }
    }

    bb.item[bb.in] = item;
    bb.in = (bb.in + 1) % BUFFER_SIZE;
    bb.totalitems++;

    status = pthread_cond_signal(&bb.full);
    if (status != 0) {
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }

    return pthread_mutex_unlock(&bb.mutex);
}

void consume_item(int item) {
    sleep((unsigned long)(5.0 * rand() / (RAND_MAX + 1.0)));
    printf("\t\tconsume_item: item=%d\n", item);
}

int remove_item(int *temp) {
    int status = pthread_mutex_lock(&bb.mutex);
    if (status != 0) return status;

    while (bb.totalitems <= 0) {
        status = pthread_cond_wait(&bb.full, &bb.mutex);
        if (status != 0) {
            pthread_mutex_unlock(&bb.mutex);
            return status;
        }
    }

    *temp = bb.item[bb.out];
    bb.out = (bb.out + 1) % BUFFER_SIZE;
    bb.totalitems--;

    status = pthread_cond_signal(&bb.empty);
    if (status != 0) {
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }

    return pthread_mutex_unlock(&bb.mutex);
}

void *producer(void *arg) {
    int item;
    while (1) {
        item = produce_item();
        insert_item(item);
    }
}

void *consumer(void *arg) {
    int item;
    while (1) {
        remove_item(&item);
        consume_item(item);
    }
}

int main() {
    int status;
    pthread_t producer_tid, consumer_tid;

    status = pthread_create(&producer_tid, NULL, producer, NULL);
    if (status != 0) {
        perror("Create producer thread");
        exit(1);
    }

    status = pthread_create(&consumer_tid, NULL, consumer, NULL);
    if (status != 0) {
        perror("Create consumer thread");
        exit(1);
    }

    status = pthread_join(producer_tid, NULL);
    if (status != 0) {
        perror("Join producer thread");
        exit(1);
    }

    status = pthread_join(consumer_tid, NULL);
    if (status != 0) {
        perror("Join consumer thread");
        exit(1);
    }

    return 0;
}
