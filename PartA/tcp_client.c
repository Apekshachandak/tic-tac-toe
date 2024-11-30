#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // convert IPv4 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    int player = 0;
    while (1)
    {
        // printf("Enter message to send to server: ");
        // fgets(message, BUFFER_SIZE, stdin);

    
        // send(sock, message, strlen(message), 0);
        // printf("Message sent to server\n");

        // if (strncmp(message, "Exit", 4) == 0)
        // {
        //     printf("Exiting client.\n");
        //     break;
        // }

        // memset(buffer, 0, BUFFER_SIZE); // Clear buffer before receiving
        // read(sock, buffer, BUFFER_SIZE);
        // printf("Message from server: %s\n", buffer);

        memset(buffer, 0, BUFFER_SIZE);
        // if(read(sock, buffer, BUFFER_SIZE) == 0)
        // {
        //     printf("Server disconnected\n");
        //     break;
        // }
        ssize_t bytes_rec = read(sock, buffer, BUFFER_SIZE);
        if (bytes_rec < 0)
        {
            perror("read");
            break;
        }
        buffer[bytes_rec] = '\0';

        if (strncmp(buffer, "Player", 6) == 0)
        {
            // printf("%s\n", buffer);
            printf("Enter your move: ");
            fgets(message, BUFFER_SIZE, stdin);
            player = buffer[7] - '0';

            send(sock, message, strlen(message), 0);
            printf("Move sent to server\n");
        }

        else if (strncmp(buffer, "Game Over", 9) == 0)
        {
            // printf("%s\n", buffer);
            // printf("Player %d\n", player);
            if (strncmp(buffer, "Game Over, It's a draw", 23) == 0)
            {
                printf("It's a draw\n");
            }
            else{
             char winning_player = buffer[18]; 
             
            if (winning_player == '1')
            {
                printf("Player 1 wins!\n");
            }
            else if (winning_player == '2') 
            {
                printf("Player 2 wins!\n");
            }
            }
           
            printf("Do you want to play again? (yes/no):");
            fgets(message, BUFFER_SIZE, stdin);
            message[strcspn(message, "\n")] = 0; 

            send(sock, message, strlen(message), 0);

        }
        else
        {
            printf("%s\n", buffer);

            send(sock, " ", 1, 0);
        }


        // printf("Enter your move: ");
        // fgets(message, BUFFER_SIZE, stdin);

        // send(sock, message, strlen(message), 0);
        // printf("Move sent to server\n");

        // memset(buffer, 0, BUFFER_SIZE); // Clear buffer before receiving
        // ssize_t bytes_rec2 = read(sock, buffer, BUFFER_SIZE);
        // if (bytes_rec2 < 0)
        // {
        //     perror("read");
        //     break;
        // }
        // buffer[bytes_rec2] = '\0';

        // printf("%s", buffer);
    }

    close(sock);
    return 0;
}
