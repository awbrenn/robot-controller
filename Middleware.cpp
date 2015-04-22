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
void setupRobotConnection();
void sendRobotResponseToClient();
int getContentLength();
int getStartOfContent();
void removeHeaderFromHTTPResponse();
vector<string> fragmentHTTPResponse();
vector<string> addHeaderToResponseToClient(vector<string>);
string convertHeaderInfoToString(uint32_t);
void middlewareMainLoop();
void getCommandFromClient();
void initializeProxyServer();



/* global variable declarations */
unsigned short ROBOT_PORT = 8083; 		// default robot port value
unsigned short PROXY_PORT = 8000;       // default proxy port value
string SERVER_NAME("");
string FILE_PATH("/");
string OUTPUT_FILENAME("");
string HTTP_RESPONSE("");
string REQUEST_MESSAGE("GET ");
string COMMAND("");
int ROBOT_SOCKET;
int PROXY_SOCKET;
struct sockaddr_in ROBOT_ADDR;  /* Robot address */
struct sockaddr_in PROXY_ADDR;  /* Local address */
struct sockaddr_in CLIENT_ADDR; /* Client address */
uint32_t REQUEST_ID = 546;        /* Stubbed for now */


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

    initializeProxyServer();
    middlewareMainLoop();

    return 0;
}


void middlewareMainLoop() {
    while(true) {
        getCommandFromClient();
        getHTTPResponse(ROBOT_SOCKET); // get a response from the robot
        sendRobotResponseToClient();
    }
    close(ROBOT_SOCKET); // close the connection with the robot
}

// void connectToClient() {
//     unsigned int clientLen = sizeof(CLIENT_ADDR);

//     /* waiting for the client to connect */
//     if ((CLIENT_SOCKET = accept(PROXY_SOCKET, (struct sockaddr *) &CLIENT_ADDR, 
//                    &clientLen)) < 0)
//         dieWithError((char *)"accept() failed");
// }


void getCommandFromClient() {
    int messageLen;
    unsigned int clientAddrLen = sizeof(CLIENT_ADDR);
    char clientRequestBuffer[1000];

    if ((messageLen = recvfrom(PROXY_SOCKET, clientRequestBuffer, RECV_BUFF_SIZE, 
        0, (struct sockaddr *) &CLIENT_ADDR, &clientAddrLen)) < 0)
        dieWithError((char *)"recv() failed");

    printf("%s\n", clientRequestBuffer);
}


