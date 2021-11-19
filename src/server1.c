/**
 * @file server1.c
 * @author Lucas Dufour(contact@lucasdufour.fr)
 * @brief This program creates a tcp over udp server which listen to a given port.
 * @version 0.1
 * @date 2021-11-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

/**
 * @brief This function is used to SYN the server to the client. The client send a SYN. Then the server respond with a SYN ACK and gives the new port number.
 * Then the server waits for the client to send a ACK.
 * 
 * @param sockfd The socket file descriptor.
 * @param struct sockaddr_in client_addr .
 * 
 * 
 * @return int The new port number.
 */
int syn_server(int sockfd , struct sockaddr_in client_addr)
{
    int new_port = 0;
    char buffer[BUFFER_SIZE];
    socklen_t client_addr_len = sizeof(client_addr);
    int n;

    // Send SYN
    printf("Sending SYNACK\n");
    n = sendto(sockfd, "SYNACK", strlen("SYNACK"), 0, (struct sockaddr *)&client_addr, client_addr_len);
    if (n < 0)
    {
        perror("sendto");
        exit(1);
    }

    // Receive ACK
    printf("Receiving ACK\n");
    n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
    if (n < 0)
    {
        perror("recvfrom");
        exit(1);
    }
    buffer[n] = '\0';
    printf("Received: %s\n", buffer);

    return new_port;
}

/**
 * @brief This function is the main function. It create a UDP socket and bind it to the given port. Then it listen to the client.
 * 
 * @param argc The number of arguments.
 * @param argv The arguments.
 * 
 * @return int The exit code.
 */
int main(int argc, char *argv[]){
    int sockfd;
    int port;
    int new_port;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];
    int n;

    if(argc != 2){
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("bind");
        exit(EXIT_FAILURE);
    }

    client_addr_len = sizeof(client_addr);
    n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
    if(n < 0){
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }

    new_port = syn_server(sockfd, client_addr);

    server_addr.sin_port = htons(new_port);
    if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("bind");
        exit(EXIT_FAILURE);
    }
}