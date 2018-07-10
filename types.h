#ifndef TYPES_HEADER
#define TYPES_HEADER
#include <stdio.h>
#include <stdlib.h>
#define EXIT_STRING "#quit"
#define SERVER_PORT 6000
#define BACKLOG_MAX 5
#define true 1
#define false 0

typedef short int bool;

typedef struct {
    char clientIp[4];
    char serverIp[4];
    char type;
    char sequenceNumber[2];
    char fileSize[4];
    char dataBlock[1024];
    char padding[2];
} PositiveAnswer;

typedef struct {
    char clientIp[4];
    char serverIp[4];
    char type;
    char nextIp[4];
} NegativeAnswer;

typedef struct {
    char serverIp[4];
    char clientIp[4];
    char type;
    char lifeTime;
    char fileName[20];
} RequisitionBlock;

#endif

