int create_udp_server(int port, int timeout);
int handle_syn(int sockfd, struct sockaddr_in *client_addr, socklen_t client_addr_len);
int send_file(int sockfd, struct sockaddr_in *client_addr, socklen_t client_addr_len, char *file_name);
int end_connection(int sockfd, struct sockaddr_in *client_addr, socklen_t client_addr_len);
int main(int argc, char *argv[]) ;