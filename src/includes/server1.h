#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "utils.h"
#include "tcp_mechanism.h"
#include "stack.h" 
#include <time.h>
#include <sys/select.h>

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define BUFFER_LIMIT 1500
#define EVER ;;
#define random_max 10000
#define MAX_CONN 2
#define LEGAL_PATH_REGEX "^FILE:(.+)\\/([^\\/]+)$"
#define DEFAULT_SEGMENT_SIZE 1500
#define BIT_OFFSET 6
#define DEFAULT_PORT 1234
#define DEFAULT_WINDOW_SIZE 1
#define DEFAULT_TIMEOUT 10000
#define DEFAULT_RTT 1500

int create_udp_server(int port);
int handle_syn(int sockfd, struct sockaddr_in *client_addr, socklen_t client_addr_len);
int send_file(int sockfd, struct sockaddr_in *client_addr, socklen_t client_addr_len, char *file_name);
int end_connection(int sockfd, struct sockaddr_in *client_addr, socklen_t client_addr_len);
int main(int argc, char *argv[]) ;