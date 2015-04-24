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
vector<string> shape1, shape2;
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
   double moveSpeed = 0.2;
   double turnSpeed = 0.2;
   double moveSleep, turnSleep;
   size_t found;
   
   unsigned int TIMEOUT_SECONDS = 5;
   struct sigaction timeoutAction;
  
   cout << "Port: " << port << " IP: " << ip << endl;
   
   //Open file for output
   FILE *output;
   output = fopen("output.txt", "w");
   
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
   //shape1 = addHeaderToCommands(shape1);
   //shape2 = addHeaderToCommands(shape2);
   
   printf("Len: %d, Num: %d, AngleFirst: %f, AngleSecond: %f\n", len, 
      num, angleFirst, angleSecond);
      
   //Print content of vector
   vector<string>::iterator it;
   // for (it = v.begin(); it != v.end(); it++)
   //    cout << *it << endl;

   int i = 0;
   //Run loop until all of shape1 has been sent 
   for (it = shape1.begin(); it != shape1.end(); it++)
   {
      memset (recvBuffer, 0, sizeof(recvBuffer));
      clntAddrLen = sizeof(clntAddr);
       
      //Send command
      if (sendto(sock, shape1.at(i).c_str(), shape1.at(i).length(), 0, 
         (struct sockaddr *) &servAddr, sizeof(servAddr)) != shape1.at(i).length())
	      cout << "Error on sendto" << endl; 

      alarm(TIMEOUT_SECONDS);

      //Recv ack
      recieveAckFromMiddleware();

      //Shape1 sleep for move
      found = shape1.at(i).find("MOVE");
      if (found != std::string::npos)
      { 
         cout << "Sleeping for shape1 move" << endl;
         moveSleep = len / moveSpeed * 1000000;
         usleep(moveSleep);
      }
      
      //Shape1 sleep for turn
      found = shape1.at(i).find("TURN");
      if (found != std::string::npos)
      {
         cout << "Sleeping for shape1 turn" << endl;
         turnSleep = angleFirst / turnSpeed * 1000000;
         usleep(turnSleep);
      }
      
      i++;
   }
   
   i = 0;
   //Run loop until all of shape2 has been sent 
   for (it = shape2.begin(); it != shape2.end(); it++)
   {
      memset (recvBuffer, 0, sizeof(recvBuffer));
      clntAddrLen = sizeof(clntAddr);
       
      //Send command
      if (sendto(sock, shape2.at(i).c_str(), shape2.at(i).length(), 0, 
         (struct sockaddr *) &servAddr, sizeof(servAddr)) != shape2.at(i).length())
	      cout << "Error on sendto" << endl; 
         
      alarm(TIMEOUT_SECONDS);

      //Recv ack
      recieveAckFromMiddleware();
      
      //Shape2 sleep for move
      found = shape2.at(i).find("MOVE");
      if (found != std::string::npos)
      { 
         cout << "Sleeping for shape2 move" << endl;
         moveSleep = len / moveSpeed * 1000000;
         usleep(moveSleep);
      }
      //Shape2 sleep for turn
      found = shape2.at(i).find("TURN");
      if (found != std::string::npos)
      {
         cout << "Sleeping for shape2 turn" << endl;
         turnSleep = angleSecond / turnSpeed * 1000000;
         usleep(turnSleep);
      }
      
      i++;
   }
   
   fclose(output);
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
      shape1.push_back(moveFirst);
      shape1.push_back("STOP");
      //shape1.push_back("GET IMAGE");
      shape1.push_back("GET DGPS");
      shape1.push_back("GET GPS");
      shape1.push_back("GET LASERS");
      shape1.push_back(turnFirst);
      shape1.push_back("STOP");
   }

   char turnSecond[20];
   sprintf(turnSecond, "TURN -%f", angleSecond);
   char moveSecond[20];
   sprintf(moveSecond, "MOVE %d", len);
   
   for (int j=0; j<num-1; j++)
   {
      shape2.push_back(moveSecond);
      shape2.push_back("STOP");
      //shape2.push_back("GET IMAGE");
      shape2.push_back("GET DGPS");
      shape2.push_back("GET GPS");
      shape2.push_back("GET LASERS");
      shape2.push_back(turnSecond);
      shape2.push_back("STOP");
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
