# Networking


**XO game using TCP**

*Compilation*

Compile with `gcc tcp_ser.c -o tcpser` and `gcc tcp_client.c -o tcpclient`.

Run the server first and then connect two clients

1. The server creates a TCP socket using socket(), binds it to a specified port (8080) using bind(), and listens for incoming connections from players using listen().

2. The server accepts connections from two clients (players) using accept(). Each client is identified by their socket descriptor, which is used to send and receive messages throughout the game.

3. A 3x3 grid is represented as a 2D array. The server alternates turns between the two players, checking for valid moves and determining the game state (win, draw, or ongoing) using the checkWin() function.

4. The server sends prompts and game state updates to each player using send(), and receives their moves via recv(). Invalid inputs are handled with error messages, and the game board is displayed after each move using a formatted string.

5. After each game round, players are prompted to play again. The server handles their responses, allowing for game restarts or disconnections as needed.

**XO game using UDP**

*Compilation*

Compile with `gcc udp_ser.c -o udpser` and `gcc udp_client.c -o udpclient`.

1. The server creates a UDP socket using socket(), binds it to a specified port using bind(), allowing it to receive messages from clients.

2. The server uses recvfrom() to listen for incoming messages (player moves) from both clients. Each client sends their move as a datagram, which includes the player's identification (socket address).

3. A 3x3 grid is represented as a 2D array. The server alternates turns between the two players, checking for valid moves and determining the game state (win, draw, or ongoing) using the checkWin() function.

4. The server constructs and sends responses back to each client using sendto(). Invalid inputs are handled with error messages sent to the respective client, and the game board is displayed after each move through formatted strings sent as datagrams.

5. After each game round, players are prompted to play again. The server handles their responses and maintains the game state, allowing for game restarts or disconnections as needed.



**TCP using UDP**

*Compilation*

Compile with `gcc ser.c -o ser` and `gcc cli.c -o client`

1.  Messages are split into smaller chunks for transmission, allowing for efficient and reliable data handling.

2. The server and client implement an ACK system to confirm receipt of each chunk. If an ACK is not received within a specified timeout, the chunk is resent.

3. Both the server and client use non-blocking sockets and the select system call to handle incoming data and timeouts, ensuring smooth operation without freezing.

4. After all chunks are sent, a termination packet is sent to indicate the end of the transmission.

