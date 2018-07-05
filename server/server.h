#ifndef SERVER_HEADER
#define SERVER_HEADER
#include <stdio.h>
#include <stdlib.h>
#include <Winsock2.h>
#include "../types.h"

void server();
int sendFile(RequisitionBlock *, SOCKET);
int Search_in_File(char *, char *);
void addToCache(char *cache, char *s);

//conversão
int MyAtoi(char *s, int tam);
int MyPow(int x, int y);
void MyItoa(char *, int v, int tam);

#endif