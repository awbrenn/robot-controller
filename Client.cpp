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
string addRequestHeaderToCommand(string);
void recieveAckFromMiddleware();
string PROXY_RESPONSE("");
void writeDataToFile(string command);
void writeFile(char * name, int file_num, char * ext) ;

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
uint32_t REQUEST_ID = 0; 
string ROBOT_ID = "town2";
float angleFirst, angleSecond;
int GPS_FILE_NUM = 0;
int DGPS_FILE_NUM = 0;
int LASERS_FILE_NUM = 0;
int IMAGE_FILE_NUM = 0;


int main (int argc, char *argv[])
{

   ROBOT_ID = argv[3];
   int len = atoi(argv[4]);
   int num = atoi(argv[5]);
   port = atoi(argv[2]);
   ip = argv[1];
   double moveSpeed = 0.2;
   double turnSpeed = 0.2;
   double moveSleep, turnSleep;
   size_t found;
   
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
//   shape1 = addHeaderToCommands(shape1);
//   shape2 = addHeaderToCommands(shape2);
   
   printf("Len: %d, Num: %d, AngleFirst: %f, AngleSecond: %f\n", len, 
      num, angleFirst, angleSecond);
      
   //Print content of vector
   vector<string>::iterator it;
   // for (it = v.begin(); it != v.end(); it++)
   //    cout << *it << endl;

   srand(time(NULL));
   int i = 0;
   //Run loop until all of shape1 has been sent 
   for (it = shape1.begin(); it != shape1.end(); it++)
   {
      memset (recvBuffer, 0, sizeof(recvBuffer));
      clntAddrLen = sizeof(clntAddr);
      
      REQUEST_ID = rand() % MAX_RAND_NUMBER;

      cout << "Request ID from client: " << REQUEST_ID << endl << endl;

      shape1[i] = addRequestHeaderToCommand(shape1[i]);

      //Send command
      if (sendto(sock, shape1.at(i).c_str(), shape1.at(i).length(), 0, 
         (struct sockaddr *) &servAddr, sizeof(servAddr)) != shape1.at(i).length())
	      cout << "Error on sendto" << endl; 

      alarm(TIMEOUT_SECONDS);

      //Recv ack
      recieveAckFromMiddleware();
      writeDataToFile(shape1[i]);

      cout << PROXY_RESPONSE <<  endl << endl;


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

   // test

  // string get_lasers("GET LASERS");

  // if (sendto(sock, get_lasers.c_str(), get_lasers.length(), 0, 
  //       (struct sockaddr *) &servAddr, sizeof(servAddr)) != get_lasers.length())
  //       cout << "Error on sendto" << endl;

  // //Recv ack
  // recieveAckFromMiddleware();

  // cout << PROXY_RESPONSE << endl;
   
   i = 0;
   //Run loop until all of shape2 has been sent 
   for (it = shape2.begin(); it != shape2.end(); it++)
   {
      memset (recvBuffer, 0, sizeof(recvBuffer));
      clntAddrLen = sizeof(clntAddr);

      REQUEST_ID = rand() % MAX_RAND_NUMBER;
      
      shape2[i] = addRequestHeaderToCommand(shape2[i]);

      //Send command
      if (sendto(sock, shape2.at(i).c_str(), shape2.at(i).length(), 0, 
         (struct sockaddr *) &servAddr, sizeof(servAddr)) != shape2.at(i).length())
	      cout << "Error on sendto" << endl; 
         
      alarm(TIMEOUT_SECONDS);

      //Recv ack
      recieveAckFromMiddleware();
      writeDataToFile(shape2[i]);


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
   
   return 0;
}


string addRequestHeaderToCommand(string command) {
  int body_length = command.length();

  char * body = (char *) calloc(body_length + 4, sizeof(char));
  memcpy(body, &REQUEST_ID, 4);
  memcpy(body + 4, command.data(), body_length);
  command.assign(body, body_length + 4);

  return command;
}


void catchAlarm(int ignored) { return; }


void createVector(int len, int num)
{
   angleFirst =  M_PI - ((M_PI * (num - 2)) / num);
   angleSecond = M_PI - ((M_PI * (num - 3)) / (num - 1));
   char turnFirst[20];
   sprintf(turnFirst, "TURN %f%s", angleFirst, "\0");
   char moveFirst[20];
   sprintf(moveFirst, "MOVE %d%s", len, "\0");
 
   for (int i=0; i<num; i++)
   {
      shape1.push_back(moveFirst);
      shape1.push_back("STOP\0");
//      shape1.push_back("GET IMAGE\O");
      shape1.push_back("GET DGPS\0");
      shape1.push_back("GET GPS\0");
      shape1.push_back("GET LASERS\0");
      shape1.push_back(turnFirst);
      shape1.push_back("STOP\0");
   }

   char turnSecond[20];
   sprintf(turnSecond, "TURN -%f%s", angleSecond, "\0");
   char moveSecond[20];
   sprintf(moveSecond, "MOVE %d%s", len, "\0");
   
   for (int j=0; j<num-1; j++)
   {
      shape2.push_back(moveSecond);
      shape2.push_back("STOP\0");
//      shape2.push_back("GET IMAGE\O");
      shape2.push_back("GET DGPS\0");
      shape2.push_back("GET GPS\0");
      shape2.push_back("GET LASERS\0");
      shape2.push_back(turnSecond);
      shape2.push_back("STOP\0");
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
  uint32_t *id = (uint32_t *) malloc(sizeof(uint32_t));
  uint32_t *total_messages = (uint32_t *) malloc(sizeof(uint32_t));
  uint32_t *sequence_number = (uint32_t *) malloc(sizeof(uint32_t));
  vector<string> fragmented_response;
  int recvMsgSize;

  cout << "\nGOT HERE\n" << endl;

  clntAddrLen = sizeof(clntAddr);

  do {
    string response_chunk;

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

    cout << "\nmessageLen: " << recvMsgSize << endl;
    memcpy(id, recvBuffer, 4);
    memcpy(total_messages, recvBuffer + 4, 4);
    memcpy(sequence_number, recvBuffer + 8, 4);

    cout << "\nMessage ID: " << *id << endl;
    cout << "\nTotal Messages: " << *total_messages << endl;
    cout << "\nSequence Number: " << *sequence_number << endl;

    response_chunk.assign(recvBuffer + 12, recvMsgSize - 11);
    fragmented_response.push_back(response_chunk);
    //printf("%s\n\n", recvBuffer + 12);
 } while (*sequence_number < (*total_messages - 1));

  alarm(0);

  PROXY_RESPONSE = "";

  for (int i = 0; i < (int)fragmented_response.size(); i++) {
        PROXY_RESPONSE += fragmented_response.at(i);
  }


}

void writeDataToFile(string command) {
    if (command.find("GET GPS") != string::npos)
      writeFile("gps", GPS_FILE_NUM++, "txt");
    else if (command.find("GET DGPS") != string::npos)
      writeFile("dgps", DGPS_FILE_NUM++, "txt");
    else if (command.find("GET LASERS") != string::npos)
      writeFile("lasers", LASERS_FILE_NUM++, "txt");
    else if (command.find("GET IMAGE") != string::npos)
      writeFile("image", IMAGE_FILE_NUM++, "png");
}

void writeFile(char * name, int file_num, char * ext) {
  FILE *file;
  char buffer[20];
  bzero(buffer, 20);
  sprintf(buffer,"%s-%d.%s",name,file_num,ext);
  file = fopen(buffer, "wb+");
  fputs(PROXY_RESPONSE.c_str(), file);
  fclose(file);
}
