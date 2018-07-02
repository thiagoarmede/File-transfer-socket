#ifndef SERVER_HEADER
#define SERVER_HEADER
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include "../types.h"

void server();
int sendFile(RequisitionBlock *, SOCKET);
int Search_in_File(char *, char *);

#endif