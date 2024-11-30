#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    int player = 0;
    socklen_t addr_len = sizeof(serv_addr);


    // Send initial "connect" message to the server
    strcpy(message, "connect");
    sendto(sock, message, strlen(message), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        ssize_t bytes_rec = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&serv_addr, &addr_len);
        if (bytes_rec < 0) {
            perror("recvfrom failed");
            break;
        }

        buffer[bytes_rec] = '\0';

        if (strncmp(buffer, "Player", 6) == 0) {
            printf("Enter your move: ");
            fgets(message, BUFFER_SIZE, stdin);
            player = buffer[7] - '0';
            sendto(sock, message, strlen(message), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            printf("Move sent to server\n");
        } else if (strncmp(buffer, "Game Over", 9) == 0) {
            char winning_player = buffer[18];
            if (winning_player == '1') {
                printf("Player 1 wins!\n");
            } else if (winning_player == '2') {
                printf("Player 2 wins!\n");
            } else if (strncmp(buffer, "Game Over, It's a draw", 23) == 0) {
                printf("It's a draw\n");
            }

            printf("Do you want to play again? (yes/no): ");
            fgets(message, BUFFER_SIZE, stdin);
            message[strcspn(message, "\n")] = 0;
            sendto(sock, message, strlen(message), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        } else {
            printf("%s\n", buffer);
        }
    }

    close(sock);
    return 0;
}
