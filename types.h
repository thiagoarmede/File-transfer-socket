#ifndef TYPES_HEADER
#define TYPES_HEADER
#include <stdio.h>
#include <stdlib.h>
#define EXIT_STRING "#quit"
#define SERVER_PORT 6000
#define BACKLOG_MAX 5

typedef struct {
    int clientIp;
    int serverIp;
    char type;
    short int sequenceNumber;
    int fileSize;
    char dataBlock[1024];
    short int padding;
} PositiveAnswer;

typedef struct
{
    int clientIp;
    int serverIp;
    char type;
    int nextIp;
} NegativeAnswer;

typedef struct
{
    int serverIp;
    int clientIp;
    char type;
    char lifeTime;
    char fileName[20];
} RequisitionBlock;

#endif

