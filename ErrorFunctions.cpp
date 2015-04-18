/*********************************************************
File Name:  httpClient.cpp
Author:     Austin Brennan, Akshit Gupta, & Connor Pickard
Course:     CPSC 3600
Instructor: Sekou Remy
Due Date:   04/24/2015


File Description:
Ths file contains a routine called dieWithError.
The routine prints an error message and exits the program.

*********************************************************/

#include <stdio.h>  /* for perror() */
#include <stdlib.h> /* for exit() */
#include <string.h>

/*  DieWithError
    input        - takes an error message
    output       - none
    description: - this routine prints an error message
                    and exits the program.
*/
extern void dieWithError(char *error_message) {
    fprintf(stderr, "ERROR\t%s\n", error_message);
    exit(1);
}


/*  stateProperUsageAndDie
    input        - program id (0 is the middleware and 1 is the client)
    output       - none
    description: - states the proper useage and calls dieWithError
*/
void stateProperUsageAndDie(int program_id) {
    if (program_id == 0) // middleware
        dieWithError((char *)"Improper Usage - See proper usage below\n"
                             "$> <path_to_executable> <port number>\n");
    else if (program_id == 1) // client
        dieWithError((char *)"Improper Usage - See proper usage below\n"
                             "$> <path_to_executable> <ip of middleware> <port of middleware>"
                             " <ip of robot> <id of robot> <length> <number of"
                             "edges>");
    else
        dieWithError((char *)"Improper Usage");
}
