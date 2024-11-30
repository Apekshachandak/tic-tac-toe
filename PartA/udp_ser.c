#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void pretty_print_grid(char grid[3][3], char *buffer)
{
    sprintf(buffer, "\n %c | %c | %c \n---|---|---\n %c | %c | %c \n---|---|---\n %c | %c | %c \n\n",
            grid[0][0], grid[0][1], grid[0][2],
            grid[1][0], grid[1][1], grid[1][2],
            grid[2][0], grid[2][1], grid[2][2]);
}

int checkWin(char grid[3][3], char input)
{
    for (int i = 0; i < 3; i++)
    {
        if (grid[i][0] == input && grid[i][1] == input && grid[i][2] == input)
        {
            return 1;
        }
    }
    for (int i = 0; i < 3; i++)
    {
        if (grid[0][i] == input && grid[1][i] == input && grid[2][i] == input)
        {
            return 1;
        }
    }
    if (grid[0][0] == input && grid[1][1] == input && grid[2][2] == input)
    {
        return 1;
    }
    if (grid[0][2] == input && grid[1][1] == input && grid[2][0] == input)
    {
        return 1;
    }
    return 0;
}

int main()
{
    int server_fd;
    struct sockaddr_in server_address, client_addresses[2];
    socklen_t client_address_lens[2];
    char buffer[BUFFER_SIZE] = {0};
    client_address_lens[0] = sizeof(client_addresses[0]);
    client_address_lens[1] = sizeof(client_addresses[1]);

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("UDP Server: Listening on port %d...\n", PORT);
    for (int i = 0; i < 2; i++)
    {
        printf("Waiting for client %d...\n", i + 1);
        recvfrom(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addresses[i], &client_address_lens[i]);
        printf("Connection from: %s:%d\n", inet_ntoa(client_addresses[i].sin_addr), ntohs(client_addresses[i].sin_port));
    }

    while (1)
    {
        int turn = 0;
        int moves = 0;
        char grid[3][3] = {{' ', ' ', ' '}, {' ', ' ', ' '}, {' ', ' ', ' '}};
        int game_over = 0;

        while (!game_over)
        {
            memset(buffer, 0, BUFFER_SIZE);
            int player = turn + 1;
            sprintf(buffer, "Player %d, it is your turn\n", player);
            sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addresses[turn], client_address_lens[turn]);

            memset(buffer, 0, BUFFER_SIZE);
            ssize_t valread = recvfrom(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addresses[turn], &client_address_lens[turn]);
            if (valread < 0)
            {
                perror("read failed");
                break;
            }
            buffer[valread] = '\0';
            printf("Message from client %d: %s", player, buffer);

            if (buffer[0] < '1' || buffer[0] > '3' || buffer[2] < '1' || buffer[2] > '3' || buffer[1] != ' ')
            {
                const char *response = "Invalid input. Please enter a valid input.\n";
                sendto(server_fd, response, strlen(response), 0, (struct sockaddr *)&client_addresses[turn], client_address_lens[turn]);
                continue;
            }

            int i = buffer[0] - '1';
            int j = buffer[2] - '1';
            if (grid[i][j] != ' ')
            {
                const char *response = "Invalid input. Please enter a valid input.\n";
                sendto(server_fd, response, strlen(response), 0, (struct sockaddr *)&client_addresses[turn], client_address_lens[turn]);
                continue;
            }

            grid[i][j] = (turn == 0) ? 'X' : 'O';
            moves++;

            int win = 0;
            char player_char = (turn == 0) ? 'X' : 'O';
            win = checkWin(grid, player_char);
            if (win)
            {
                memset(buffer, 0, BUFFER_SIZE);
                sprintf(buffer, "Game Over, Player %d wins!\n", player);
                sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addresses[0], client_address_lens[0]);
                sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addresses[1], client_address_lens[1]);
                game_over = 1;
                break;
            }

            if (moves == 9)
            {
                const char *response = "Game Over, It's a draw!\n";
                sendto(server_fd, response, strlen(response), 0, (struct sockaddr *)&client_addresses[0], client_address_lens[0]);
                sendto(server_fd, response, strlen(response), 0, (struct sockaddr *)&client_addresses[1], client_address_lens[1]);
                game_over = 1;
                break;
            }

            memset(buffer, 0, BUFFER_SIZE);
            pretty_print_grid(grid, buffer);
            sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addresses[0], client_address_lens[0]);
            sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addresses[1], client_address_lens[1]);

            turn = 1 - turn;
        }

        char response1[BUFFER_SIZE] = {0};
        char response2[BUFFER_SIZE] = {0};
        ssize_t valread1 = recvfrom(server_fd, response1, BUFFER_SIZE, 0, (struct sockaddr *)&client_addresses[0], &client_address_lens[0]);
        ssize_t valread2 = recvfrom(server_fd, response2, BUFFER_SIZE, 0, (struct sockaddr *)&client_addresses[1], &client_address_lens[1]);
        if (valread1 < 0 || valread2 < 0)
        {
            perror("read failed");
            break;
        }

        response1[valread1] = '\0';
        response2[valread2] = '\0';

        if (strncmp(response1, "no", 2) == 0 || strncmp(response2, "no", 2) == 0)
        {
            const char *quit_msg = "One player does not want to play again. Closing game.\n";
            sendto(server_fd, quit_msg, strlen(quit_msg), 0, (struct sockaddr *)&client_addresses[0], client_address_lens[0]);
            sendto(server_fd, quit_msg, strlen(quit_msg), 0, (struct sockaddr *)&client_addresses[1], client_address_lens[1]);
            break;
        }
        else if (strncmp(response1, "yes", 3) == 0 && strncmp(response2, "yes", 3) == 0)
        {
            const char *restart_msg = "Restarting game...\n";
            sendto(server_fd, restart_msg, strlen(restart_msg), 0, (struct sockaddr *)&client_addresses[0], client_address_lens[0]);
            sendto(server_fd, restart_msg, strlen(restart_msg), 0, (struct sockaddr *)&client_addresses[1], client_address_lens[1]);
            continue;
        }
    }

    close(server_fd);
    return 0;
}
