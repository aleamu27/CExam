#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <errno.h>
#include "konte_h24_oppgave3.h"

#define BUFFER_SIZE 1024

char cwd[1024];

void log_message(const char *message) {
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    printf("[%s] %s\n", timestamp, message);
    fflush(stdout);
}

void handle_put(int client_socket) {
    char filename[256];
    char log_buffer[512];
    char buffer[BUFFER_SIZE];
    int bytes_received;

    bytes_received = recv(client_socket, filename, sizeof(filename) - 1, 0);
    if (bytes_received <= 0) {
        log_message("Error receiving filename for PUT");
        return;
    }
    filename[bytes_received] = '\0';
    
    snprintf(log_buffer, sizeof(log_buffer), "Received PUT request for file: %s", filename);
    log_message(log_buffer);
    
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", cwd, filename);
    snprintf(log_buffer, sizeof(log_buffer), "Attempting to create file: %s", full_path);
    log_message(log_buffer);

    FILE *file = fopen(full_path, "wb");
    if (file == NULL) {
        snprintf(log_buffer, sizeof(log_buffer), "Error opening file: %s. Errno: %d", full_path, errno);
        log_message(log_buffer);
        send(client_socket, "Error opening file", 18, 0);
        return;
    }
    
    log_message("File opened successfully, waiting for data");
    
    printf("Waiting for file data from client...\n");
    
    int total_bytes = 0;
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes_received, file);
        total_bytes += bytes_received;
        snprintf(log_buffer, sizeof(log_buffer), "Received %d bytes, total: %d", bytes_received, total_bytes);
        log_message(log_buffer);
    }
    
    if (bytes_received < 0) {
        snprintf(log_buffer, sizeof(log_buffer), "Error receiving file data. Errno: %d", errno);
        log_message(log_buffer);
    } else if (bytes_received == 0) {
        log_message("Client closed connection");
    }
    
    fclose(file);
    
    // Send response to client
    const char *response = "File uploaded successfully";
    if (send(client_socket, response, strlen(response), 0) < 0) {
        log_message("Error sending response to client");
    } else {
        log_message("Sent response to client");
    }
    
    snprintf(log_buffer, sizeof(log_buffer), "File upload completed. Total bytes: %d", total_bytes);
    log_message(log_buffer);
}

void handle_get(int client_socket) {
    char filename[256];
    char log_buffer[512];
    int bytes_received;

    bytes_received = recv(client_socket, filename, sizeof(filename) - 1, 0);
    if (bytes_received <= 0) {
        log_message("Error receiving filename for GET");
        return;
    }
    filename[bytes_received] = '\0';
    
    snprintf(log_buffer, sizeof(log_buffer), "Received GET request for file: %s", filename);
    log_message(log_buffer);
    
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", cwd, filename);
    snprintf(log_buffer, sizeof(log_buffer), "Attempting to open file: %s", full_path);
    log_message(log_buffer);

    FILE *file = fopen(full_path, "rb");
    if (file == NULL) {
        snprintf(log_buffer, sizeof(log_buffer), "Error opening file: %s. Errno: %d", full_path, errno);
        log_message(log_buffer);
        send(client_socket, "File not found", 14, 0);
        return;
    }
    
    log_message("File opened successfully, sending data");
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    snprintf(log_buffer, sizeof(log_buffer), "File size: %ld bytes", file_size);
    log_message(log_buffer);
    
    char buffer[BUFFER_SIZE];
    int bytes_read;
    int total_bytes = 0;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (send(client_socket, buffer, bytes_read, 0) < 0) {
            snprintf(log_buffer, sizeof(log_buffer), "Error sending file data. Errno: %d", errno);
            log_message(log_buffer);
            break;
        }
        total_bytes += bytes_read;
        snprintf(log_buffer, sizeof(log_buffer), "Sent %d bytes, total: %d", bytes_read, total_bytes);
        log_message(log_buffer);
    }
    
    fclose(file);
    snprintf(log_buffer, sizeof(log_buffer), "File download completed. Total bytes: %d", total_bytes);
    log_message(log_buffer);
    
    // Send end-of-file marker
    send(client_socket, "EOF", 3, 0);
    log_message("Sent EOF marker");
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    char username[256], password[256];
    char log_buffer[512];
    int bytes_received;
    
    log_message("New client connected");
    
    // Authentication
    bytes_received = recv(client_socket, username, sizeof(username) - 1, 0);
    if (bytes_received <= 0) {
        log_message("Error receiving username");
        close(client_socket);
        return;
    }
    username[bytes_received] = '\0';

    bytes_received = recv(client_socket, password, sizeof(password) - 1, 0);
    if (bytes_received <= 0) {
        log_message("Error receiving password");
        close(client_socket);
        return;
    }
    password[bytes_received] = '\0';
    
    snprintf(log_buffer, sizeof(log_buffer), "Authentication attempt - Username: %s", username);
    log_message(log_buffer);
    
    if (ftp_check_user(username, password)) {
        if (send(client_socket, "OK", 2, 0) < 0) {
            log_message("Error sending OK response");
            close(client_socket);
            return;
        }
        log_message("Authentication successful");
        
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_received <= 0) break;
            buffer[bytes_received] = '\0';
            
            snprintf(log_buffer, sizeof(log_buffer), "Received command: %s", buffer);
            log_message(log_buffer);
            
            if (strncmp(buffer, "PUT", 3) == 0) {
                handle_put(client_socket);
            } else if (strncmp(buffer, "GET", 3) == 0) {
                handle_get(client_socket);
            } else {
                send(client_socket, "Unknown command", 15, 0);
                log_message("Unknown command received");
            }
        }
    } else {
        if (send(client_socket, "Authentication failed", 21, 0) < 0) {
            log_message("Error sending authentication failure response");
        }
        log_message("Authentication failed");
    }
    
    close(client_socket);
    log_message("Client disconnected");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        exit(1);
    }
    printf("Current working directory: %s\n", cwd);
    
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        exit(1);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }
    
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        exit(1);
    }
    
    printf("Server listening on port %s\n", argv[1]);
    
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        
        handle_client(client_socket);
    }
    
    close(server_socket);
    return 0;
}
