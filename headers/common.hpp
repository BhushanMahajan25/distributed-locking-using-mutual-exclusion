/** @file common.hpp
 *  @brief This file contains common headers and functions used by multiple cpp files
*/

#ifndef COMMON_HPP
#define COMMON_HPP

/*client and serve headers*/
#include<iostream>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>
#include<string>
#include<algorithm>
#include<unistd.h>
#include<cmath>
#include<vector>
#include<queue>

using namespace std;

int openFile(ifstream&, string);

void enqueue(int* pClient);
int* dequeue();

#endif