#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>

#define SHM_SIZE 1024 // Size of the shared memory segment
#define SEM_NAME "/shm_sync_semaphore"

int main() {
    key_t key = IPC_PRIVATE; // Unique key for shared memory segment
    int shmid;
    char *shared_memory;
    pid_t pid;
    sem_t *sem;

    // 1. Initialize a named POSIX semaphore to synchronize processes
    // Initial value is 0 so the reader must wait for the writer to signal
    sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }

    // 2. Use shmget() to create a shared memory segment
    shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget failed");
        sem_close(sem);
        sem_unlink(SEM_NAME);
        exit(1);
    }

    // 3. Create a child process using fork()
    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } 
    else if (pid == 0) {
        // ==========================================
        // CHILD PROCESS (Reader)
        // ==========================================
        
        // Use shmat() to attach memory segment to child's address space
        shared_memory = (char *)shmat(shmid, NULL, 0);
        if (shared_memory == (char *)-1) {
            perror("Child shmat failed");
            exit(1);
        }

        printf("[Child] Waiting for Parent to write data...\n");
        
        // Wait for the parent process to complete writing
        sem_wait(sem);

        // Read data written by the parent
        printf("[Child] Successfully read from shared memory: \"%s\"\n", shared_memory);

        // Clean up: Detach shared memory from child process
        shmdt(shared_memory);
        sem_close(sem);
        exit(0);
    } 
    else {
        // ==========================================
        // PARENT PROCESS (Writer)
        // ==========================================
        
        // Use shmat() to attach memory segment to parent's address space
        shared_memory = (char *)shmat(shmid, NULL, 0);
        if (shared_memory == (char *)-1) {
            perror("Parent shmat failed");
            exit(1);
        }

        printf("[Parent] Writing data into shared memory...\n");
        
        // Write data into the shared memory segment
        strncpy(shared_memory, "Hello from the Parent Process via Shared Memory!", SHM_SIZE);
        
        // Simulate a slight delay for realism
        sleep(1); 

        // Signal the child process that data is ready
        sem_post(sem);
        printf("[Parent] Data written and semaphore signaled.\n");

        // Wait for child process to finish reading and exit cleanly
        wait(NULL);

        // Clean up: Detach shared memory from parent process
        shmdt(shared_memory);

        // Clean up: Destroy the shared memory segment allocation
        shmctl(shmid, IPC_RMID, NULL);

        // Clean up: Close and unlink the named semaphore
        sem_close(sem);
        sem_unlink(SEM_NAME);

        printf("[Parent] Shared memory and semaphores cleaned up successfully.\n");
    }

    return 0;
}
