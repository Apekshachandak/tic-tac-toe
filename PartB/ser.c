#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include<fcntl.h>
#define PORT 8080
#define BUFFER_SIZE 1024
#define CHUNK_SIZE 4

struct packet
{
    int seq_num;
    int total_chunks;
    char data[CHUNK_SIZE];
};

void die(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void send_chunks(int server_fd, struct sockaddr_in *client_addr, socklen_t addr_len, const char *message)
{
    struct packet send_pkt;
    int total_chunks = (strlen(message) + CHUNK_SIZE - 1) / CHUNK_SIZE;
    int ack, seq_num;
    int ack_received[total_chunks]; 
    memset(ack_received, 0, sizeof(ack_received));

    int flags = fcntl(server_fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(server_fd, F_SETFL, flags);

    while (1) {
        int resend = 0; 

        for (seq_num=0;seq_num<total_chunks;seq_num++) {
            if (ack_received[seq_num] == 0) { 
                resend = 1;
                send_pkt.seq_num = seq_num;
                send_pkt.total_chunks = total_chunks;
                strncpy(send_pkt.data, message + seq_num * CHUNK_SIZE, CHUNK_SIZE);
                send_pkt.data[CHUNK_SIZE] = '\0'; 

                sendto(server_fd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)client_addr, addr_len);
                printf("Sent chunk %d\n", seq_num);

                fd_set readfds;
                FD_ZERO(&readfds);
                FD_SET(server_fd, &readfds);

                struct timeval timer;
                timer.tv_sec = 0;
                timer.tv_usec = 100000; 

                int n = select(server_fd + 1, &readfds, NULL, NULL, &timer);

                if (n > 0 && FD_ISSET(server_fd, &readfds)) {
                    recvfrom(server_fd, &ack, sizeof(ack), 0, (struct sockaddr *)client_addr, &addr_len);
                    printf("Ack received for chunk %d\n", ack);
                    ack_received[ack] = 1; 
                } else {
                    printf("No ACK received for chunk %d, resending...\n", seq_num);
                }


            }
        }

        
        if (resend == 0) {
            send_pkt.seq_num = total_chunks;
            send_pkt.total_chunks = total_chunks;
            strcpy(send_pkt.data, "End"); 
            sendto(server_fd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)client_addr, addr_len);
            printf("Sent termination packet\n");
            break; 
        }
    }
    printf("Chunks sent:%d\n",total_chunks);

}
void receive_chunks(int server_fd, struct sockaddr_in *client_addr, socklen_t addr_len)
{
    struct packet recv_pkt;
    char *chunks[100];
    int bytes_received, total_chunks = 0;
    int ack;
    char message[BUFFER_SIZE] = {0};
    int ack_received[100] = {0}; 

    while (1)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        struct timeval timer;
        timer.tv_sec = 10;      
        timer.tv_usec = 0;

        int n = select(server_fd + 1, &readfds, NULL, NULL, &timer);

        if (n < 0)
        {
            die("select error");
        }
        else if (n == 0)
        {
            continue; 
        }
        else{
        bytes_received = recvfrom(server_fd, &recv_pkt, sizeof(recv_pkt), 0, (struct sockaddr *)client_addr, &addr_len);
        if (bytes_received < 0)
        {
            die("Failed to receive packet");
        }

        if (recv_pkt.seq_num == 0)
        {
            total_chunks = recv_pkt.total_chunks;
        }
        else if (strcmp(recv_pkt.data, "End") == 0)
        {
            printf("End of transmission received.\n");
            break; 
        }

        if (ack_received[recv_pkt.seq_num] == 1)
        {
            continue; 
        }

        chunks[recv_pkt.seq_num] = strdup(recv_pkt.data);

        // if (rand() % 3 != 0) 
        // {
            
            ack = recv_pkt.seq_num;
            sendto(server_fd, &ack, sizeof(ack), 0, (struct sockaddr *)client_addr, addr_len);
            printf("Ack sent for chunk %d\n", ack);
            ack_received[ack] = 1; 
        // }
        // else
        // {
        //     printf("Acknowledgment lost for chunk %d\n", recv_pkt.seq_num);
        // }
        }
    }
    for(int i=0;i<total_chunks;i++)
    {
        if (chunks[i])
        {
            strncat(message, chunks[i], CHUNK_SIZE); 
            free(chunks[i]); 
        }
    }

    printf("Received message: %s\n", message);
}

int main()
{
    int server_fd;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        die("Socket creation failed");
    }

    memset(&client_addr, 0, sizeof(client_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(PORT);

    if (bind(server_fd, (const struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        die("Bind failed");
    }

    printf("Server is ready...\n");

    while (1)
    {
        printf("Waiting for client's message...\n");
        receive_chunks(server_fd, &client_addr, addr_len);

        char message[BUFFER_SIZE];
        printf("Enter message to send to client: ");
        fgets(message, BUFFER_SIZE, stdin);
        send_chunks(server_fd, &client_addr, addr_len, message);
    }

    close(server_fd);
    return 0;
}