#ifndef COMMON_H_
#define COMMON_H_

void *get_in_addr(struct sockaddr *sa);
int get_IP();
void print_ip(int result);
void print_author(char *ubit_name);
void print_port(char *port);
int check_ip(char *ip, struct client_list *c_list_start);
int is_Number(char *digits);
int do_ip_validate(char *input_ip);
int do_port_validate(char *input_port);

#endif
