#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Winsock2.h>
#include "../types.h"
#include <unistd.h>
    
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

int getNextIp() {
    
}

int sendFile(RequisitionBlock *fileRequisition, SOCKET socket){
    FILE *file = fopen(fileRequisition->fileName, "r");

    if(file == NULL) {
        printf("Erro ao buscar arquivo, enviando mensagem negativa.\n");
        if(fileRequisition->lifeTime == '0') {
            printf("Tempo de vida zerado, fim da requisição.");
            fclose(file);
            return 0;
        }
        NegativeAnswer *negAnswer;
        negAnswer->clientIp = inet_addr(inet_ntoa(remote_address.sin_addr));
        negAnswer->serverIp = inet_addr(inet_ntoa(local_address.sin_addr));
        negAnswer->type = '3';

        negAnswer->nextIp = inet_addr(inet_ntoa(next_address.sin_addr));

        send(socket, (char *)negAnswer, sizeof(NegativeAnswer), 0);
        fclose(file);
        return 0;
    } else {
        PositiveAnswer *posAnswer;

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

            printf("Enviando bloco... \n");
            fread(posAnswer->dataBlock, sizeof(char), readableSize, file);
            posAnswer->clientIp = inet_addr(inet_ntoa(remote_address.sin_addr));
            posAnswer->serverIp = inet_addr(inet_ntoa(local_address.sin_addr));
            posAnswer->type = '2';
            posAnswer->sequenceNumber = i+1;

            send(socket, (char *)posAnswer, sizeof(PositiveAnswer), 0);
            remainingSize -= 1024;
        }

        printf("Arquivo completamente enviado!");
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

    if (find_result == 0){
        printf("\nArquivo não presente na STA.\n");
    }

    if (fp) {
        fclose(fp);
    }
    return (0);
}

void server()
{
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

    printf("aguardando alguma conexao...\n");
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
        printf("%s", buffer);
        // reqBlock = malloc(sizeof(RequisitionBlock));
        reqBlock = malloc(sizeof(RequisitionBlock));
        memcpy(reqBlock, buffer, sizeof(RequisitionBlock));
        
        // {
        //     printf("Erro: %i\n", WSAGetLastError());
        //     printf("erro no socket.");
        //     continue;
        // }

            // if(message_length == SOCKET_ERROR) {
            //     printf("%i", remote_socket);
            //     // msg_err_exit("Falha no recebimento de nome do arquivo.\n");
            //     continue;
            // }
        printf("%i %s %c", reqBlock->clientIp, reqBlock->fileName, reqBlock->type);

        printf("\n%s buscou: %s\n", inet_ntoa(remote_address.sin_addr), reqBlock->fileName);
        printf("consultando cache de arquivos...\n");

        int hasFoundWord;
        char *fileEntireText;

        hasFoundWord = Search_in_File("cache.txt", reqBlock->fileName);
        printf("Encontrou palavra: %i\n", hasFoundWord);

        if(hasFoundWord) {
            sendFile(reqBlock, local_socket);
        }     
    
    }while(!strstr(reqBlock->fileName, EXIT_STRING)); 
 
    printf("Encerrando aplicacao\n");

    WSACleanup();
    closesocket(local_socket);
    closesocket(remote_socket);
 
    system("PAUSE");
    return;
}