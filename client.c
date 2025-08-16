#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1" // Localhost
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char command_input[BUFFER_SIZE];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");
    printf("Available commands:\n");
    printf("  BALANCE <account_id>\n");
    printf("  DEPOSIT <account_id> <amount>\n");
    printf("  WITHDRAW <account_id> <amount>\n");
    printf("  TRANSFER <source_account_id> <amount> <destination_account_id>\n");
    printf("  EXIT\n");

    while (1) {
        printf("\nEnter command: ");
        fgets(command_input, BUFFER_SIZE, stdin);
        command_input[strcspn(command_input, "\n")] = 0; // Remove newline

        if (strcmp(command_input, "EXIT") == 0) {
            printf("Exiting client.\n");
            break;
        }

        send(sock, command_input, strlen(command_input), 0);
        printf("Command sent: %s\n", command_input);

        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(sock, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            printf("Server disconnected or read error.\n");
            break;
        }
        printf("Server response: %s\n", buffer);
    }

    close(sock);
    return 0;
}

