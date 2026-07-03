#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 10
#define INCREMENTS_PER_THREAD 100000

// Shared resources
int counter = 0;
pthread_mutex_t lock;

// Thread function
void* increment_counter(void* arg) {
    int use_mutex = *((int*)arg);

    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        if (use_mutex) {
            // Protect the critical section
            pthread_mutex_lock(&lock);
            counter++;
            pthread_mutex_unlock(&lock);
        } else {
            // Unprotected shared resource (Race Condition)
            counter++;
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t threads[NUM_THREADS];
    int use_mutex = 1; // Change to 0 to see the race condition

    if (argc > 1) {
        use_mutex = atoi(argv[1]);
    }

    printf("Running WITH %s...\n", use_mutex ? "MUTEX LOCK" : "NO MUTEX (Race Condition)");

    // 1. Initialize the mutex
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Mutex initialization failed\n");
        return 1;
    }

    // 2. Create multiple threads
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, increment_counter, &use_mutex) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // 3. Expected vs Actual Results
    int expected_value = NUM_THREADS * INCREMENTS_PER_THREAD;
    printf("Expected Counter Value: %d\n", expected_value);
    printf("Actual Counter Value:   %d\n", counter);

    if (counter == expected_value) {
        printf("Success: Mutual exclusion worked flawlessly.\n");
    } else {
        printf("Data Race Detected: Value is incorrect due to overlapping execution.\n");
    }

    // 4. Clean up
    pthread_mutex_destroy(&lock);

    return 0;
}
