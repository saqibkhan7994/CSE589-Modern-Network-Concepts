#ifndef SERVER_H_
#define SERVER_H_

int create_server(char *port);
void sort_list_server(struct client_list *c_list_start);
int get_client_socket(char *ip, struct client_list *c_list_start);
void client_status(int socket, struct client_list *c_list_start);
struct client_list* add_client(int socket, struct sockaddr_in client_addr, int hostID_count, struct client_list *c_list_start);
void send_client_list(int socket, struct client_list *c_list_start);
char* get_source_ip(int socket, struct client_list *c_list_start);
void block_client(char *block_ip, char *source_ip, struct client_list *c_list_start);
int isBlocked(char *source_ip, char *dest_ip, struct client_list *c_list_start);
int getPort(char *client_ip, struct client_list *c_list_start);
char* getHostName(char *client_ip, struct client_list *c_list_start);
char* getBlockedList(char *client_ip, struct client_list *c_list_start);

#endif
