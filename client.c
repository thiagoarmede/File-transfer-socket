#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>
        
#define BUFFER_SIZE 128
#define EXIT_CALL_STRING "#quit"
#define SERVER_PORT 6000

int remote_socket = 0;
int message_length = 0;

unsigned short remote_port = 0;

char remote_ip[32];
struct sockaddr_in remote_address;

typedef struct
{
    int clientIp;
    int serverIp;
    char type;
    short int sequenceNumber;
    int fileSize;
    char datablock[1024];
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

WSADATA wsa_data;

void msg_err_exit(char *msg)
{
    fprintf(stderr, msg);
    system("PAUSE");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if (WSAStartup(MAKEWORD(2, 0), &wsa_data) != 0)
        msg_err_exit("WSAStartup() failed\n");

    printf("IP do servidor: ");
    scanf("%s", remote_ip);
    fflush(stdin);

    remote_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (remote_socket == INVALID_SOCKET)    {
        WSACleanup();
        msg_err_exit("socket() failed\n");
    }

    // preenchendo o remote_address (servidor)
    memset(&remote_address, 0, sizeof(remote_address));
    remote_address.sin_family = AF_INET;
    remote_address.sin_addr.s_addr = inet_addr(remote_ip);
    remote_address.sin_port = htons(SERVER_PORT);

    printf("conectando ao servidor %s...\n", remote_ip);
    if (connect(remote_socket, (struct sockaddr *) &remote_address, sizeof(remote_address)) == SOCKET_ERROR){
        WSACleanup();
        msg_err_exit("connect() failed\n");
    }

    RequisitionBlock *reqBlock;
    do
    {
        memset(reqBlock, 0, sizeof(reqBlock));

        printf("Digite o nome do arquivo a ser buscado: \n");
        gets(reqBlock->fileName);
        fflush(stdin);
        // envia a mensagem para o servidor
        if (send(remote_socket, (char *)reqBlock, message_length, 0) == SOCKET_ERROR){
            WSACleanup();
            closesocket(remote_socket);
            msg_err_exit("send() failed\n");
        }
    }
    while(strcmp((char *)reqBlock, EXIT_CALL_STRING));

    printf("encerrando\n");

    WSACleanup();
    closesocket(remote_socket);

    system("PAUSE");
    return 0;
}