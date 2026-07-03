#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5  // Fixed size of the circular buffer
#define TOTAL_ITEMS 10 // Total items to produce/consume

int buffer[BUFFER_SIZE];
int in = 0;  // Index where the producer inserts items
int out = 0; // Index where the consumer removes items

// Semaphores
sem_t mutex; // For mutual exclusion (binary)
sem_t empty; // Counts empty buffer slots (initialized to BUFFER_SIZE)
sem_t full;  // Counts filled buffer slots (initialized to 0)

// Producer thread function
void* producer(void* arg) {
    for (int i = 0; i < TOTAL_ITEMS; i++) {
        int item = rand() % 100; // Generate a random item

        // 1. Wait for an empty slot
        sem_wait(&empty);
        // 2. Lock the critical section
        sem_wait(&mutex);

        // --- CRITICAL SECTION: Insert item ---
        buffer[in] = item;
        printf("[Producer] Produced Item %d at slot %d\n", item, in);
        in = (in + 1) % BUFFER_SIZE; // Move pointer in a circular manner
        // -------------------------------------

        // 3. Unlock the critical section
        sem_post(&mutex);
        // 4. Signal that a slot is full
        sem_post(&full);

        // Simulate varying production speed
        usleep(300000); // Sleep for 300ms
    }
    return NULL;
}

// Consumer thread function
void* consumer(void* arg) {
    for (int i = 0; i < TOTAL_ITEMS; i++) {
        // 1. Wait for a full slot
        sem_wait(&full);
        // 2. Lock the critical section
        sem_wait(&mutex);

        // --- CRITICAL SECTION: Remove item ---
        int item = buffer[out];
        printf("[Consumer] Consumed Item %d from slot %d\n", item, out);
        out = (out + 1) % BUFFER_SIZE; // Move pointer in a circular manner
        // -------------------------------------

        // 3. Unlock the critical section
        sem_post(&mutex);
        // 4. Signal that a slot is empty
        sem_post(&empty);

        // Simulate varying consumption speed (making it slower to see buffer filling up)
        usleep(600000); // Sleep for 600ms
    }
    return NULL;
}

int main() {
    pthread_t prod_thread, cons_thread;

    // Initialize semaphores
    sem_init(&mutex, 0, 1);           // binary mutex
    sem_init(&empty, 0, BUFFER_SIZE); // initially all slots are empty
    sem_init(&full, 0, 0);            // initially no slots are full

    // Create producer and consumer threads
    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);

    // Wait for threads to finish
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    // Clean up resources
    sem_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    printf("Simulation completed successfully.\n");
    return 0;
}
