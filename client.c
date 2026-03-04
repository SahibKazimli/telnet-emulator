/**
 * Simple Hello World Socket Client
 */

#include <stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define TELNET_PORT 9999 // Should be port 23 but that may not be available on the machine.

/**
 * Creates a TCP socket for IpV4
 *
 * @return socket file descriptor (int)
 */
int create_socket() {
    int sockfd;
    //AF_INET means to use IPv4
    //SOCK_STREAM means to use TCP
    //0 is the protocol value for IP header (default)
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("[Client] socket creation failed");
        exit(1);
    }
    return sockfd;
}

/**
 * Creates the address struct to describe the IP address of the socket
 *
 * @return a struct with the address information
 */
struct sockaddr_in setup_socket_address(char server_host[]) {
    //Special struct for describing IP addresses for sockets
    struct sockaddr_in address;
    address.sin_family = AF_INET; //IPv4
    address.sin_port = htons(TELNET_PORT); //Port 23
    inet_pton(AF_INET, server_host, &address.sin_addr); //inet_pton takes the string address and converts it to binary and sets it in the struct field sin_addr
    return address;
}

/**
 * Connects the socket to the server specified in server_address
 *
 * @param sockfd the socket to connect with
 * @param server_address the address of the host to connect with
 * @return 0 if successful
 */
int connect_to_server(int sockfd, struct sockaddr_in server_address) {
    printf("[Client] Connecting to server \n");
    //Connects is a system call that connects the socket to the specified address and port
    int connect_res = connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address));
    if(connect_res < 0 ){
        printf("[Client] Connection failed\n");
        return 1;
    }
    printf("[Client] Connected successfully with server \n");
    return 0;
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
  printf("[Client] Sending msg: %s , of length: %zu \n", msg, strlen(msg));
  send(sockfd, msg, strlen(msg), 0);
  printf("[Client] Message sent successfully\n");
}

/**
 * Reads a message of size buffer_size into the given buffer from the given socket
 *
 * @param client_socket socket to read from
 * @param buffer the buffer to read the message into
 * @param buffer_size the size of the buffer
 */
void receive_socket_msg(int client_socket, char *buffer, int buffer_size) {
    printf("[Client] Reading from socket.. \n");
    //System call for reading from a file descriptor (a socket in this case)
    //On success, the number of bytes read is returned (zero indicates end of file)
    int read_res = read(client_socket, buffer, buffer_size);
    printf("[Client] Read %d bytes successfully \n",read_res);
    printf("[Client] Read the message: %s \n", buffer);
}

/**
 * Program entrypoint, implements a simple socket client
 *
 * 1. Creates a TCP socket
 * 2. Opens up a connection to the server
 * 3. Performs a hello-world exchange with the server
 * 4. Exit.
 *
 * @return
 */
int main() {
    printf("Telnet Client Starting\n");

    //1. Create & configure socket
    char *server_host = "127.0.0.1";
    int client_socket = create_socket();
    printf("[Client] Configuring server to connect to, address: %s, port: %d\n", server_host, TELNET_PORT);
    struct sockaddr_in server_addr = setup_socket_address(server_host);

    //2. Connect to server
    connect_to_server(client_socket, server_addr);

    //3. Send message to server
    char *hello = "Hello from Enköping/Client";
    socket_send_msg(client_socket, hello);

    //4. Receive response from server
    char receive_buffer[1024] = {0};
    receive_socket_msg(client_socket, receive_buffer, 1024);

    //5. Close and Exit
    printf("[Client] Closing the socket and exiting\n");
    close(client_socket);
    return 0;
}