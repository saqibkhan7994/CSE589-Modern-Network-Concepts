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
 * This contains the common functions used by client and server
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

//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * get_in_addr function - Returns the family of Internet Address - IPv4 or IPv6
 *
 * @param  sa: sockaddr struct containing socket details
 * @return void
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
	return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * get_IP function - Returns the ip address of a client connected on a socket
 * Reference from https://ubmnc.wordpress.com/2010/09/22/on-getting-the-ip-name-of-a-machine-for-chatty/
 * @param  NONE
 * @return int: returns the result if IP has been successfully stored in global variable
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
 int get_IP(){

	int sockfd;
	struct sockaddr_in s_addr;
	struct addrinfo hints, *servinfo, *p;
	int rv, cn, flag;
	char str[INET_ADDRSTRLEN];

	/* Set up hints structure - Reference taken from Beej Guide */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	/* Set up hints structure - Reference taken from Beej Guide */
	if ((rv = getaddrinfo(GIP, GPORT, &hints, &servinfo)) != 0) {
		perror("getaddrinfo failed");
	}

	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,
			servinfo->ai_protocol);
	if(sockfd < 0)
		perror("Cannot create socket");

	/* Set up hints structure - Reference taken from Beej Guide */
	cn = connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
	
	if(cn < 0)
		perror("Cannot connect");
		
	int size_addr = sizeof(s_addr);
	/* Function to get the socket name */
	if (getsockname(sockfd,(struct sockaddr *)&s_addr,(unsigned int*) &size_addr) == -1)
    	{
        	perror("getsockname");
        	flag = 0;
    	}
    	else{
		inet_ntop(AF_INET, &(s_addr.sin_addr), str, size_addr);
		flag = 1;
	}

	freeaddrinfo(servinfo);
	close(sockfd);
	strcpy(ip_host, str); //Storing hostname in global variable
	return flag;
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * print_ip function - prints the IP address
 *
 * @param  result: 1 if IP is loaded in global variable, otherwise 0
 * @return void
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
void print_ip(int result){
	if( result!=0)
	{
		cse4589_print_and_log("[%s:SUCCESS]\n","IP");
		cse4589_print_and_log("IP:%s\n", ip_host);
		cse4589_print_and_log("[%s:END]\n","IP");
	}
	else{
	
		cse4589_print_and_log("[%s:ERROR]\n", "IP");
		cse4589_print_and_log("[%s:END]\n", "IP");
	}
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * print_author function - prints the author and AI policy
 *
 * @param  NONE
 * @return void
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
void print_author(char *ubit_name){
	cse4589_print_and_log("[%s:SUCCESS]\n","AUTHOR");
    cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", ubit_name);
    cse4589_print_and_log("[%s:END]\n","AUTHOR");
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * print_port function - prints the connected port
 *
 * @param  port: Port of the connected host
 * @return void
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
void print_port(char *port){
	cse4589_print_and_log("[%s:SUCCESS]\n","PORT");
	cse4589_print_and_log("PORT:%s\n",port);
	cse4589_print_and_log("[%s:END]\n","PORT");
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * check_ip function - check ip IP is present in the list
 *
 * @param  port: Port of the connected host
 * @return int: 1 if Yes, otherwise 0
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
int check_ip(char *ip, struct client_list *c_list_start){

	struct client_list *itr;
	itr = c_list_start;
	
	int result =0;
	while(itr != NULL){
		if(strcmp(itr->hostIP, ip) ==0){
			result = 1;
			break;
		}
		itr = itr->next;		
	}
	
	return result;
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * is_Number function - checks if passed string is a number
 *
 * @param  digits: any string
 * @return int: 1 if Number, otherwise 0
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
int is_Number(char *digits) {
   while (*digits) {
      if(!isdigit(*digits)){
         return 0;
      }
      digits++;
   }
   return 1;
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * do_ip_validate function - validates any IP
 * IP address validation reference taken from https://www.tutorialspoint.com/c-program-to-validate-an-ip-address*
 * @param  input_ip: IP of any host
 * @return int: 1 if valid IP, otherwise 0
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
int do_ip_validate(char *input_ip) {
   int i, n, token = 0;
   char *digits;
   if (input_ip == NULL)
      return 0;
      
      digits = strtok(input_ip, ".");
      if (digits == NULL)
         return 0;
         
   while (digits != NULL) {
      if (!is_Number(digits))
         	return 0;
         n = atoi(digits);
         
         if (n >= 0 && n<= 255) {
            digits = strtok(NULL, ".");
            if (digits != NULL)
               token++;
         } else
            return 0;
    }
    if (token != 3)
       return 0;
    return 1;
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * do_port_validate function - Validates the port of any connected host
 *
 * @param  input_port: Port of the connected host
 * @return int: 1 if valid IP, otherwise 0
 */
//-------------------------------------------------------------------------------------------------------------------------------------------//
int do_port_validate(char *input_port){
	if (!is_Number(input_port))
         	return 0;
        if(atoi(input_port) >= 65536){
        	return 0;
        }
        return 1;
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