void initializeProxyServer() {
    /* Create socket for sending/receiving datagrams */
    if ((PROXY_SOCKET = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        dieWithError((char *)"socket() failed");

    /* Construct local address structure */
    memset(&PROXY_ADDR, 0, sizeof(PROXY_ADDR));   /* Zero out structure */
    PROXY_ADDR.sin_family = AF_INET;                /* Internet address family */
    PROXY_ADDR.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    PROXY_ADDR.sin_port = htons(PROXY_PORT);      /* Local port */

    /* Bind to the local address */
    if (bind(PROXY_SOCKET, (struct sockaddr *) &PROXY_ADDR, sizeof(PROXY_ADDR)) < 0)
        dieWithError((char *)"bind() failed");
}


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


/*
    Sends the client the response using
    the UDCP protocol
*/
void sendRobotResponseToClient() {
    cout << HTTP_RESPONSE << endl; // test - remove later
    vector<string> fragmented_response;

    /* first remove the header from the HTTP response */
    removeHeaderFromHTTPResponse();

    /* then fragement the response into 1000 Byte chunks */
    if (HTTP_RESPONSE.length() > UDP_PACKET_MAX_SIZE - 12) // subtract 12 for header
        fragmented_response = fragmentHTTPResponse();
    else // only one UDP packet needed
        fragmented_response.push_back(HTTP_RESPONSE);

    fragmented_response = addHeaderToResponseToClient(fragmented_response);

    cout << HTTP_RESPONSE.length() << endl;

    cout << "\n\nNEW MESSASGE:" << endl;

    int i;
    for (i = 0; i < (int)fragmented_response.size(); i++) {
        uint32_t num = *((uint32_t *)fragmented_response.at(i).c_str());
        printf("%d\n", (int) num);
        cout << (uint32_t)fragmented_response.at(i).at(3) << endl; 
        cout << (uint32_t)fragmented_response.at(i).at(7) << endl;
        cout << (uint32_t)fragmented_response.at(i).at(11) << endl;
        cout << fragmented_response.at(i).length() << endl;
        cout << fragmented_response.at(i) << "\n" << endl;
    }
}

string convertHeaderInfoToString(uint32_t header_info) {
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


vector<string> addHeaderToResponseToClient(vector<string> fragmented_response) {
    uint32_t number_of_messages = fragmented_response.size();
    uint32_t sequence_number;
    string body;
    int i;

    number_of_messages = fragmented_response.size();

    /* add header to each of the fragmented responses */
    for (i = 0; i < (int) number_of_messages; i++) {
        sequence_number = i;
        body = fragmented_response[i];
        body = convertHeaderInfoToString(REQUEST_ID)
             + convertHeaderInfoToString(number_of_messages)
             + convertHeaderInfoToString(sequence_number)
             + body;
        fragmented_response[i] = body;
    }

    return fragmented_response;
}


void removeHeaderFromHTTPResponse() {
    int start_of_content = getStartOfContent();

    /* remove the header form the HTTP_RESPONSER */
    HTTP_RESPONSE = HTTP_RESPONSE.substr(start_of_content, HTTP_RESPONSE.length() - (start_of_content));
}


/* Fragments the response into 1000 Byte chunnks */
vector<string> fragmentHTTPResponse() {
    size_t index;
    size_t size;
    vector<string> fragmented_response;

    /* split the HTTP response into 1000 byte chunks */
    for (index = 0; index < HTTP_RESPONSE.length(); index += UDP_PACKET_MAX_SIZE - 12) {
        /* get the size of the fragment */
        if ((HTTP_RESPONSE.length() - index) > UDP_PACKET_MAX_SIZE - 12)
            size = UDP_PACKET_MAX_SIZE - 12;
        else
            size = HTTP_RESPONSE.length() - index;

        fragmented_response.push_back(HTTP_RESPONSE.substr(index, size));
    }

    return fragmented_response;
}


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

    if ((messageLen = read(ROBOT_SOCKET, httpRequestBuffer, RECV_BUFF_SIZE)) < 0)
        dieWithError((char *)"recv() failed");
    
    cout << messageLen << endl;

    /* build the HTTP_RESPONSE as a char vector */
    for (int i = 0; i < messageLen; ++i) {
        HTTP_RESPONSE += httpRequestBuffer[i];
    }

    int content_length = getContentLength();
    int start_of_content = getStartOfContent();

    cout << "Content Length: " << content_length <<
    "\nStart of Content: " << start_of_content << "\n\n" << endl;
}


int getContentLength() {
    size_t start_of_content_length;
    size_t end_of_content_length;

    /* search the response for "Content-Length" */
    if ((start_of_content_length = HTTP_RESPONSE.find("Content-Length: ")) == HTTP_RESPONSE.npos)
        dieWithError((char *)"getHTTPResponse(): no content length specified in response");

    start_of_content_length += 16; // move start to the number, strlen("Content-Length: ")

    /* search the string for the next carriage return */
    end_of_content_length = HTTP_RESPONSE.find("\r\n", start_of_content_length);

    /* convert the Content-Length section to an int and return to caller */
    return atoi(HTTP_RESPONSE.substr(start_of_content_length, end_of_content_length - start_of_content_length).c_str());
}


int getStartOfContent() {
    return HTTP_RESPONSE.find("\r\n\r\n") + 4; // add 4 to the number, strlen("\r\n\r\n")
}
