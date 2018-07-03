#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Winsock2.h>
#include "../types.h"
#include <unistd.h>
#include <ctype.h>
#include <windows.h>
#include "../client/client.h"

int local_socket = 0;
int remote_socket = 0;

int remote_length = 0;
int message_length = 0;

struct sockaddr_in local_address;
struct sockaddr_in remote_address;
struct sockaddr_in next_address;

WSADATA wsa_data;

RequisitionBlock *reqBlock;

#define REQUISITION_BUFFER_SIZE sizeof(RequisitionBlock);

void msg_err_exit(char *msg) {
    fprintf(stderr, msg);
    system("PAUSE");
    exit(EXIT_FAILURE);
}

int getNextIp(char *nextIp) {
    FILE *fp = fopen("nextip.txt", "r");
    if(!fp) {
        return 0;
    }
    fscanf(fp, "%s", nextIp);
    fclose(fp);

    if(strlen(nextIp)) {
        return 1;
    }
    return 0;
}

void addToCache(char *cache, char *s)
{
    FILE *f;
    f = fopen(cache, "a");
    if (f != NULL)
    {
        fprintf(f, "%s", s);
        printf("Arquivo adicionado ao cache!\n");
    }
    else
        printf("Erro ao abrir cache.\n");
    fclose(f);
}

int sendFile(RequisitionBlock *fileRequisition, SOCKET socket){
    FILE *file = fopen(trimwhitespace(fileRequisition->fileName), "rb+");
    if(file == NULL) {
        printf("Enviando mensagem negativa.\n");
        if(fileRequisition->lifeTime == '0') {
            printf("Tempo de vida zerado, fim da requisição.");
            fclose(file);
            return 0;
        }
        NegativeAnswer *negAnswer = malloc(sizeof(NegativeAnswer));
        char nextIp[20];
        char negBuffer[sizeof(PositiveAnswer)];

        negAnswer->clientIp = inet_addr(inet_ntoa(remote_address.sin_addr));
        negAnswer->serverIp = inet_addr(inet_ntoa(local_address.sin_addr));
        negAnswer->type = '3';
        if(getNextIp(nextIp)) {
            negAnswer->nextIp = inet_addr(nextIp);
        } else {
            negAnswer->nextIp = 0;
        }

        memcpy(negBuffer, negAnswer, sizeof(NegativeAnswer));
        send(socket, negBuffer, sizeof(PositiveAnswer), 0);
        fclose(file);
        free(negAnswer);
        return 0;
    } else {
        PositiveAnswer *posAnswer = malloc(sizeof(PositiveAnswer));

        fseek(file, 0, SEEK_END);
        int remainingSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        int blocks = remainingSize/1024;
        if(remainingSize%1024) {
            blocks++;
        }

        posAnswer->fileSize = remainingSize;

        for(int i=0; i < blocks; i++) {
            int readableSize;

            if(remainingSize >= 1024) {
                posAnswer->padding = 1024 - remainingSize;
                readableSize = 1024;
            } else {
                readableSize = remainingSize;
                posAnswer->padding = 0;
            }

            printf("Enviando bloco... %i\n", i);
            fread(posAnswer->dataBlock, sizeof(char), readableSize, file);
            posAnswer->clientIp = inet_addr(inet_ntoa(remote_address.sin_addr));
            posAnswer->serverIp = inet_addr(inet_ntoa(local_address.sin_addr));
            posAnswer->type = '2';
            posAnswer->sequenceNumber = i+1;

            send(socket, (char *)posAnswer, sizeof(PositiveAnswer), 0);
            remainingSize -= 1024;
            Sleep(100);
        }
        fclose(file);

        return 1;
    }
}

int Search_in_File(char *fname, char *str){
    FILE *fp;
    int line_num = 1;
    int find_result = 0;
    char temp[512];

    if((fp = fopen(fname, "r+")) == NULL) {
    	return(-1);
    }

    while (fgets(temp, 512, fp) != NULL) {
        if ((strstr(temp, str)) != NULL) {
            printf("\n%s\n", temp);
            find_result++;
        }
        line_num++;
    }
    fclose(fp);

    if (find_result == 0){
        printf("\nArquivo nao presente na STA.\n");
        return 0;
    } else {
        return 1;
    }
}

void server()
{
    printf("\n///////////////INICIANDO SERVIDOR///////////////\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
        msg_err_exit("WSAStartup() failed\n");

    local_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (local_socket == INVALID_SOCKET){
        WSACleanup();
        msg_err_exit("socket() failed\n");
    }

    memset(&local_address, 0, sizeof(local_address));

    local_address.sin_family = AF_INET;
    local_address.sin_port = htons(SERVER_PORT);
    local_address.sin_addr.s_addr = htonl(INADDR_ANY); 

    if (bind(local_socket, (struct sockaddr *) &local_address, sizeof(local_address)) == SOCKET_ERROR){
        WSACleanup();
        closesocket(local_socket);
        msg_err_exit("bind() failed\n");
    }

    if (listen(local_socket, BACKLOG_MAX) == SOCKET_ERROR){
        WSACleanup();
        closesocket(local_socket);
        msg_err_exit("listen() failed\n");
    }

    remote_length = sizeof(remote_address);

    printf("Aguardando alguma conexao...\n");
    remote_socket = accept(local_socket, (struct sockaddr *) &remote_address, &remote_length);

    if(remote_socket == INVALID_SOCKET){
        WSACleanup();
        closesocket(local_socket);
        msg_err_exit("accept() failed\n");
    }

    printf("Conexao estabelecida com %s\n", inet_ntoa(remote_address.sin_addr));
    printf("aguardando busca de arquivo...\n");
    
    do{ 
        unsigned char buffer[32];
        // recebe o ip do cliente
        message_length = recv(remote_socket, buffer, sizeof(RequisitionBlock), 0) == SOCKET_ERROR;
        if(message_length){
            printf("erro.. %i\n", WSAGetLastError());
        }
        reqBlock = malloc(sizeof(RequisitionBlock));
        memcpy(reqBlock, buffer, sizeof(RequisitionBlock));
        
        printf("\n%s buscou: %s\n", inet_ntoa(remote_address.sin_addr), reqBlock->fileName);
        printf("consultando cache de arquivos...\n");

        int hasFoundWord;
        char *fileEntireText;
    
        hasFoundWord = Search_in_File("cache.txt", reqBlock->fileName);

        if(!hasFoundWord) {
            printf("Arquivo não encontrado. \n");
            sendFile(reqBlock, remote_socket);
            break;
        } else {
            sendFile(reqBlock, remote_socket);
            printf("Arquivo enviado ao cliente!\n");
            break;
        }    
    }while(!strstr(reqBlock->fileName, EXIT_STRING)); 
 
    printf("Encerrando aplicacao\n");

    WSACleanup();
    closesocket(local_socket);
    closesocket(remote_socket);
 
    system("PAUSE");
    return;
}