/**
 * Simple Hello World Socket Server
 */

#include <stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>

#define TELNET_PORT 9999 // Should be port 23 but that may not be available on the machine.

/**
 * Creates a TCP socket for IpV4
 *
 * @return socket file descriptor (int)
 */
int create_socket() {
    int sockfd;
    int opt = 1;
    //AF_INET means to use IPv4
    //SOCK_STREAM means to use TCP
    //0 is the protocol value for IP header (default)
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket creation failed");
        exit(1);
    }

    return sockfd;
}

/**
 * Creates the address struct to describe the IP address of the socket
 *
 * @return a struct with the address information
 */
struct sockaddr_in setup_socket_address() {
    //Special struct for describing IP addresses for sockets
    struct sockaddr_in address;
    address.sin_family = AF_INET; //IPv4
    address.sin_addr.s_addr = INADDR_ANY; //Bind to all interfaces, can accept connection on all the IPs of the machine
    address.sin_port = htons(TELNET_PORT); //Port
    return address;
}

/**
 * Binds the socket to the given address
 *
 * @param sockfd the socket descriptor
 * @param sock_addr the IP address
 */
int bind_and_listen(int sockfd, struct sockaddr_in sock_addr){

    printf("[Server] Listening to port %d\n", TELNET_PORT);

    //Binds the socket to the address and port number specified in the address struct
    int bind_res = bind(sockfd, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
    if (bind_res < 0) {
        perror("[Server] bind failed");
        exit(1);
    }
    //Puts the socket in passive mode waiting for a client to open up a connection
    // The backlog, defines the maximum length to which the queue of pending connections for sockfd may grow.
    // If a connection request arrives when the queue is full, the client may receive an error
    // with an indication of ECONNREFUSED.
    int tcp_backlog = 3;
    int listen_res = listen(sockfd, tcp_backlog);
    if(listen_res < 0){
        perror("[Server] listen failed");
        exit(1);
    }
    return sockfd;
}

/**
 * Accepts incoming client connection on the given socket
 *
 * @param sockfd the server socket file descripto
 * @param sock_addr struct describing the IP address to listen for incoming connections on
 * @return a file descriptor for the socket of the accepted connection
 */
int accept_incoming_connections(int sockfd, struct sockaddr_in sock_addr) {
    printf("[Server] Waiting for incoming connections.. \n");

    int addrlen = sizeof(sock_addr);
    //Extracts the first connection request from the queue of pending connections for the listening socket.
    //Returns a file descriptor to the new socket. At this point, the connection is established between client and
    //server and they are ready to transfer data.
    int client_socket;
    if ((client_socket = accept(sockfd, (struct sockaddr *)&sock_addr,
                                (socklen_t*)&addrlen))<0)
    {
        perror("[Server] failure trying to accept an incoming client connection");
        exit(-1);
    }
    printf("[Server] Accepted client connection\n");
    return client_socket;
}

/**
 * Reads a message of size buffer_size into the given buffer from the given socket
 *
 * @param client_socket socket to read from
 * @param buffer the buffer to read the message into
 * @param buffer_size the size of the buffer
 */
void receive_socket_msg(int client_socket, char *buffer, int buffer_size) {
    printf("[Server] Reading from socket.. \n");
    //System call for reading from a file descriptor (a socket in this case)
    //On success, the number of bytes read is returned (zero indicates end of file)
    int read_res = read(client_socket, buffer, buffer_size);
    printf("[Server] Read %d bytes successfully \n",read_res);
    printf("[Server] Read the message: %s \n", buffer);
}

/**
 * Sends a message over the socket
 *
 * @param sockfd the socket
 * @param msg  the mesage to send
 */
void socket_send_msg(int sockfd, char msg[]) {
    //Send system call, sends a message over the the given socket
    //0 in the last argument is a default flag.
    printf("[Server] Sending msg: %s , of length: %zu \n", msg, strlen(msg));
    send(sockfd, msg, strlen(msg), 0);
    printf("[Server] Message sent successfully\n");
}



/**
 * Sends all bytes in the buffer over the socket
 * @param sockfd the socket to send on
 * @param buffer pointer to the data to send
 * @param length number of bytes to send
 * @return 0 on success, -1 on error
 */
int send_all(int sockfd, const void *buffer, int length) {
    int total_bytes_sent = 0;
    while (total_bytes_sent < length) {
        int bytes_sent = send(sockfd, (const char *)buffer + total_bytes_sent, length - total_bytes_sent, 0);
        if (bytes_sent <= 0) {
            perror("[Server] send failed");
            return -1;
        }
        total_bytes_sent += bytes_sent;
    }
    return 0;
}

/**
 * Sends a message using the length-prefixed protocol.
 * Different CPUs store multi-byte integers differently,
 * so we use htonl().
 * @param sockfd the socket
 * @param msg the message string to send
 * @param msg_len length of the message
 * @return 0 on success, -1 on error
 */
int send_length_prefixed(int sockfd, const char *msg, int msg_len) {
    // Convert length and send it
    uint32_t net_len = htonl(msg_len);
    if (send_all(sockfd, &net_len, sizeof(net_len)) < 0) {
        return -1;
    }
    printf("[Server] Sent length prefix: %d bytes\n", msg_len);

    // Send the data
    if (msg_len > 0) {
        if (send_all(sockfd, msg, msg_len) < 0) {
            return -1;
        }
        printf("[Server] Sent message: %.*s\n", msg_len, msg);
    }
    return 0;
}

/**
 * Reads length bytes from the socket into the buffer.
 * We loop, reading chunks until we have the expected amount.
 * @param sockfd the socket to read from
 * @param buffer the buffer to fill
 * @param length number of bytes to read
 * @return 0 on success, -1 on error or disconnect
 */
int read_exact(int sockfd, void *buffer, int length) {
    int total_bytes_read = 0;
    while (total_bytes_read < length) {
        int bytes_read = read(sockfd, (char *)buffer + total_bytes_read, length - total_bytes_read);
        if (bytes_read <= 0) {
            return -1;
        }
        total_bytes_read += bytes_read;
    }
    return 0;
}

/**
 * Program entrypoint, implements a simple socket server.
 *
 * 1. Create TCP socket
 * 2. In a loop: Listen for incoming connections and do a hello/world exchange before closing the connection.
 *
 * @return 0
 */
int main() {
    printf("Telnet Server Starting\n");

    //1. Create & configure socket
    int server_socket = create_socket();
    int client_socket;
    struct sockaddr_in sock_addr = setup_socket_address();

    //2. bind and listen socket to the given port
    server_socket = bind_and_listen(server_socket, sock_addr);

    //3. Handle incoming connections one by one sequentially
    // (note: a real server would handle client requests in new threads/processes
    // and manage many clients in parallel
    while(1) {
        //Wait for and accept incoming client connection (blocking)
        client_socket = accept_incoming_connections(server_socket, sock_addr);

        //Read message from client
        char receive_buffer[1024] = {0};
        int bytes_read = read(client_socket, receive_buffer, sizeof(receive_buffer));

        if (bytes_read <= 0) {
            // Disconnect client here
            printf("[Server] Client disconnected\n");
            break;
        }

        // convert length_buffer to integer
        int cmd_length = atoi(length_buffer);
        if (cmd_length <= 0) {
            // Handle errors
            printf("[Server] Invalid command length, skipping\n");
            continue;
        }

        // Allocate memory for command dynamically, since the messages obviously vary in length
        // +1 because we need to account for null terminator
        char *command = malloc(cmd_length + 1); 
        if (command == NULL) {
            perror("[Server] malloc failed");
            break;
        }
        memset(command, 0, cmd_length + 1);

        int total_bytes_read = 0;
        // Loop to read in all bytes
        while (total_bytes_read < cmd_length) {
            int bytes_read = read(client_socket, command + total_bytes_read, cmd_length - total_bytes_read);
            if (bytes_read <= 0) {
                // handle error
                printf("[Server] Error reading command\n");
                break;
            }
            total_bytes_read += bytes_read;
        }
        command[cmd_length] = '\0'; // I really want to make sure the null terminator is included
        printf("[Server] Received command: %s\n", command);

        // IDK what to do here right now
        free(command)

        //Send response to client
        char *hello = "Hello from Stockholm/Server";
        socket_send_msg(client_socket, hello);

        //Close connection (note a real server would typicalyl wait until client closes the connection
        printf("[Server] Closing the client connection and waiting for the next connection \n");
        close(client_socket);
    }

    return 0;
}