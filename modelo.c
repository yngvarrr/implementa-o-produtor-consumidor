#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define PRODUCER_THREADS 3
#define CONSUMER_THREADS 3
#define BUFFER_SIZE 5

typedef struct VectorClock {
    int id;
    int timestamp[10];
} VectorClock;

VectorClock buffer[BUFFER_SIZE];
int head = 0;
int tail = 0;
int count = 0;

pthread_mutex_t mutex;
pthread_cond_t condFull;
pthread_cond_t condEmpty;

void *producer(void *arg) {
    int id = *(int*)arg;
    while (1) {
        pthread_mutex_lock(&mutex);

        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&condEmpty, &mutex);
        }

        VectorClock vc;
        vc.id = id;
        for (int i = 0; i < 10; i++) {
            vc.timestamp[i] = rand() % 100;
        }
        buffer[tail] = vc;
        tail = (tail + 1) % BUFFER_SIZE;
        count++;

        printf("Produtor %d produziu um relogio vetorial com o id %d e timestamp: [", id, vc.id);
        for (int i = 0; i < 10; i++) {
            printf("%d ", vc.timestamp[i]);
        }
        printf("]\n");

        pthread_cond_signal(&condFull);
        pthread_mutex_unlock(&mutex);

        sleep(rand() % 5);
    }
}

void *consumer(void *arg) {
    int id = *(int*)arg;
    while (1) {
        pthread_mutex_lock(&mutex);

        while (count == 0) {
            pthread_cond_wait(&condFull, &mutex);
        }

        VectorClock vc = buffer[head];
        head = (head + 1) % BUFFER_SIZE;
        count--;

        printf("Consumidor %d consumiu um relogio vetorial de id %d e timestamp: [", id, vc.id);
        for (int i = 0; i < 10; i++) {
            printf("%d ", vc.timestamp[i]);
        }
        printf("]\n");

        pthread_cond_signal(&condEmpty);
        pthread_mutex_unlock(&mutex);

        sleep(rand() % 5);
    }
}

int main() {
    srand(time(NULL));

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condFull, NULL);
    pthread_cond_init(&condEmpty, NULL);

    pthread_t producerThreads[PRODUCER_THREADS];
    pthread_t consumerThreads[CONSUMER_THREADS];

    int producerIds[PRODUCER_THREADS];
    int consumerIds[CONSUMER_THREADS];

    for (int i = 0; i < PRODUCER_THREADS; i++) {
        producerIds[i] = i;
        pthread_create(&producerThreads[i], NULL, producer, (void*)&producerIds[i]);
    }

    for (int i = 0; i < CONSUMER_THREADS; i++) {
        consumerIds[i] = i;
        pthread_create(&consumerThreads[i], NULL, consumer, (void*)&consumerIds[i]);
    }

    for (int i = 0; i < PRODUCER_THREADS; i++) {
        pthread_join(producerThreads[i], NULL);
    }

    for (int i = 0; i < CONSUMER_THREADS; i++) {
        pthread_join(consumerThreads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condFull);
    pthread_cond_destroy(&condEmpty);

    return 0;
}