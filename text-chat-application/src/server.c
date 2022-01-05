/**
 * @saqibhus_assignment1
 * @author  Saqib Hussain Khan <saqibhus@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the server side code
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>

#include "../include/global.h"
#include "../include/logger.h"
#include "../include/server.h"
#include "../include/common.h"

//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * create_server function: This functions creates the server socket and bind provided port number with the socket.
 *			    It uses the system call to accept user commands on STDIN and handles incoming requests from client on the socket.
 *			    This function also received data from a client to target client using system calls.
 *
 * @param  port Port number on which client runs
 * @return 0 EXIT_SUCCESS
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//

int create_server(char *port)
{
	printf("Inside server code\n");
	
	/* Local variable declaration - Reference taken from Beej Guide*/
	struct client_list *c_list_start =NULL, *c_list_end = NULL;
	struct sockaddr_in client_addr;
	struct addrinfo hints, *res;
	
	int hostID_count =0;
	int server_socket, new_socket, valread, caddr_len;
	char ip_addr[INET_ADDRSTRLEN];
	char *your_ubit_name = "saqibhus";

	/* Set up hints structure - Reference taken from Beej Guide */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	/* Fill up address structures - Reference taken from Beej Guide */
	if (getaddrinfo(NULL, port, &hints, &res) != 0)
		perror("getaddrinfo failed");

	printf("\ngetaddrinf: %p\n", res);

	/* Socket - Reference taken from Beej Guide */
	server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(server_socket < 0)
		perror("Cannot create socket");

	/* Bind - Reference taken from Beej Guide */
	if(bind(server_socket, res->ai_addr, res->ai_addrlen) < 0 )
		perror("Bind failed");


	/* Listen - Reference taken from Beej Guide*/
	if(listen(server_socket, BACKLOG) < 0)
		perror("Unable to listen on port");
		
	
	/* Declaring variables of system call */
	fd_set fds, readfds;
	FD_ZERO (&fds);
	FD_ZERO (&readfds);
	FD_SET (server_socket, &fds); //Adding server socket in master set for system call
	FD_SET (STDIN, &fds); // Adding standard input in master set for system call
	int fdmax = server_socket;
	char inbuf [100];
	int size, selret, result;
	char command [1][10];
	char *arg;
	
	/* Starting code of system call - Reference from Demo Code*/
	while(1){
	
		memcpy(&readfds, &fds, sizeof(fds));
		selret = select(fdmax+1, &readfds, NULL, NULL, NULL);
		if(selret<0){
			return printf("Select failed");
		}
		else{
			for (int i_fd = 0; i_fd < (fdmax + 1); i_fd++) {
			
				if(FD_ISSET(i_fd, &readfds)){
				
					if(i_fd == STDIN){
						memset(inbuf,0,strlen(inbuf));
						
						if (fgets (inbuf, sizeof (inbuf),  stdin) == NULL)
							return printf("Enter an input\n");
							
						inbuf[strcspn(inbuf, "\n")] = 0; //Removing the trailing newline character
						size = 0;
						arg = strtok(inbuf, " ");
						memset(command,0,strlen(command[1])); //Tokenizing the input command
						
						/* Storing the input tokens in an array - Reference from PA Guidance */
						while(arg!=NULL){
							strcpy(command[size], arg);
							size = size+1;   								
							arg = strtok(NULL, " ");
						}
						
						/* Calling method print_author to print AUTHOR command */
						if(strcmp(command[0] , "AUTHOR") ==0){
							print_author(your_ubit_name);
						}
						
						/* Calling method print_ip to print IP command */
						if(strcmp(command[0] , "IP") ==0){
							result = get_IP();
							print_ip(result);
						}
						
						/* Calling method print_port to print PORT command */
						if(strcmp(command[0], "PORT") ==0){
							print_port(port);
						}
						
						/* Code when LIST command is executed */
						if(strcmp(command[0], "LIST") ==0){
    								
							cse4589_print_and_log("[%s:SUCCESS]\n","LIST");
							struct client_list *itr;
							itr = c_list_start;
							client_count = 0;
							sort_list_server(c_list_start); //Sorting the clients list
							/* Iterating over client list to print it */
							while(itr != NULL){
								client_count++;
								if(itr->active_flag !=0)
									cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", client_count, itr->hostname, itr->hostIP, itr->port);
								itr = itr->next;
							}
							cse4589_print_and_log("[%s:END]\n","LIST");
    						}
    						
    						/* Code to print the list of blocked clients by any input client - Syntax BLOCKED <client-ip> */
    						if(strcmp(command[0], "BLOCKED") ==0){
							
							char ip_block[20];
							memset(ip_block,0,strlen(ip_block));
							strcpy(ip_block, command[1]);
							int val_ip = 0;
							
							/* Exception Handling - IP validation*/
							val_ip = do_ip_validate(ip_block);

							if(val_ip == 0){
								cse4589_print_and_log("[%s:ERROR]\n","BLOCKED");
					    			cse4589_print_and_log("[%s:END]\n","BLOCKED");
								break;
							}
							
							/* Exception Handling - Existing IP in the client list validation */
							int check = check_ip(command[1], c_list_start);
							if(check != 1){
								cse4589_print_and_log("[%s:ERROR]\n", "BLOCKED");
								cse4589_print_and_log("[%s:END]\n", "BLOCKED");
								break;
							}
							
							struct client_list *block_clist =NULL;
							char *blocked_ips = getBlockedList(command[1], c_list_start); //Getting the list of blocked clients separated by #
							char *tok;
							int c_count;
							tok = strtok(blocked_ips, "#"); //Tokenizing based on #
							
							while(tok != NULL){
								int b_port = getPort(tok, c_list_start);
								if(b_port == 0){
									tok = strtok(NULL, "#");
									continue;
								}
								
								/* Creating a node of blocked client and storing it in the linked list */
								struct client_list *block_clist_new = (struct client_list*) malloc(sizeof(struct client_list));
								char *b_hostName = getHostName(tok, c_list_start);
								strcpy(block_clist_new->hostIP, tok);
								block_clist_new->port = b_port;
								strcpy(block_clist_new->hostname , b_hostName);
								block_clist_new->next = block_clist;
								block_clist = block_clist_new;
								tok = strtok(NULL, "#");
							}
							
							sort_list_server(block_clist); //Sorting the blocked clients list
							
							/* Printing the list of blocked clients */
							struct client_list *itr;
							itr = block_clist;
							c_count = 0;
							cse4589_print_and_log("[%s:SUCCESS]\n","BLOCKED");
							while(itr != NULL){
								c_count++;
								cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", c_count, itr->hostname, itr->hostIP, itr->port);
								itr = itr->next;
							}
							cse4589_print_and_log("[%s:END]\n","BLOCKED");
						}
						   							
					}
					
					/* Code to check if any incoming client connection is present*/
					else if(i_fd == server_socket){
						/* Accept */
						caddr_len = sizeof(client_addr);
						if ((new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len))<0){
							perror("accept");
							exit(EXIT_FAILURE);
						}
						else {
						
							/* Add newly created socket in the master set */
                        				FD_SET(new_socket, &fds); 
                        				if (new_socket > fdmax) {
                            					fdmax = new_socket;
                        				}
                        				
                        				/* Adding new client to existing logged in client list */
                        				c_list_start = add_client(new_socket, client_addr, hostID_count, c_list_start);
                        
                        				/* Calling function to send list of logged in clients to the newly connected client */
                        				send_client_list(new_socket, c_list_start);
                        			}
                        		}
                        		/* Code to check if any data or request is received and process it */
                        		else{
                        			char buffer[1024] = {0};  
                        			memset(buffer,0,strlen(buffer)); 
						valread = recv(i_fd , buffer, sizeof buffer, 0);
						
						if(valread == -1)
							printf("Receive failed\n");
							
						else if(valread == 0){
							client_status(i_fd, c_list_start); // Changing the status of logged in client to logged out
							close(i_fd); // Closing the socket
							FD_CLR(i_fd, &fds); // Removing it from master set
						}
						/* If data is present then processing it */
						else{	
							char *source_ip = get_source_ip(i_fd, c_list_start);					
							char *token;
							char message[257];
							char final_message[300];
							memset(message,0,strlen(message));
							memset(final_message,0,strlen(final_message));
							char client_ip[20];
							token = strtok(buffer, " ");
							
							/* Checking if client has sent some data to other client */
							if(strcmp(token, "SEND") ==0){ // Checking if SEND command is executed by the client
								token = strtok(NULL ," ");
								strcpy(client_ip, token);
								token = strtok(NULL, " ");
								strcat(final_message, "c_send#"); // Tokenizing the received data based of c_send identifier
								strcat(final_message, source_ip);
								strcat(final_message, ":");
								
								while(token != NULL){								
									strcat(message, token);
									token = strtok(NULL, " ");
									if(token != NULL){
										strcat(message, " ");
									}								
								}
								
								/* Code to check if the destination client has blocked sending client*/
								int blockFlag =0;
								blockFlag = isBlocked(source_ip, client_ip, c_list_start);
								if(blockFlag == 1)
									break;
									
								/* If not blocked then send message */	
								strcat(final_message, message);
								int socket = get_client_socket(client_ip, c_list_start); // Get destination client socket descriptor 
								
								if (socket !=0){
									send(socket , final_message , sizeof final_message , 0 ); // executing send function
									cse4589_print_and_log("[%s:SUCCESS]\n", "RELAYED");
									cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", source_ip, client_ip, message);
									cse4589_print_and_log("[%s:END]\n", "RELAYED");
								}
							}
							/* Checking if client has requested refreshed client list*/
							else if(strcmp(token, "REFRESH") ==0){
								send_client_list(i_fd, c_list_start);
							}
							/* Checking if client has requested to block another client */
							else if(strcmp(token, "BLOCK") ==0){
								char block_ip [20];
								token = strtok(NULL, " ");
								strcpy(block_ip, token);
								char *s_ip = get_source_ip(i_fd, c_list_start);
								block_client(block_ip, s_ip, c_list_start);			
								
							}
							
							free(source_ip); //Freeing up used variable
						}

					}
				}
				
			}
		}
	}
	freeaddrinfo(res);
	return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * sort_list_server function: This functions sorts the active client list based on their port numbers.
 *			       This method uses selection sort algorithm since the number of connections are small.
 * @param  c_list_start: Pointer to the head of clients linked list
 * @return void
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
void sort_list_server(struct client_list *c_list_start){

	struct client_list *itr;
	itr = c_list_start;
	
	while (itr != NULL) {
		struct client_list *temp = itr;
		struct client_list *current = temp->next;

		while (current != NULL) {
		    if (temp->port > current->port)
			temp = current;

		    current = current->next;
		}

		int port_temp = itr->port;
		char hostname[100];
		char hostip[20];
		char blocked_ip[100];
		int active = itr->active_flag;
		int hostid = itr->hostId;
		int socket = itr->socket;
		
		strcpy(hostname, itr->hostname);
		strcpy(hostip, itr->hostIP);
		strcpy(blocked_ip, itr->blocked_ip);

		strcpy(itr->hostname , temp->hostname);
		strcpy(itr->blocked_ip, temp->blocked_ip);
		itr->port = temp->port;
		strcpy(itr->hostIP , temp->hostIP);
		itr->active_flag = temp->active_flag;
		itr->hostId = temp->hostId;
		itr->socket = temp->socket;

		strcpy(temp->hostname , hostname);
		strcpy(temp->blocked_ip, blocked_ip);
		temp->port = port_temp;
		strcpy(temp->hostIP , hostip);
		temp->active_flag = active;
		temp->hostId = hostid;
		temp->socket = socket;

		itr = itr->next;
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * get_client_socket function: This functions returns the socket descriptor of the client ip passed in the arguments.
 *			 
 * @param  c_list_start: Pointer to the head of clients linked list
 * @param  ip: Pointer to the client IP
 * @return int: socket descriptor
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//

int get_client_socket(char *ip, struct client_list *c_list_start){

	struct client_list *itr;
	itr = c_list_start;
	
	int socket =0;
	while(itr != NULL){
		if(strcmp(itr->hostIP, ip) ==0){
			socket = itr->socket;
			break;
		}
		itr = itr->next;		
	}
	
	return socket;
}

//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * client_status function: This functions returns the logged in or logged out status of the client.
 *			 
 * @param  c_list_start: Pointer to the head of clients linked list
 * @param  socket: socket number of the connected client
 * @return void
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
void client_status(int socket, struct client_list *c_list_start){

	struct client_list *itr;
	itr = c_list_start;
	
	while(itr != NULL){
		if(socket == itr->socket){
			itr->active_flag = 0;
			break;
		}
		itr = itr->next;		
	}			
}

//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * add_client function: This functions adds newly connected client in the linked list
 *			 
 * @param  c_list_start: Pointer to the head of clients linked list
 * @param  socket: socket number of the connected client
 * @param  client_addr: sockaddr_in structure containing client address
 * @param  hostID_count: integer variable containing number of clients connected
 * @return client_list: Returns the pointer to head of linked list
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
struct client_list* add_client(int socket, struct sockaddr_in client_addr, int hostID_count, struct client_list *c_list_start){

	struct hostent *he; //from Beej Guide
	char str[INET_ADDRSTRLEN];
	
	/* Used to get IP address using sockaddr_in object - Reference from Beej Guide */
	inet_ntop(AF_INET, &(client_addr.sin_addr), str, sizeof client_addr);
	hostID_count++;
	
	/* Creating new node and adding it to the exixting client linked list */
	struct client_list *c_list_new = (struct client_list*) malloc(sizeof(struct client_list));

	strcpy(c_list_new->hostIP, str);
	c_list_new->port = ntohs(client_addr.sin_port);	
	c_list_new->active_flag = 1;	
	c_list_new->hostId = hostID_count;	
	c_list_new->socket = socket;
	memset(c_list_new->blocked_ip,0,strlen(c_list_new->blocked_ip));
	
	/* Fetching Host Name - Reference from Beej Guide */
	he = gethostbyaddr(&(client_addr.sin_addr), sizeof (client_addr.sin_addr), AF_INET);
	strcpy(c_list_new->hostname , he->h_name);
	
	//Add new node to list
	c_list_new->next = c_list_start;
	c_list_start = c_list_new;
	
	return c_list_start;
}

//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * send_client_list function: This functions sends the client list to the newly connected client
 *			 
 * @param  c_list_start: Pointer to the head of clients linked list
 * @param  socket: socket number of the connected client
 * @return void
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
void send_client_list(int socket, struct client_list *c_list_start){

	struct client_list *itr;
	itr = c_list_start;
	char c_list[256];
	
	/*Appending c_list identifier at the start and # separator between two client lists*/
	memset(c_list,0,strlen(c_list));
	strcat(c_list , "c_list");
	strcat(c_list , "#");
	char active_flag[2], c_port[10];
	
	while(itr != NULL){

		if(itr->active_flag == 1){
			strcat(c_list , itr->hostIP);
			strcat(c_list , " ");
			sprintf(c_port, "%d", itr->port);
			strcat(c_list , c_port);
			strcat(c_list , " ");
			strcat(c_list , itr->hostname);
			strcat(c_list , " ");
			sprintf(active_flag, "%d", itr->active_flag);
			strcat(c_list , active_flag);
			strcat(c_list , "#");
			itr = itr->next;
		}
		else{
			itr = itr->next;
		}
	}
	
	send(socket , c_list , sizeof c_list , 0 ); //sending client list
}

//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * get_source_ip function: This functions returns the ip of the client connected to a socket.
 *			 
 * @param  c_list_start: Pointer to the head of clients linked list
 * @param  socket: socket number of the connected client
 * @return char: Pointer to the ip of connected client on a specific socket
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
char* get_source_ip(int socket, struct client_list *c_list_start){

	struct client_list *itr;
	itr = c_list_start;
	
	char *source_ip = (char*) malloc(sizeof(char)*20);
	while(itr != NULL){
		if(itr->socket == socket){
			strcpy(source_ip, itr->hostIP);
			break;
		}
		itr = itr->next;		
	}
	
	return source_ip;
}

//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * block_client function: This functions blocks the client by adding it's IP to the blocked client list.
 *			 
 * @param  c_list_start: Pointer to the head of clients linked list
 * @param  source_ip: IP of the client requesting block
 * @param  block_ip: IP of the client being blocked
 * @return void
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
void block_client(char *block_ip, char *source_ip, struct client_list *c_list_start){
	struct client_list *itr;
	itr = c_list_start;
	int flag = 0;
	
	while(itr != NULL){
		if(strcmp(itr->hostIP, source_ip) ==0){
			break;
		}
		itr = itr->next;		
	}
	
	char blocked_ip[100];
	memset(blocked_ip,0,strlen(blocked_ip));
	strcpy(blocked_ip, itr->blocked_ip);
	
	char * token = strtok(blocked_ip , "#");
	if(token == NULL){
		strcpy(itr->blocked_ip, block_ip);
		flag = 1;
	}
	while(token != NULL){
		if(strcmp(token, block_ip) == 0){
			flag = 1;
			break;
		}
		token = strtok(NULL, "#");			
	}
	if(flag != 1){
		strcat(itr->blocked_ip, "#");
		strcat(itr->blocked_ip, block_ip);
	}		
}

//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * isBlocked function: This functions checks if a client is blocked by a specific client
 *			 
 * @param  c_list_start: Pointer to the head of clients linked list
 * @param  source_ip: IP of the client requesting block
 * @param  block_ip: IP of the client being blocked
 * @return int: Flag 1 if blocked, otherwise 0
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
int isBlocked(char *source_ip, char *dest_ip, struct client_list *c_list_start){
	struct client_list *itr;
	itr = c_list_start;
	
	while(itr != NULL){
		if(strcmp(itr->hostIP, dest_ip)==0){
			break;
		}
		itr = itr->next;		
	}
	if(itr->blocked_ip == NULL){
		return 0;
	}
	else{
		char blocked_ip[100];
		memset(blocked_ip,0,strlen(blocked_ip));
		strcpy(blocked_ip, itr->blocked_ip);
		
		char * token = strtok(blocked_ip , "#");
		while(token != NULL){
			if(strcmp(token, source_ip) == 0){
				return 1;
			}
			token = strtok(NULL, "#");			
		}	
	}
	return 0;	
}

//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * getBlockedList function: This functions returns a # separated list of blocked clients by a specific client
 *			 
 * @param  c_list_start: Pointer to the head of clients linked list
 * @param  client_ip: IP of the client storing blocked list
 * @return char: string of blocked clients
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
char* getBlockedList(char *client_ip, struct client_list *c_list_start){
	struct client_list *itr;
	itr = c_list_start;
	
	char *blockedList = (char*) malloc(sizeof(char)*100);
	while(itr != NULL){
		if(strcmp(itr->hostIP, client_ip) ==0){
			strcpy(blockedList, itr->blocked_ip);
		}
		itr = itr->next;		
	}
	
	return blockedList;
}

//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * getPort function: This functions returns the port of a specific client
 *			 
 * @param  c_list_start: Pointer to the head of clients linked list
 * @param  client_ip: IP of the client
 * @return int: returns the port of a connected client
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
int getPort(char *client_ip, struct client_list *c_list_start){
	
	struct client_list *itr;
	itr = c_list_start;
	
	while(itr != NULL){
		if(strcmp(itr->hostIP, client_ip) ==0){
			return itr->port;
		}
		itr = itr->next;		
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * getHostName function: This functions returns the hostname of a specific client
 *			 
 * @param  c_list_start: Pointer to the head of clients linked list
 * @param  client_ip: IP of the client storing blocked list
 * @return char: hostname of a client
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
char* getHostName(char *client_ip, struct client_list *c_list_start){
	
	struct client_list *itr;
	itr = c_list_start;
	
	char *hostName = (char*) malloc(sizeof(char)*50);
	while(itr != NULL){
		if(strcmp(itr->hostIP, client_ip) ==0){
			strcpy(hostName, itr->hostname);
			break;
		}
		itr = itr->next;		
	}
	
	return hostName;
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
