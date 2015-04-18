/*********************************************************
File Name:  DNSClient.h
Author:     Austin Brennan
Course:     CPSC 3600
Instructor: Sekou Remy
Due Date:   03/25/2015


File Description:
This file contains most of my includes and some defined
constants.

*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>     /* for memset() */
#include <netinet/in.h> /* for in_addr */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <netdb.h>      /* for getHostByName() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <unistd.h>     /* for close() */
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>


#define RECV_BUFF_SIZE 1000000     /* Longest string to server */
#define MAX_PENDING 5
#define HTTP_URL_SECTION 7

void dieWithError(char *errorMessage);  /* External error handling function */
void stateProperUsageAndDie(int program_id);
