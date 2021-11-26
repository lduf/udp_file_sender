#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "includes/utils.h"
#include "includes/server1.h"

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define BUFFER_LIMIT 1500
#define EVER ;;
#define random_max 10000
#define MAX_CONN 2
#define LEGAL_PATH_REGEX "^FILE:(.+)\\/([^\\/]+)$"
#define SEGMENT_SIZE 536
#define DEFAULT_PORT 1234


/**
 * @brief This function is used to create the UDP server, and bind it to the specified port.
 * @param port The port number to bind the server to.
 * 
 * @return -1 if failed, else the socket file descriptor.
 */
int create_udp_server(int port) {
    int sockfd;
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        handle_error("socket creation failed");
    printf("Socket created : %d\n", sockfd);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    printf("Binding to port %d\n", port);
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        return -1;
        //handle_error("bind failed");

    printf("Binding successful\n");
    return sockfd;
}


/**
 * @brief This function is used to handle the SYN process. It's called when the server receives a SYN packet. It will send a SYNACK packet to the client. 
 * This SYNACK packet will contain the port number of the server with the given format: "SYN-ACK<port>". The port number must be between 1000 and 9999.
 * The function creates a new UDP listener and bind it to the new port number.
 * When the SYN-ACK packed had been sent, the server will wait for the client to send an ACK packet. It returns the new socket file descriptor.
 * 
 * @param sockfd The socket file descriptor.
 * @param client_addr The client's address.
 * @param client_addr_len The length of the client's address.
 * 
 * @return The new socket file descriptor.
 */
int handle_syn(int sockfd, struct sockaddr_in *client_addr, socklen_t client_addr_len) {
    int new_sockfd;
    char buffer[BUFFER_LIMIT];
    char *syn_ack = (char*)malloc(16 * sizeof(char));
    int new_port = 0;
    struct sockaddr_in new_servaddr;
    socklen_t new_servaddr_len;

    // Create new UDP listener
    do{
        new_port = random_int(1000, 9999);
        new_sockfd = create_udp_server(new_port);
    }while(new_sockfd == -1);
    

    // Not required
    new_servaddr_len = sizeof(new_servaddr);
    if (getsockname(new_sockfd, (struct sockaddr *)&new_servaddr, &new_servaddr_len) < 0)
        handle_error("getsockname failed");

    
    // Send SYNACK
    sprintf(syn_ack, "SYN-ACK%d", new_port);
    if (sendto(sockfd, syn_ack, strlen(syn_ack), 0, (struct sockaddr *)client_addr, client_addr_len) < 0)
        handle_error("sendto failed");

    // Receive ACK
    memset(buffer, 0, BUFFER_LIMIT);
    if (recvfrom(sockfd, buffer, BUFFER_LIMIT, 0, (struct sockaddr *)client_addr, &client_addr_len) < 0)
        handle_error("recvfrom failed");
    if(compareString(buffer, "ACK")){
        printf("Received ACK from client.\n");
    }
    else{
        printf("Received something else.\n");
        handle_error("Received something else after SYN-ACK");
    }

    return new_sockfd;
}

/**
 * @brief This function return the file from the given path.
 * 
 * @param path The path of the file.
 * 
 * @return The file.
 */
FILE* get_file(char* path){
    printf("Recherche du fichier au chemin suivant : %s\n", path);
    FILE* file = fopen(path, "r");
    if(file == NULL){
        printf("File not found.\n");
        return NULL;
    }
    return file;
}


/**
 * @brief This function is used to send the file to the client. It will send the file in segments of 536 bytes.
 * 
 * @param sockfd The socket file descriptor.
 * @param client_addr The client's address.
 * @param client_addr_len The length of the client's address.
 * @param file_name The name of the file to send.
 * 
 * @return 0 if successful, -1 if failed.
 */
int send_file(int sockfd, struct sockaddr_in *client_addr, socklen_t client_addr_len, char *file_name) {
    FILE *file;
    char buffer[SEGMENT_SIZE];
    
    file_name[strcspn(file_name, "\n")] = 0;
    // Open file
    file = get_file(file_name);
    if(file == NULL){
        return -1;
    }
    printf("File opened.\n");
    // Send file
    
    int i = 5;
    int packet_number = 0;
    int flag_eof = 0;
    do{
        i=6;
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%06d", packet_number);
        printf("New buffer, segment n°%c %c %c %c %c %c", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
        do{
            buffer[i] = fgetc(file);
           // printf("%c\n", buffer[i]);
            if(buffer[i] == EOF ){
                flag_eof = 1;
            }
            i++;
        }while(flag_eof == 0 && i-1 < SEGMENT_SIZE);
        printf("Sending segment %s\n", buffer);
        
        if(sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)client_addr, client_addr_len) < 0){
                printf("sendto failed.\n");
                return -1;
        }
        packet_number++;
    }while(flag_eof == 0);


    printf("File sent.\n");

    //end the session
    end_connection(sockfd, client_addr, client_addr_len);
    return 0;
}

/**
 * @brief This function is used to end the connection. It will send a FIN packet to the client.
 * 
 * @param sockfd The socket file descriptor.
 * @param client_addr The client's address.
 * @param client_addr_len The length of the client's address.
 * 
 * @return 0 if successful, -1 if failed.
 */
int end_connection(int sockfd, struct sockaddr_in *client_addr, socklen_t client_addr_len) {
    char *fin = (char*)malloc(16 * sizeof(char));
    sprintf(fin, "FIN");
    if (sendto(sockfd, fin, strlen(fin), 0, (struct sockaddr *)client_addr, client_addr_len) < 0)
        return -1;
    printf("Connection ended.\n");
    return 0;
}


/**
 * @brief This is the main function. It will create a UDP server to the given port and wait for the client to send a SYN packet. 
 * 
 * @param argc The number of arguments. If argc != 2, the default port would be 1234, else it will be the second argument.
 * @param argv The arguments. The second argument is the port number.
 * 
 * @return 0 if success, else -1.
 */
int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;

    if (argc != 2)
        printf("Usage: ./serverXXX <port>\n default port %d used\n", DEFAULT_PORT);
    else
        port = atoi(argv[1]);
    
    printf("Server is running on port %d\n", port);
    
    int sockfd = create_udp_server(port);

    // Wait for SYN
    printf("Waiting for client to send SYN...\n");
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_LIMIT];
    memset(buffer, 0, BUFFER_LIMIT);
    if (recvfrom(sockfd, buffer, BUFFER_LIMIT, 0, (struct sockaddr *)&client_addr, &client_addr_len) < 0)
        handle_error("recvfrom failed");

    printf("Received : %s\n", buffer);
    int new_sockfd = 0; 
    if(compareString(buffer, "SYN")){
        printf("Received SYN from client.\n");
        new_sockfd = handle_syn(sockfd, &client_addr, client_addr_len);
        printf("New socket: %d\n", new_sockfd);
    }

    // clear buffer
    memset(buffer, 0, BUFFER_LIMIT);
    // Send file given by the client
    if (recvfrom(new_sockfd, buffer, BUFFER_LIMIT, 0, (struct sockaddr *)&client_addr, &client_addr_len) < 0)
        handle_error("recvfrom failed");

    printf("Received file name : %s\n", buffer);
    send_file(new_sockfd, &client_addr, client_addr_len, buffer);

    return 0;
}