/*********************************************************
File Name:  httpClient.cpp
Author:     Austin Brennan, Akshit Gupta, & Connor Pickard
Course:     CPSC 3600
Instructor: Sekou Remy
Due Date:   04/24/2015


File Description:
This file contains an implementation of a simple our
client implementation for our program.

*********************************************************/

#include "Header.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <ctime>
#include <time.h>

using namespace std;

void catchAlarm(int);

int main (int argc, char *argv[])
{

   int len = atoi(argv[1]);
   int num = atoi(argv[2]);
   float angle =  M_PI - ((M_PI * (num - 2)) / num);
   int port = atoi(argv[3]);
   char *ip = argv[4];
   unsigned int TIMEOUT_SECONDS = 5;
   struct sigaction timeoutAction;
   char sendBuffer[1000];
   char recvBuffer[1000];
   float speed = 0.2;
   
   int sock;
   struct sockaddr_in servAddr;
   struct sockaddr_in clntAddr;
   struct hostent *thehost;
   unsigned int clntAddrLen;
   int recvMsgSize;
   int sendMsgSize;
   
   clock_t startTime;
   double secondsPassed;

   char turn[20];
   sprintf(turn, "TURN %f", angle);
   char move[20];
   sprintf(move, "MOVE %d", len);
      
   printf("Len: %d, Num: %d, Angle: %f\n", len, num, angle);
   cout << "Port: " << port << " IP: " << ip << endl;
   vector<string> v;
 
   for (int i=0; i<num; i++)
   {
      v.push_back(move);
      v.push_back("STOP");
      v.push_back("GET IMAGE");
      v.push_back("GET DGPS");
      v.push_back("GET GPS");
      v.push_back("GET LASERS");
      v.push_back(turn);
      v.push_back("STOP");
   }
   
   vector<string>::iterator it;
   for (it = v.begin(); it != v.end(); it++)
      cout << *it << endl;
      
   // Create socket for sending/receiving datagrams 
   if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
      cout << "socket() failed" << endl;

   // Construct local address structure 
   memset(&servAddr, 0, sizeof(servAddr));   // Zero out structure 
   servAddr.sin_family = AF_INET;                  // Internet address family 
   servAddr.sin_addr.s_addr = htonl(INADDR_ANY);   // Any incoming interface 
   servAddr.sin_port = htons(port);       // Local port    
   
   // If user gave a dotted decimal address, we need to resolve it  
   if (servAddr.sin_addr.s_addr == -1) {
      thehost = gethostbyname(ip);
      servAddr.sin_addr.s_addr = *((unsigned long *) thehost->h_addr_list[0]);
   }
	
   //Run loop until all of v has been sent 
//   for (it = v.begin(); it != v.end(); it++)
//   {
	memset (sendBuffer, ' ', sizeof(sendBuffer));
	memset (recvBuffer, ' ', sizeof(recvBuffer));
	clntAddrLen = sizeof(clntAddr);
   
   //Current code set up to send an int to ease testing, will be removed later. 
   printf("enter guess: ");
   gets(sendBuffer);
   cout << "Guess is: " << sendBuffer << endl;
   
   //Send command
	sendMsgSize = strlen(sendBuffer);
	if (sendto(sock, sendBuffer, sendMsgSize, 0, (struct sockaddr *) &servAddr, sizeof(servAddr)) != sendMsgSize)
  		cout << "Error on sendto" << endl; 
   
   //start timer
   startTime = time(0);
  		
   //Set timeout
   timeoutAction.sa_handler = catchAlarm;
   if (sigfillset(&timeoutAction.sa_mask) < 0)
      cout << "Sigfillset() failed" << endl;
   timeoutAction.sa_flags = 0;
   if (sigaction(SIGALRM, &timeoutAction, 0) < 0)
      cout << "Sigaction() failed" << endl;
      
   alarm(TIMEOUT_SECONDS);
   
   //Recv ack
   clntAddrLen = sizeof(clntAddr);
   if ((recvMsgSize = recvfrom(sock, recvBuffer, 1000, 0, (struct sockaddr *) &clntAddr, &clntAddrLen )) < 0 )
   	cout << ("error on recvFrom") << endl; 
   	
   //Check for timeout
   if (recvMsgSize == -1) 
      if (errno == EINTR)
      {
         double timePassed1 = difftime(time(0), startTime);
         cout << "Timeout occured. Program Ending. Time passed is " << timePassed1 << endl;
      }
      
   //Sleep based on timer if move or send
   double timePassed2 = difftime(time(0), startTime);
   sleep((len/speed) - timePassed); //Need to check math
//   }
   
   
return 0;
}

void catchAlarm(int ignored) { return; }
