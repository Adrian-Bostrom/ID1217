#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_THREADS 10
#define MAX_NAPPYTIME 500000000 // in nanoseconds (half a second)
#define MAX_EEPYTIME 5 // in seconds

sem_t *semaphore;
int TripsToBeDone = 10;

typedef struct car {
    int id;
    sem_t *condition;
    struct car *next;
    struct timespec *eepyTime;
    struct timespec *nappyTime;
} car;

typedef struct queue {
    struct car *first;
    struct car *last;
    int carCount;
} queue;

queue *northBound;
queue *southBound;

void enqueue(car *car, queue *q) {
    printf("car %d is being enqueued\n", car->id);
    if (q->first == NULL) {
        q->first = car;
        q->last = car;
    } else {
        q->last->next = car;
        q->last = car;
    }
    q->carCount++;
    //wait for the car to be allwed to drive
    sem_wait(car->condition);
    int ret = nanosleep(car->nappyTime, NULL);
    if (ret != 0) {
        perror("nanosleep failed in enqueue");
    }
    printf("car %d finished napping \n", car->id);
}

car *dequeue(queue *q) {
    if (q->first != NULL) {
        q->carCount--;
        car *cr = q->first;
        q->first = cr->next;
        cr->next = NULL;
        return cr;
    }
    return NULL;
}

void* threadFunc(void *arg) {
    int *index = (int *)arg;
    bool north = *index % 2;
    int trips = 0;
    car *threadCar = malloc(sizeof(car));
    sem_t car_sem;
    sem_init(&car_sem, 0, 0);
    threadCar->condition = &car_sem;

    // Allocate memory for eepyTime and nappyTime
    struct timespec *eepyTime = malloc(sizeof(struct timespec));
    struct timespec *nappyTime = malloc(sizeof(struct timespec));

    eepyTime->tv_nsec = 0;
    printf("thread %d has started\n", *index);
    nappyTime->tv_nsec = (rand() % MAX_NAPPYTIME) + 100000000; // time in nanoseconds (minimum 1/10 second)
    eepyTime->tv_sec = (rand() % MAX_EEPYTIME) + 1; // time in seconds (minimum 1 sec)
    nappyTime->tv_sec = 0;
    threadCar->eepyTime = eepyTime;
    threadCar->nappyTime = nappyTime;
    threadCar->id = *index;
    threadCar->next = NULL;

    while (trips != TripsToBeDone) {
        printf("car %d is being eepy from driving %s", *index, ((trips + north) % 2)? ("North\n"):("South\n"));
        if ((trips + north) % 2) enqueue(threadCar, northBound);
        if ((trips + north + 1) % 2) enqueue(threadCar, southBound);
        // comes back if it's done crossing the road
        int ret = nanosleep(eepyTime, NULL);
        if (ret == 0) {
        } else {
            perror("nanosleep failed in threadFunc");
        }
        trips++;
    }

    // Free allocated memory
    free(eepyTime);
    free(nappyTime);
    free(threadCar);
    return NULL;
}

int main(int argc, char *argv[]) {
    int numWorkers = (argc > 1) ? atoi(argv[1]) : NUM_THREADS;
    pthread_t threads[numWorkers];
    int thread_args[NUM_THREADS];
    northBound = calloc(1, sizeof(queue));
    southBound = calloc(1, sizeof(queue));

    for (int i = 0; i < NUM_THREADS; ++i) {
        // Set the thread argument to the current index
        thread_args[i] = i;
        // Create a new thread
        pthread_create(&threads[i], NULL, threadFunc, (void*)&thread_args[i]);
        printf("created thread %d\n", i);
    }
    bool north_driving = true;
    printf("all threads have been created\n");
    sleep(3);
    while (1) {
        if (north_driving) {
            car *next_car = dequeue(northBound);
            if (next_car != NULL) {
                printf("car %d has been dequeued\n", next_car->id);
                sem_post(next_car->condition);
                if (northBound->carCount == 0){
                    sleep(1);
                    north_driving = false;
                }
            } else {
                sleep(1);
            }
        } else {
            car *next_car = dequeue(southBound);
            if (next_car != NULL) {
                printf("car %d has been dequeued\n", next_car->id);
                sem_post(next_car->condition);
                if (southBound->carCount == 0){
                    sleep(1);
                    north_driving = true;
                }
            } else {
                sleep(1);
            }
        }
    }
}

/*
enqueue
wait(cars condition)
then when released its crossed,
increase trips and wait nappy time


main:
dequeues in order and waits the eepy time % to cross
then releases semaforo 

*/