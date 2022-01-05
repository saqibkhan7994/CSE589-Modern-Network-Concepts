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
 * This contains the client side code
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
#include "../include/client.h"
#include "../include/common.h"

//-------------------------------------------------------------------------------------------------------------------------------------------//

/**
 * create_client function: This functions creates the client socket and bind provided port number with the socket.
 *			    It uses the system call to accept user commands on STDIN and handles incoming data on the socket.
 *			    This function also sends the request to the connected server using Input command and system calls.
 *
 * @param  port Port number on which client runs
 * @return 0 EXIT_SUCCESS
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//

int create_client(char *port)
{
	printf("\nInside client code\n");
	
	/* Structure pointer for list of clients */
	struct client_list *c_list_start =NULL, *c_list_end = NULL;
	
 	/* Declaring variables for system call - Reference taken from Beej Guide*/
	fd_set fds, readfds;
	FD_ZERO (&fds);
	FD_ZERO (&readfds);
	FD_SET (STDIN, &fds);
	int fdmax = STDIN;
	char inbuf [300];
	int size, selret, result;
	char command [1][50];
	char *arg;
	int hostID_count =0;
	char *your_ubit_name = "saqibhus";
	client_count = 0;
	
	struct addrinfo hints, *clientinfo;
	int client_socket = 0;
	int  rv;
	
	/* Set up hints structure - Reference taken from Beej Guide */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	result = get_IP();

	/* Fill up address structures - Reference taken from Beej Guide */
	if ((rv = getaddrinfo(ip_host, port, &hints, &clientinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	/* Socket - Reference taken from Beej Guide */
	if ((client_socket = socket(clientinfo->ai_family, clientinfo->ai_socktype, clientinfo->ai_protocol)) == -1) {
	    perror("client: socket");
	    return 1;
	}
	/* Bind - Reference taken from Beej Guide */
	if (bind(client_socket, clientinfo->ai_addr, clientinfo->ai_addrlen) == -1) {
		close(client_socket);
		perror("server: bind");
		return 1;
	}
	
	/* Starting code of system call - Reference taken from Demo code*/	
	while(1){
		memcpy(&readfds, &fds, sizeof(fds));  //copying master list to a temporary list
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
						char message[300];
						char send_ip[20];
						memset(send_ip,0,strlen(send_ip));
						memset(message,0,strlen(message));
						strcpy(message, inbuf);
						size = 0;

						arg = strtok(inbuf, " "); //Tokenizing the input command
						
						/* Storing the input tokens in an array - Reference from PA Guidance */
						while(arg!=NULL){
							strcpy(command[size], arg);
							size = size+1;
							if(strcmp(command[0], "SEND") ==0){
								arg = strtok(NULL, " ");
								strcpy(send_ip, arg);
								break;
							}   								
							arg = strtok(NULL, " ");
						}
						
						/* Calling method print_author to print AUTHOR command */
						if(strcmp(command[0] , "AUTHOR") ==0){
							print_author(your_ubit_name);
						}
						
						/* Calling method print_ip to print IP command */
						if(strcmp(command[0] , "IP") ==0){
							print_ip(get_IP()); 								
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
							
							/* Iterating over client list to print it */
							while(itr != NULL){
								client_count++;
								cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", client_count, itr->hostname, itr->hostIP, itr->port);
								itr = itr->next;
							}
							cse4589_print_and_log("[%s:END]\n","LIST");
    						}
    						
    						/* code to logout out client from server when LOGOUT command is executed - Reference taken from Beej Guide*/
    						if(strcmp(command[0], "LOGOUT") ==0){
							close(client_socket);
							FD_CLR(client_socket, &fds);
							cse4589_print_and_log("[%s:SUCCESS]\n","LOGOUT");
							cse4589_print_and_log("[%s:END]\n","LOGOUT");
						}
						
						/* Code to refresh the logged in client list when REFRESH command is executed */
						if(strcmp(command[0], "REFRESH") ==0){
							char * refresh = "REFRESH ";
							send(client_socket , refresh , strlen(refresh) , 0 );
							cse4589_print_and_log("[%s:SUCCESS]\n","REFRESH");
							cse4589_print_and_log("[%s:END]\n","REFRESH");
						}
						
						/* Code to block an inputted IP when BLOCK <client-ip> command is executed */
						if(strcmp(command[0], "BLOCK") ==0){
						
							char block[30];
							memset(block,0,strlen(block));
							strcpy(block, message);
							
							char ip_block[20];
							memset(ip_block,0,strlen(ip_block));
							strcpy(ip_block, command[1]);
							int val_ip = 0;
							
							/* IP validation*/
							val_ip = do_ip_validate(ip_block);

							if(val_ip == 0){
								cse4589_print_and_log("[%s:ERROR]\n","BLOCK");
					    			cse4589_print_and_log("[%s:END]\n","BLOCK");
								break;
							}
							
							/* Existing IP in the client list validation */
							int check = check_ip(command[1], c_list_start);
							if(check != 1){
								cse4589_print_and_log("[%s:ERROR]\n", "BLOCK");
								cse4589_print_and_log("[%s:END]\n", "BLOCK");
								break;
							}
							
							/* Sending request to the server to block the input IP */
							send(client_socket , block , strlen(block) , 0 );
							cse4589_print_and_log("[%s:SUCCESS]\n","BLOCK");
							cse4589_print_and_log("[%s:END]\n","BLOCK");
						}
						
						/* Code to unblock any specific ip when UNBLOCK <client-ip> is executed */
						if(strcmp(command[0], "UNBLOCK") ==0){
						
							char unblock[30];
							memset(unblock,0,strlen(unblock));
							strcpy(unblock, message);
							
							char *ip_unblock = command[1];
							int val_ip = 0;
							
							/* IP validation*/
							val_ip = do_ip_validate(ip_unblock);

							if(val_ip == 0){
								cse4589_print_and_log("[%s:ERROR]\n","UNBLOCK");
					    			cse4589_print_and_log("[%s:END]\n","UNBLOCK");
								break;
							}							
							
							/* Existing IP in the client list validation */
							int check = check_ip(ip_unblock, c_list_start);
							if(check != 1){
								cse4589_print_and_log("[%s:ERROR]\n", "UNBLOCK");
								cse4589_print_and_log("[%s:END]\n", "UNBLOCK");
								break;
							}
							
							/* Sending request to the server to unblock the input IP */
							send(client_socket , unblock , strlen(unblock) , 0 );
							cse4589_print_and_log("[%s:SUCCESS]\n","UNBLOCK");
							cse4589_print_and_log("[%s:END]\n","UNBLOCK");
						}
						
						
						/* Code to login into the server on a specific port. Syntax - LOGIN <server-ip> <server - port> */
						if(strcmp(command[0], "LOGIN") ==0){
						
							int valread;
							struct sockaddr_in serv_addr;
							struct addrinfo *servinfo, *p;
							char s[INET_ADDRSTRLEN];
							char *ip = command[1];
							char *port1 = command[2];
							char s_ip[20], s_port[10]; 
							strcpy(s_ip, command[1]);
							strcpy(s_port, command[2]);
							port1[strcspn(port1, "\n")] = 0;
							
							int val_ip = 0, val_port = 0;
							
							/* Exception handling for LOGIN - Validating IP address and Port*/
							val_ip = do_ip_validate(s_ip);
							val_port = do_port_validate(s_port);
							
							if(val_ip == 0 || val_port == 0){
								cse4589_print_and_log("[%s:ERROR]\n","LOGIN");
					    			cse4589_print_and_log("[%s:END]\n","LOGIN");
								break;
							}
							
							/* Calling getaddrinfo to populate serverinfo struct for server IP and port details - Reference from Beej Guide*/
							if ((rv = getaddrinfo(ip, port1, &hints, &servinfo)) != 0) {
								fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
								return 1;
					    		}
							
							// loop through all the results and connect to the first we can
							for(p = servinfo; p != NULL; p = p->ai_next) {
								
								if (connect(client_socket, p->ai_addr, p->ai_addrlen) == -1) {
								    close(client_socket);
								    perror("client: connect");
								    continue;
								}

								break;
							}

							/* Reference from Beej Guide */
							if (p == NULL) {
								fprintf(stderr, "client: failed to connect\n");
								return 2;
							}
							
							/* Adding connected socket in master list for system call handling */
							FD_SET (client_socket, &fds);
							fdmax = client_socket;
							
							inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
							
					    		cse4589_print_and_log("[%s:SUCCESS]\n","LOGIN");
					    		cse4589_print_and_log("[%s:END]\n","LOGIN");

							// Freeing up struct after use
					    		freeaddrinfo(servinfo);
					    		freeaddrinfo(clientinfo);
					    		
						}
						
						/* Code to send data to other clients via server using connections created above */
						if(strcmp(command[0], "SEND") ==0){
						
							char hello[300];
							memset(hello,0,strlen(hello));
							strcpy(hello, message);
							
							/* Exception Handling for SEND - validating IP */
							int check = check_ip(send_ip, c_list_start);
							if(check != 1){
								cse4589_print_and_log("[%s:ERROR]\n", "SEND");
								cse4589_print_and_log("[%s:END]\n", "SEND");
								break;
							}
							
							/* Calling send to send data to client via server */
							send(client_socket , hello , strlen(hello) , 0 ); // Reference from Demo code
							cse4589_print_and_log("[%s:SUCCESS]\n", "SEND");
							cse4589_print_and_log("[%s:END]\n", "SEND");							
						}
						   							
					}
					
					/* code to handle if any data is available to be received at client socket*/
					else if(i_fd == client_socket){
					
						char buffer[300];
						memset(buffer,0,strlen(buffer));
						int rec_bytes = recv(client_socket , buffer, sizeof buffer, 0);
						
						if(rec_bytes == -1){						
							printf("Receive failed\n");
						}
						else if(rec_bytes == 0) // Break if no data is available
							break;
						
						/* Tokenizing received data on the basis of "#" to identify if a client list is sent or actual data from any client*/
						char* token = strtok(buffer, "#");
						if(strcmp(token, "c_list") == 0){
							c_list_start =NULL;
							int length, length2 = 0;
							char list [1][200];
							char list2 [1][300];
							while (token != NULL) {
								if(strcmp(token, "c_list") == 0){ //Using c_list identifier to identify if a logged in client list is sent
									token = strtok(NULL, "#");       									
								}
								strcpy(list2[length2], token);
								length2 = length2+1;
								token = strtok(NULL, "#");
							}	
							while(length2 >0){
								length2 = length2-1;
								char* token1 = strtok(list2[length2], " ");
								length = 0;
								
								while(token1 !=NULL){
									strcpy(list[length], token1);
									length = length+1;
									token1 = strtok(NULL, " ");
								}
								
								hostID_count++;
								
								//Creating a new node to store the client information
								struct client_list *c_list_new = (struct client_list*) malloc(sizeof(struct client_list));
								strcpy(c_list_new->hostIP, list[0]);
								c_list_new->port = atoi(list[1]);
								c_list_new->active_flag = atoi(list[3]);
								c_list_new->hostId = hostID_count;
								strcpy(c_list_new->hostname , list[2]);
								
								//Adding new node to existing list of clients
								c_list_new->next = c_list_start;
								c_list_start = c_list_new;
							}								
						    	sort_list_client(c_list_start); //Calling function to sort the received client list based on port number
					    	}
					    	
					    	/* Code to display the received data from other clients */
					    	else if(strcmp(token, "c_send") == 0){
							char s_ip [20];
							char s_message [300];
							token = strtok(NULL, ":");       									
							strcpy(s_ip, token);
							token = strtok(NULL, ":");
							strcpy(s_message, token);
							cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");
							cse4589_print_and_log("msg from:%s\n[msg]:%s\n", s_ip, s_message);
							cse4589_print_and_log("[%s:END]\n", "RECEIVED");
					    	}
					}
				}
			}
		}
	}
	return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------------//

/**
 * sort_list_client function: This functions sorts the client list received from server based on their port numbers.
 *			       This method uses selection sort algorithm since the number of connections are small.
 * @param  c_list_start: Pointer to the head of clients linked list
 * @return void
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//

void sort_list_client(struct client_list *c_list_start){

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
		
		/* Swapping Data to sort the list*/
		int port = itr->port;
		char hostname[100];
		char hostip[20];
		char blocked_ip[100];
		int active = itr->active_flag;
		int hostid = itr->hostId;
		
		strcpy(hostname, itr->hostname);
		strcpy(hostip, itr->hostIP);
		strcpy(blocked_ip, itr->blocked_ip);

		strcpy(itr->hostname , temp->hostname);
		strcpy(itr->blocked_ip, temp->blocked_ip);
		itr->port = temp->port;
		strcpy(itr->hostIP , temp->hostIP);
		itr->active_flag = temp->active_flag;
		itr->hostId = temp->hostId;

		strcpy(temp->hostname , hostname);
		strcpy(temp->blocked_ip, blocked_ip);
		temp->port = port;
		strcpy(temp->hostIP , hostip);
		temp->active_flag = active;
		temp->hostId = hostid;

		itr = itr->next;
	}
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
