#ifndef GLOBAL_H_
#define GLOBAL_H_

#define HOSTNAME_LEN 128
#define PATH_LEN 256
#define BACKLOG 5
#define STDIN 0
#define GIP "8.8.8.8"
#define GPORT "53"

struct client_list {
 	char hostIP[20];
 	int  port;
 	char hostname[100];
 	int  active_flag;
 	int  hostId;
 	int socket;
 	char blocked_ip[100];
 	struct client_list *next;
 };

char ip_host[20];
int client_count;

#endif
