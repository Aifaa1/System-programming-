#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h> // For future multi-threading, not used in this single-threaded example

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_ACCOUNTS 10

// Simple Account Structure
typedef struct {
    int account_id;
    double balance;
} Account;

Account accounts[MAX_ACCOUNTS];
int num_accounts = 0;

// Initialize some dummy accounts
void init_accounts() {
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        accounts[i].account_id = 1000 + i;
        accounts[i].balance = 1000.00; // Starting balance
        num_accounts++;
    }
    printf("Initialized %d accounts.\n", num_accounts);
}

// Find an account by ID
Account* find_account(int account_id) {
    for (int i = 0; i < num_accounts; i++) {
        if (accounts[i].account_id == account_id) {
            return &accounts[i];
        }
    }
    return NULL; // Not found
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};
    int valread;

    printf("Client connected.\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        valread = read(client_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            printf("Client disconnected or read error.\n");
            break;
        }

        printf("Received from client: %s\n", buffer);

        // Simple protocol parsing: COMMAND ACCOUNT_ID [AMOUNT/DEST_ACCOUNT_ID]
        char command[50];
        int account_id, dest_account_id;
        double amount;

        if (sscanf(buffer, "%s %d %lf %d", command, &account_id, &amount, &dest_account_id) >= 2) {
            Account* acc = find_account(account_id);

            if (strcmp(command, "BALANCE") == 0) {
                if (acc) {
                    sprintf(response, "BALANCE_OK %.2f", acc->balance);
                } else {
                    sprintf(response, "ERROR Account not found.");
                }
            } else if (strcmp(command, "DEPOSIT") == 0) {
                if (acc && amount > 0) {
                    acc->balance += amount;
                    sprintf(response, "DEPOSIT_OK %.2f", acc->balance);
                } else {
                    sprintf(response, "ERROR Invalid deposit amount or account.");
                }
            } else if (strcmp(command, "WITHDRAW") == 0) {
                if (acc && amount > 0) {
                    if (acc->balance >= amount) {
                        acc->balance -= amount;
                        sprintf(response, "WITHDRAW_OK %.2f", acc->balance);
                    } else {
                        sprintf(response, "ERROR Insufficient funds.");
                    }
                } else {
                    sprintf(response, "ERROR Invalid withdrawal amount or account.");
                }
            } else if (strcmp(command, "TRANSFER") == 0) {
                // Expected format: TRANSFER SOURCE_ACC_ID AMOUNT DEST_ACC_ID
                // Re-parsing to ensure correct order for transfer
                sscanf(buffer, "%s %d %lf %d", command, &account_id, &amount, &dest_account_id);

                Account* source_acc = find_account(account_id);
                Account* dest_acc = find_account(dest_account_id);

                if (source_acc && dest_acc && amount > 0) {
                    if (source_acc->balance >= amount) {
                        source_acc->balance -= amount;
                        dest_acc->balance += amount;
                        sprintf(response, "TRANSFER_OK From %d (%.2f) to %d (%.2f)",
                                source_acc->account_id, source_acc->balance,
                                dest_acc->account_id, dest_acc->balance);
                    } else {
                        sprintf(response, "ERROR Insufficient funds for transfer.");
                    }
                } else {
                    sprintf(response, "ERROR Invalid transfer details.");
                }
            } else {
                sprintf(response, "ERROR Unknown command.");
            }
        } else {
            sprintf(response, "ERROR Invalid command format.");
        }

        send(client_socket, response, strlen(response), 0);
        printf("Sent to client: %s\n", response);
    }

    close(client_socket);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    init_accounts();

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces
    address.sin_port = htons(PORT);

    // Bind the socket to the specified IP and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        // Accept a new connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        handle_client(new_socket); // Handles one client at a time
    }

    close(server_fd);
    return 0;
}

