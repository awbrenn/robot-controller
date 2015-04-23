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
//#include <string>
//#include <cstdlib>
#include <math.h>
//#include <ctime>

using namespace std;
//Functions
void catchAlarm(int);
void createVector(int len, int num);
void initalizeMiddleware();
string convertHeaderInfoToString(uint32_t header_info);
vector<string> addHeaderToCommands(vector<string> commands);
void recieveAckFromMiddleware();

//Globals
vector<string> v;
int sock;
struct sockaddr_in servAddr;
struct sockaddr_in clntAddr;
struct hostent *thehost;
unsigned int clntAddrLen;
int port;
char *ip;
char recvBuffer[RECV_BUFF_SIZE];
uint32_t REQUEST_ID = 546; 
string ROBOT_ID = "town2";
float angleFirst, angleSecond;

int main (int argc, char *argv[])
{

   int len = atoi(argv[3]);
   int num = atoi(argv[4]);
   port = atoi(argv[2]);
   ip = argv[1];
   float speed = 0.2;
   
   unsigned int TIMEOUT_SECONDS = 5;
   struct sigaction timeoutAction;
  
   cout << "Port: " << port << " IP: " << ip << endl;
   
   //Set timeout
   timeoutAction.sa_handler = catchAlarm;
   if (sigfillset(&timeoutAction.sa_mask) < 0)
      cout << "Sigfillset() failed" << endl;
   timeoutAction.sa_flags = 0;
   if (sigaction(SIGALRM, &timeoutAction, 0) < 0)
      cout << "Sigaction() failed" << endl;
   
   //Call function to populate the vector
   createVector(len, num);
     
   //Call function to start connection
   initalizeMiddleware();   
   
   //Call function to add header to vector components
   //v = addHeaderToCommands(v);
   
   printf("Len: %d, Num: %d, AngleFirst: %f, AngleSecond: %f\n", len, 
      num, angleFirst, angleSecond);
      
   //Print content of vector
   vector<string>::iterator it;
   // for (it = v.begin(); it != v.end(); it++)
   //    cout << *it << endl;
   
   
   int i = 0;
   //Run loop until all of v has been sent 
   for (it = v.begin(); it != v.end(); it++)
   {
      memset (recvBuffer, 0, sizeof(recvBuffer));
      clntAddrLen = sizeof(clntAddr);
       
      //Send command
      if (sendto(sock, v.at(i).c_str(), v.at(i).length(), 0, 
         (struct sockaddr *) &servAddr, sizeof(servAddr)) != v.at(i).length())
	      cout << "Error on sendto" << endl; 
         
      alarm(TIMEOUT_SECONDS);

      //Recv ack
      recieveAckFromMiddleware();
      i++;
   }
   
   
return 0;
}

void catchAlarm(int ignored) { return; }

void createVector(int len, int num)
{
   angleFirst =  M_PI - ((M_PI * (num - 2)) / num);
   angleSecond = M_PI - ((M_PI * (num - 3)) / (num - 1));
   char turnFirst[20];
   sprintf(turnFirst, "TURN %f", angleFirst);
   char moveFirst[20];
   sprintf(moveFirst, "MOVE %d", len);
 
   for (int i=0; i<num; i++)
   {
      v.push_back(moveFirst);
      v.push_back("STOP");
      //v.push_back("GET IMAGE");
      v.push_back("GET DGPS");
      v.push_back("GET GPS");
      v.push_back("GET LASERS");
      v.push_back(turnFirst);
      v.push_back("STOP");
   }

   char turnSecond[20];
   sprintf(turnSecond, "TURN -%f", angleSecond);
   char moveSecond[20];
   sprintf(moveSecond, "MOVE %d", len);
   
   for (int j=0; j<num-1; j++)
   {
      v.push_back(moveSecond);
      v.push_back("STOP");
      //v.push_back("GET IMAGE");
      v.push_back("GET DGPS");
      v.push_back("GET GPS");
      v.push_back("GET LASERS");
      v.push_back(turnSecond);
      v.push_back("STOP");
   }
}

void initalizeMiddleware()
{
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
}

string convertHeaderInfoToString(uint32_t header_info) 
{
    string header_info_string;
    char uint4_chunk;
    int size_of_uint32_t = 4;
    int bits_in_a_byte = 8;

    cout << "HEADER INFO:" << header_info << endl;

    for (int i = 0; i < size_of_uint32_t; ++i) {
        uint4_chunk = (char) ((header_info << (i * bits_in_a_byte)) >> (3 * bits_in_a_byte));
        cout << "step " << i << ": " << header_info << ", " << (uint32_t) uint4_chunk << endl;
        header_info_string.push_back(uint4_chunk);
    }

    return header_info_string;
}

vector<string> addHeaderToCommands(vector<string> commands) 
{
    uint32_t number_of_commands = commands.size();
    string body;
    int i;

    /* add header to each of the fragmented responses */
    for (i = 0; i < (int) number_of_commands; i++) {
        body = commands[i];
        body = convertHeaderInfoToString(REQUEST_ID)
             + ROBOT_ID
             + body;
        commands[i] = body;
    }

    return commands;
}


void recieveAckFromMiddleware() 
{
  int recvMsgSize;

  clntAddrLen = sizeof(clntAddr);
  if ((recvMsgSize = recvfrom(sock, recvBuffer, UDP_PACKET_MAX_SIZE, 0, 
     (struct sockaddr *) &clntAddr, &clntAddrLen )) < 0 )
       cout << ("error on recvFrom") << endl;

  //Check for timeout
  if (recvMsgSize == -1) 
     if (errno == EINTR)
     {
        cout << "Timeout occured. Program Ending." << endl;
        exit(0);
     }

  recvBuffer[recvMsgSize] = '\0';
  printf("%s\n\n", recvBuffer);

  alarm(0);
}
