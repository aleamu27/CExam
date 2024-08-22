#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#define NUM_THREADS 10
#define BUFFER_SIZE 4096
#define DEFAULT_FILENAME "lese.pdf"

// Structure to hold thread data
typedef struct {
    int id;
    FILE *file;
    pthread_mutex_t *file_mutex;
} ThreadData;

void *worker_thread(void *arg);

int main(int argc, char *argv[]) {
    FILE *file;
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    pthread_mutex_t file_mutex;
    char *filename;

    // Use command-line argument or use default filename
    filename = (argc > 1) ? argv[1] : DEFAULT_FILENAME;
    printf("Using file: %s\n", filename);

    // Open the file
    file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error opening file '%s': %s\n", filename, strerror(errno));
        return 1;
    }
    printf("File opened successfully.\n");

    // Initialize the mutex for file access 
    if (pthread_mutex_init(&file_mutex, NULL) != 0) {
        perror("Mutex initialization failed");
        fclose(file);
        return 1;
    }

    // make threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].id = i;
        thread_data[i].file = file;
        thread_data[i].file_mutex = &file_mutex;
        if (pthread_create(&threads[i], NULL, worker_thread, &thread_data[i]) != 0) {
            perror("Thread creation failed");
            fclose(file);
            pthread_mutex_destroy(&file_mutex);
            return 1;
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Clean up
    pthread_mutex_destroy(&file_mutex);
    fclose(file);
    return 0;
}

void *worker_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    uint8_t buffer[BUFFER_SIZE];
    size_t bytes_read;
    uint32_t sum;
    int modulo;

    while (1) {
        // file access between threads
        pthread_mutex_lock(data->file_mutex);
        bytes_read = fread(buffer, 1, BUFFER_SIZE, data->file);
        pthread_mutex_unlock(data->file_mutex);

        // Exit if no more data to read
        if (bytes_read == 0) break;

        // Calculate sum of bytes and output readable characters
        sum = 0;
        for (size_t i = 0; i < bytes_read; i++) {
            sum += buffer[i];
            // Output readable characters
            if (isprint(buffer[i]) || buffer[i] == '\n') {
                putchar(buffer[i]);
            }
        }

        // Calculate modulo and sleep
        modulo = sum % 2000;
        usleep(modulo * 1000); // Convert milliseconds to microseconds

        // Print thread results
        printf("\nThread %d: Read %zu bytes, Sum: %u, Modulo: %d\n",
               data->id, bytes_read, sum, modulo);
        fflush(stdout); // flush buffer
    }

    return NULL;
}
