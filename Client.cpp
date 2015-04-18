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
using namespace std;

int main (int argc, char *argv[])
{

   int len = atoi(argv[1]);
   int num = atoi(argv[2]);
   float angle =  M_PI - ((M_PI * (num - 2)) / num);
   
   char turn[20];
   sprintf(turn, "TURN %f", angle);
   char move[20];
   sprintf(move, "MOVE %d", len);
      
   printf("Len: %d, Num: %d, Angle: %f\n", len, num, angle);
   
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
   
return 0;
}
