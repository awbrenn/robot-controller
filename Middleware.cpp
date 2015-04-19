/*********************************************************
File Name:  httpClient.cpp
Author:     Austin Brennan
Course:     CPSC 3600
Instructor: Sekou Remy
Due Date:   03/25/2015


File Description:
This file contains an implementation of a simple
http client. See readme.txt for more details.

*********************************************************/


#include "Header.h"
using namespace std;


/* function declarations */
void checkFlag(int index, char *argv[]);
void parseURL(string URL);
void getHTTPResponse(int);
void buildRequestHeader();
void sendRobotResponseToClient();
void setupRobotConnection();


/* global variable declarations */
unsigned short ROBOT_PORT = 8083; 		// default robot port value
unsigned short PROXY_PORT = 8000;       // default proxy port value
string SERVER_NAME("");
string FILE_PATH("/");
string OUTPUT_FILENAME("");
string HTTP_RESPONSE("");
string REQUEST_MESSAGE("GET ");
int ROBOT_SOCKET;
int PROXY_SOCKET;
struct sockaddr_in ROBOT_ADDR; /* Robot address */
struct sockaddr_in PROXY_ADDR; /* Local address */


int main (int argc, char *argv[]) {
    
    /* check for valid command line arguments */
    if (argc != 2)
    stateProperUsageAndDie(0);


    /* convert the URL to a string (to make parsing easier) */
   	string URL(argv[1]);
	parseURL(URL); // then parse it

    /*build the header of the GET request*/
    buildRequestHeader();

    /*set up a TCP connection to the robot*/
    setupRobotConnection();

    /* Send the string to the robot */
    if (send(ROBOT_SOCKET, REQUEST_MESSAGE.c_str(), REQUEST_MESSAGE.length(), 0) != (int)REQUEST_MESSAGE.length())
        dieWithError((char *)"send() sent a different number of bytes than expected");

    getHTTPResponse(ROBOT_SOCKET); // get a response from the robot
    close(ROBOT_SOCKET); // close the connection with the robot

    sendRobotResponseToClient();

    return 0;
}

// void initializeProxyServer() {
//     /* Create socket for sending/receiving datagrams */
//     if ((PROXY_SOCK = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
//         dieWithError((char *)"socket() failed");

//     /* Construct local address structure */
//     memset(&PROXY_ADDR, 0, sizeof(PROXY_ADDR));   /* Zero out structure */
//     PROXY_ADDR.sin_family = AF_INET;                /* Internet address family */
//     PROXY_ADDR.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
//     PROXY_ADDR.sin_port = htons(serverPort);      /* Local port */

//     /* Bind to the local address */
//     if (bind(sock, (struct sockaddr *) &PROXY_ADDR, sizeof(PROXY_ADDR)) < 0)
//         dieWithError((char *)"bind() failed");
// }


void setupRobotConnection() {
    struct hostent *thehost; /* Hostent from gethostbyname() */
   
    /* Create a TCP socket for communication with the robot */
    if ((ROBOT_SOCKET = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        dieWithError((char *)"socket() failed");

    /* Construct the server address structure */
    memset(&ROBOT_ADDR, 0, sizeof(ROBOT_ADDR));    /* Zero out structure */
    ROBOT_ADDR.sin_family = AF_INET;                 /* Internet addr family */
    ROBOT_ADDR.sin_addr.s_addr = inet_addr(SERVER_NAME.c_str()); /* Server IP address */
    ROBOT_ADDR.sin_port = htons(ROBOT_PORT);     /* Server port */

    /* If user gave a non dotted decimal address, we need to resolve it  */
    if ((int)ROBOT_ADDR.sin_addr.s_addr == -1) {
        thehost = gethostbyname(SERVER_NAME.c_str());
            ROBOT_ADDR.sin_addr.s_addr = *((unsigned long *) thehost->h_addr_list[0]);
    }

    /* Establish the connection to the robot */
    if (connect(ROBOT_SOCKET, (struct sockaddr *) &ROBOT_ADDR, sizeof(ROBOT_ADDR)) < 0)
        dieWithError((char *)"connect() failed");
}


// void setupClientConnection() {
// }


void parseURL(string URL) {
	size_t serverName_endIndex;

	// basic checks for valid URL
   	if (URL.length() < HTTP_URL_SECTION + 1) // check if the url is long enough
	    dieWithError((char *)"parseURL() failed: URL is too short");
	else if (URL.substr(0, HTTP_URL_SECTION).compare("http://") != 0) // check if the url starts with "http://"
	    dieWithError((char *)"parseURL() failed: URL does not start with \"http://\"");

    serverName_endIndex = URL.find_first_of("/", HTTP_URL_SECTION);

    if (serverName_endIndex == URL.npos) { // no path 
        SERVER_NAME = URL.substr(HTTP_URL_SECTION, URL.length() - HTTP_URL_SECTION);
    }
    else {
        SERVER_NAME = URL.substr(HTTP_URL_SECTION, serverName_endIndex - HTTP_URL_SECTION);
        FILE_PATH += URL.substr(serverName_endIndex + 1, URL.length() - serverName_endIndex);
    }
}


void buildRequestHeader() {
    REQUEST_MESSAGE += FILE_PATH;
    REQUEST_MESSAGE += " HTTP/1.1\r\n";
    REQUEST_MESSAGE += "Host: " + SERVER_NAME + "\r\n";
    REQUEST_MESSAGE += "Connection: close\r\n";
    REQUEST_MESSAGE += "\r\n";
}


void getHTTPResponse(int ROBOT_SOCKET) {
    char httpRequestBuffer[RECV_BUFF_SIZE];
    int messageLen;
    int start_of_content;

    if ((messageLen = read(ROBOT_SOCKET, httpRequestBuffer, RECV_BUFF_SIZE)) < 0)
        dieWithError((char *)"recv() failed");
    
    cout << messageLen << endl;

    /* build the HTTP_RESPONSE as a char vector */
    for (int i = 0; i < messageLen; ++i) {
        HTTP_RESPONSE += httpRequestBuffer[i];
    }

    content_length = getContentLength();
    start_of_content = getStartOfContent();

    cout << "Content-Length: "
}


int getContentLength() {
    char buffer[]
    int start_of_content_length;
    int end_of_content_length;

    if (start_of_content_length = HTTP_RESPONSE.find_first_of("Content-Length: ") == HTTP_RESPONSE.npox)
        dieWithError((char *)"getHTTPResponse(): no content length specified in response");
    
    end_of_content_length = HTTP_RESPONSE.find_first_of("");

}


int getStartOfContent() {
    return HTTP_RESPONSE.find_first_of("\n\r\c");
}


/*********************************
TODO:
change this function to send the robots
response to the client instead of just
printing it out.
**********************************/
void sendRobotResponseToClient() {
    //fprintf(stdout, "%s", HTTP_RESPONSE.c_str());
    cout << HTTP_RESPONSE << endl;
}
