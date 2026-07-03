#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define TOTAL_THREADS 6
#define MAX_CONCURRENT_THREADS 3 // Max threads allowed in the resource pool at once

// Shared resources
int shared_counter = 0;
sem_t counting_sem;

// Thread function for demonstrating resource pooling
void* access_resource_pool(void* arg) {
    int thread_id = *(int*)arg;
    free(arg); // Free dynamic memory passed to thread

    printf("[Thread %d] Waiting to enter the resource pool...\n", thread_id);
    
    // 1. Decrement semaphore. If value is 0, thread blocks/waits.
    sem_wait(&counting_sem);

    // --- CRITICAL SECTION / RESOURCE POOL ---
    printf(" -> [Thread %d] ENTERED resource pool.\n", thread_id);
    
    // Safely increment counter (just to show safe modification)
    shared_counter++; 
    
    // Sleep to simulate active usage of the pool resource (e.g., database operation)
    usleep(800000); 

    printf(" <- [Thread %d] LEAVING resource pool.\n", thread_id);
    // ----------------------------------------

    // 2. Increment semaphore, notifying other waiting threads.
    sem_post(&counting_sem);

    return NULL;
}

int main() {
    pthread_t threads[TOTAL_THREADS];

    // 1. Initialize semaphore with a value of 3 (MAX_CONCURRENT_THREADS)
    // 0 means shared between threads of the same process.
    sem_init(&counting_sem, 0, MAX_CONCURRENT_THREADS);

    printf("Starting Simulation. Pool Limit: %d threads simultaneously.\n\n", MAX_CONCURRENT_THREADS);

    // 2. Create multiple threads
    for (int i = 0; i < TOTAL_THREADS; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        if (pthread_create(&threads[i], NULL, access_resource_pool, id) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    // 3. Wait for all threads to finish
    for (int i = 0; i < TOTAL_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // 4. Clean up
    sem_destroy(&counting_sem);

    printf("\nSimulation finished. Final shared counter increment value: %d\n", shared_counter);
    return 0;
}
