#include "includes/server1.h"

int segment_size = DEFAULT_SEGMENT_SIZE - BIT_OFFSET;
int window_size = DEFAULT_WINDOW_SIZE;

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

    printf("I have to send the file\n");
    // variables for the select
    fd_set readset;
    struct timeval tv;

    // Initialize the set.
    FD_ZERO(&readset);

    // Initialize time out struct.
    tv.tv_sec = 0;
    tv.tv_usec = DEFAULT_TIMEOUT;

    int timedout = 0; // Flag to check if the select timed out.



    // Theses stacks are used to store data in order to implement some TCP mechanisms 
    STACK acks = stack_init(); // Lasts received ACKs
    STACK segments = stack_init(); // Here we store the segments corresponding sequence numbers.

    acks = stack_push(acks, -1); // We push -1 to the stack, because we don't know the first ACK yet.
    acks->RTT = DEFAULT_RTT; // We initialize the RTT to 50.
    segments = stack_push(segments, -2); // Because of the calculation method, we push -2 to the stack. So the first segment will be sent with sequence number 0.


    int packet_number = -1; // Packet number should be -1 
    int acked = acks->element; // The last ACK received
    int last_segment_number = -1; // The last segment number sent

    file_name[strcspn(file_name, "\n")] = 0;
    // Open file
    file = get_file(file_name);
    if(file == NULL){
        return -1;
    }
    // Send file
    int flag_eof = 0;
    int flag_all_received = 0;

    //clocks
    clock_t begin =0;
    clock_t end = 0;
    do{
        char buffer[DEFAULT_SEGMENT_SIZE];
        char segmented_file[segment_size];

        if(acked >= 0 && acked == last_segment_number){
            flag_all_received = 1;
            break;
        }
        
        
        //  packet_number = next_seq_to_send(acks, segments);
        // Windows congestion. If the window is full, wait for the client to send an ACK.
        for (int i = 0; i < window_size && flag_all_received == 0; i++)
        {
            segments = stack_push(segments, packet_number);
            packet_number = next_seq_to_send(acks, segments, timedout);

            memset(buffer, 0, sizeof(buffer));
            memset(segmented_file, 0, sizeof(segmented_file));
            // Add the segment number to the header
            sprintf(buffer, "%06d", packet_number);
            // Read the file
            fseek(file, segment_size*packet_number, SEEK_SET);
            if(fread(segmented_file, sizeof(char), segment_size-7, file) < segment_size-7){
                flag_eof = 1;
                last_segment_number = packet_number;
            }

            strcat(buffer, segmented_file);
            //printf("\n \n Sending segment %06d\n", packet_number);

            if(timedout == 0){
                begin = clock();
            }
            timedout = 0;
            if(sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)client_addr, client_addr_len) < 0){
                    printf("sendto failed.\n");
                    handle_error("sendto failed");
                    return -1;
            }
        }

        //wait for ACK messages

        FD_SET(sockfd, &readset);
        printf("estimated timeout : %d us\n",estimate_timeout(acks->RTT));
        tv.tv_usec = estimate_timeout(acks->RTT);
        char ack_buffer[16];
        memset(ack_buffer, 0, sizeof(ack_buffer));

        if (select(sockfd+1, &readset, NULL, NULL, &tv)== 0){
            printf("TIMEOUT %ld us!\n", tv.tv_usec);
            timedout = 1;
        }
        else{
            //printf("Received ACK on packet %d\n", packet_number);
            if(recvfrom(sockfd, ack_buffer, sizeof(ack_buffer), 0, (struct sockaddr *)client_addr, &client_addr_len) < 0){
                printf("recvfrom failed.\n");
            }
            else{
                end = clock();
                if (compareString(ack_buffer, "ACK[0-9]{6}")){
                    acked = atoi(extract(ack_buffer, "ACK([0-9]{6})", 1));
                // printf("Received ACK %d\n", acked);
                    acks = stack_push(acks, acked);
                    acks->RTT= 1000000 * (double) (end - begin) / CLOCKS_PER_SEC; // RTT in microseconds
                    stack_print(acks);
                    if(acked < packet_number){
                        window_size = DEFAULT_WINDOW_SIZE;
                        //printf("Resetting window size to %d\n", window_size);
                    }
                    else{
                        window_size = window_size * 1;
                    // printf("Upscaling the window size to %d\n", window_size);
                    }
                }
            }
        }
     
        
    }while(flag_eof == 0 || flag_all_received == 0); // We send the file until we reach the end of the file AND until we receive an ACK for the last sent packet.
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