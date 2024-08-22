#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024

void log_message(const char *message) {
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    printf("[%s] %s\n", timestamp, message);
    fflush(stdout);
}

void handle_put(int sock) {
    char filename[256];
    printf("Enter filename to upload: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0;
    
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error opening file\n");
        return;
    }
if (file == NULL) {
    perror("Error opening file");
    return;
}
printf("File opened for reading: %s\n", filename);

fseek(file, 0, SEEK_END);
long file_size = ftell(file);
fseek(file, 0, SEEK_SET);
printf("File size: %ld bytes\n", file_size);
    send(sock, "PUT", 3, 0);
    send(sock, filename, strlen(filename), 0);
    
    printf("Sending file: %s\n", filename);
    
    char buffer[BUFFER_SIZE];
    int bytes_read;
    int total_bytes = 0;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (send(sock, buffer, bytes_read, 0) < 0) {
            perror("Error sending file data");
            fclose(file);
            return;
        }
        total_bytes += bytes_read;
        printf("Sent %d bytes, total: %d\n", bytes_read, total_bytes);
    }
    
    fclose(file);
    
    printf("File sending completed. Total bytes sent: %d\n", total_bytes);
    
    // Wait for server response
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Server response: %s\n", buffer);
    } else if (bytes_received == 0) {
        printf("Server closed the connection\n");
    } else {
        perror("Error receiving server response");
    }
    
    printf("PUT operation completed\n");
}

void handle_get(int sock) {
    char filename[256];
    printf("Enter filename to download: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0;
    
    send(sock, "GET", 3, 0);
    send(sock, filename, strlen(filename), 0);
    
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Error creating file\n");
        return;
    }
    
    char buffer[BUFFER_SIZE];
    int bytes_received;
    int total_bytes = 0;
    while (1) {
        bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Error receiving file data or connection closed\n");
            break;
        }
        
        if (bytes_received >= 3 && memcmp(buffer, "EOF", 3) == 0) {
            printf("Received EOF marker\n");
            break;
        }
        
        fwrite(buffer, 1, bytes_received, file);
        total_bytes += bytes_received;
        printf("Received %d bytes, total: %d\n", bytes_received, total_bytes);
    }
    
    fclose(file);
    printf("File download completed. Total bytes received: %d\n", total_bytes);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP address> <port>\n", argv[0]);
        exit(1);
    }
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }
    
    char username[256], password[256];
    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;
    
    printf("Enter password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;
    
    log_message("Sending username and password");
    if (send(sock, username, strlen(username), 0) < 0) {
        perror("Failed to send username");
        close(sock);
        exit(1);
    }
    usleep(100000);  // Sleep for 100ms
    if (send(sock, password, strlen(password), 0) < 0) {
        perror("Failed to send password");
        close(sock);
        exit(1);
    }

    log_message("Waiting for server response");
    
    // Set socket to non-blocking mode
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    fd_set readfds;
    struct timeval tv;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    tv.tv_sec = 5;  // 5 seconds timeout
    tv.tv_usec = 0;

    int select_result = select(sock + 1, &readfds, NULL, NULL, &tv);
    if (select_result == -1) {
        perror("Select error");
        close(sock);
        exit(1);
    } else if (select_result == 0) {
        log_message("Timeout waiting for server response");
        close(sock);
        exit(1);
    }

    int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0) {
        if (errno != EWOULDBLOCK) {
            perror("Recv failed");
            close(sock);
            exit(1);
        }
    } else if (bytes_received == 0) {
        log_message("Server closed connection");
        close(sock);
        exit(1);
    } else {
        buffer[bytes_received] = '\0';
        log_message("Received response from server");
        printf("Server response: %s\n", buffer);
    }

    // Set socket back to blocking mode
    fcntl(sock, F_SETFL, flags);
    
    if (strcmp(buffer, "OK") == 0) {
        printf("Authentication successful\n");
        
        while (1) {
            printf("Enter command (PUT/GET/EXIT): ");
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            
            if (strcmp(buffer, "EXIT") == 0) break;
            
            if (strcmp(buffer, "PUT") == 0) {
                handle_put(sock);
            } else if (strcmp(buffer, "GET") == 0) {
                handle_get(sock);
            } else {
                printf("Unknown command\n");
            }
            
            printf("\n");  // Add a newline for better readability
        }
    } else {
        printf("Authentication failed\n");
    }
    
    close(sock);
    return 0;
}
