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
#include <time.h>

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define BUFFER_LIMIT 1500
#define EVER ;;
#define random_max 10000
#define MAX_CONN 2
#define LEGAL_PATH_REGEX "^FILE:(.+)\\/([^\\/]+)$"
#define DEFAULT_SEGMENT_SIZE 536
#define BIT_OFFSET 7
#define DEFAULT_PORT 1234
#define DEFAULT_WINDOW_SIZE 1
#define DEFAULT_TIMEOUT 300000

int segment_size = DEFAULT_SEGMENT_SIZE - BIT_OFFSET;
int window_size = DEFAULT_WINDOW_SIZE;
/**
 * @brief This function is used to create the UDP server, and bind it to the specified port.
 * @param port The port number to bind the server to.
 * 
 * @return -1 if failed, else the socket file descriptor.
 */
int create_udp_server(int port, int timeout) {
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

    if(timeout > 0) {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = timeout;
        printf("Setting timeout to %d\n", timeout);
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) < 0)
            handle_error("Error");
    }
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
        new_sockfd = create_udp_server(new_port, DEFAULT_TIMEOUT);
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
    file_name[strcspn(file_name, "\n")] = 0;
    // Open file
    file = get_file(file_name);
    if(file == NULL){
        return -1;
    }
    printf("File opened.\n");
    // Send file
    int acked = -1;
    int packet_number = 0;
    int flag_eof = 0;
    do{
        char buffer[DEFAULT_SEGMENT_SIZE];
        char segmented_file[segment_size];

        
            packet_number = acked +1;
        // Windows congestion. If the window is full, wait for the client to send an ACK.
        for (int i = 0; i < window_size; i++)
        {
            packet_number = packet_number + i;
            memset(buffer, 0, sizeof(buffer));
            memset(segmented_file, 0, sizeof(segmented_file));
            // Add the segment number to the header
            sprintf(buffer, "%06d", packet_number);
            // Read the file
            fseek(file, segment_size*packet_number, SEEK_SET);
            if(fread(segmented_file, sizeof(char), segment_size-7, file) < segment_size-7){
                flag_eof = 1;
            }

            strcat(buffer, segmented_file);
            //printf("Sending segment %06d\n", packet_number);
            
            if(sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)client_addr, client_addr_len) < 0){
                    printf("sendto failed.\n");
                    handle_error("sendto failed");
                    return -1;
            }
        }

        //wait for ACK messages
        char ack_buffer[16];
        memset(ack_buffer, 0, sizeof(ack_buffer));
        if(recvfrom(sockfd, ack_buffer, sizeof(ack_buffer), 0, (struct sockaddr *)client_addr, &client_addr_len) < 0){
            printf("TIMEOUT on packet %d!\n", packet_number);
            window_size = DEFAULT_WINDOW_SIZE;
            printf("Resetting window size to %d\n", window_size);
        }
        else{
            if (compareString(ack_buffer, "ACK[0-9]{6}")){
                acked = atoi(extract(ack_buffer, "ACK([0-9]{6})", 1));
                window_size = window_size*2;
            }
        }
    }while(flag_eof == 0 && acked <= packet_number);
    printf("File sent.\n");
    printf("Closing file.\n");

    fclose(file);

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
 * @brief This function returns the size of the file.
 * 
 * @param file The file.
 * 
 * @return The size of the file.
 */
int get_file_size(FILE *file){
    fseek(file, 0L, SEEK_END);
    int size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    return size;
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
    
    int sockfd = create_udp_server(port, 0);

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

    //calculate the execution time
    // to store the execution time of code
    double time_taken = 0.0;
 
    clock_t begin = clock();
 
    send_file(new_sockfd, &client_addr, client_addr_len, buffer);
 
    clock_t end = clock();
 
    // calculate elapsed time by finding difference (end - begin) and
    // dividing the difference by CLOCKS_PER_SEC to convert to seconds
    time_taken += (double)(end - begin) / CLOCKS_PER_SEC;
    
    //calculate the throughput
    int file_size = get_file_size(get_file(buffer));
    double throughput = file_size / time_taken;
    printf("Time taken: %f\n", time_taken);
    printf("Throughput: %E Byte/s\n", throughput);

    //end the session
    end_connection(sockfd, &client_addr, client_addr_len);

    return 0;
}