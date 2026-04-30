#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>

#define PORT 50551
#define BUFFER_SIZE 4096

char logged_user[100] = "";
char session_token[100] = "";

// Security globals
int request_count = 0;
time_t last_time = 0;
int login_attempts = 0;

// Simple hashing
void hash_password(char *input, char *output) {
    int i;
    for (i = 0; input[i]; i++) {
        output[i] = input[i] + 3;
    }
    output[i] = '\0';
}

// Generate token
void generate_token(char *token) {
    sprintf(token, "TKN%d", rand() % 100000);
}

// Logging
void write_log(char *ip, int port, char *user, char *command, char *result) {
    FILE *fp = fopen("server_IT24101551.log", "a");

    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str)-1] = '\0';

    fprintf(fp, "%s | %s:%d | PID:%d | USER:%s | CMD:%s | RESULT:%s\n",
            time_str, ip, port, getpid(), user, command, result);

    fclose(fp);
}

// Register user
void register_user(char *user, char *pass, char *response) {

    // Username validation
    for (int i = 0; user[i]; i++) {
        if (!((user[i] >= 'a' && user[i] <= 'z') ||
              (user[i] >= 'A' && user[i] <= 'Z') ||
              (user[i] >= '0' && user[i] <= '9'))) {
            sprintf(response, "ERR 400 SID:1015 Invalid username\n");
            return;
        }
    }

    FILE *fp = fopen("users.txt", "a+");
    char line[200], u[100], p[100];

    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%s %s", u, p);
        if (strcmp(u, user) == 0) {
            sprintf(response, "ERR 400 SID:1015 User exists\n");
            fclose(fp);
            return;
        }
    }

    char hashed[100];
    hash_password(pass, hashed);
    fprintf(fp, "%s %s\n", user, hashed);
    fclose(fp);

    sprintf(response, "OK 200 SID:1015 Registered\n");
}

// Login user
void login_user(char *user, char *pass, char *response) {

    // Brute force protection
    if (login_attempts >= 3) {
        sprintf(response, "ERR 403 SID:1015 Account locked\n");
        return;
    }

    FILE *fp = fopen("users.txt", "r");
    char line[200], u[100], p[100];

    char hashed[100];
    hash_password(pass, hashed);

    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%s %s", u, p);
        if (strcmp(u, user) == 0 && strcmp(p, hashed) == 0) {
            strcpy(logged_user, user);
            generate_token(session_token);
            login_attempts = 0;

            sprintf(response, "OK 200 SID:1015 TOKEN:%s Login successful\n", session_token);
            fclose(fp);
            return;
        }
    }

    fclose(fp);
    login_attempts++;
    sprintf(response, "ERR 401 SID:1015 Invalid login\n");
}

// Handle client
void handle_client(int client_socket, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    int bytes;

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);

    while ((bytes = recv(client_socket, buffer, sizeof(buffer)-1, 0)) > 0) {
        buffer[bytes] = '\0';

        char command[20] = "", user[100] = "", pass[100] = "";
        char response[BUFFER_SIZE];

        // Payload size check
        if (bytes > 4096) {
            sprintf(response, "ERR 413 SID:1015 Payload too large\n");
            send(client_socket, response, strlen(response), 0);
            continue;
        }

        // Rate limiting
        time_t now = time(NULL);
        if (now == last_time) {
            request_count++;
        } else {
            request_count = 1;
            last_time = now;
        }

        if (request_count > 5) {
            sprintf(response, "ERR 429 SID:1015 Too many requests\n");
            send(client_socket, response, strlen(response), 0);
            continue;
        }

        // Correct parsing
        char *payload = strchr(buffer, '\n');
        if (payload != NULL) {
            payload++;
            sscanf(payload, "%s %s %s", command, user, pass);
        } else {
            strcpy(command, "UNKNOWN");
        }

        if (strcmp(command, "REGISTER") == 0) {
            register_user(user, pass, response);
            write_log(client_ip, client_port, user, "REGISTER", response);
        }
        else if (strcmp(command, "LOGIN") == 0) {
            login_user(user, pass, response);
            write_log(client_ip, client_port, user, "LOGIN", response);
        }
        else if (strcmp(command, "LOGOUT") == 0) {
            strcpy(logged_user, "");
            strcpy(session_token, "");
            sprintf(response, "OK 200 SID:1015 Logged out\n");
            write_log(client_ip, client_port, "none", "LOGOUT", response);
        }
        else {
            sprintf(response, "ERR 400 SID:1015 Unknown command\n");
            write_log(client_ip, client_port, "none", "UNKNOWN", response);
        }

        send(client_socket, response, strlen(response), 0);
    }

    close(client_socket);
}

// Main server
int main() {
    int server_fd, client_socket;
    struct sockaddr_in address, client_addr;
    socklen_t client_len = sizeof(client_addr);

    srand(time(0));

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);

    printf("Server running on port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        if (fork() == 0) {
            close(server_fd);
            handle_client(client_socket, client_addr);
            exit(0);
        } else {
            close(client_socket);
        }

        while (waitpid(-1, NULL, WNOHANG) > 0);
    }

    return 0;
}
