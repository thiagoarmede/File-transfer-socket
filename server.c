#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>
#define BACKLOG_MAX 5
#define EXIT_CALL_STRING "#quit"
#define LOCAL_PORT 6000
    
int local_socket = 0;
int remote_socket = 0;

int remote_length = 0;
int message_length = 0;

struct sockaddr_in local_address;
struct sockaddr_in remote_address;
struct sockaddr_in next_address;

WSADATA wsa_data;

typedef struct {
    int clientIp;
    int serverIp;
    char type;
    short int sequenceNumber;
    int fileSize;
    char datablock[1024];
    short int padding;
}PositiveAnswer;

typedef struct
{
    int clientIp;
    int serverIp;
    char type;
    int nextIp;
}NegativeAnswer;

typedef struct {
    int serverIp;
    int clientIp;
    char type;
    char lifeTime;
    char fileName[20];
}RequisitionBlock;

RequisitionBlock *reqBlock;

#define REQUISITION_BUFFER_SIZE sizeof(RequisitionBlock);

/* Exibe uma mensagem de erro e termina o programa */
void msg_err_exit(char *msg) {
    fprintf(stderr, msg);
    system("PAUSE");
    exit(EXIT_FAILURE);
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
            fread(posAnswer->datablock, sizeof(char), readableSize, file);
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

int main(int argc, char **argv)
{
    // inicia o Winsock 2.0 (DLL), Only for Windows
    if (WSAStartup(MAKEWORD(2, 0), &wsa_data) != 0)
        msg_err_exit("WSAStartup() failed\n");

    // criando o socket local para o servidor
    local_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (local_socket == INVALID_SOCKET){
        WSACleanup();
        msg_err_exit("socket() failed\n");
    }

    memset(&local_address, 0, sizeof(local_address));

    local_address.sin_family = AF_INET;
    local_address.sin_port = htons(LOCAL_PORT);
    local_address.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr("127.0.0.1")

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
        memset(reqBlock, 0, sizeof(RequisitionBlock));
        // recebe o ip do cliente
        message_length = recv(remote_socket, (char *)reqBlock, sizeof(RequisitionBlock), 0);
        if(message_length == SOCKET_ERROR)
            msg_err_exit("Falha no recebimento de nome do arquivo.\n");
        else {
            printf("%s buscou: %s\n", inet_ntoa(remote_address.sin_addr), reqBlock->fileName);
            printf("consultando cache de arquivos...\n");

            int hasFoundWord;
            char *fileEntireText;

            hasFoundWord = Search_in_File("cache.txt", reqBlock->fileName);
            
            if(hasFoundWord) {
                sendFile(reqBlock, local_socket);
            }     
        }
    }while(strcmp(reqBlock->fileName, EXIT_CALL_STRING)); 
 
    printf("Encerrando aplicacao\n");

    WSACleanup();
    closesocket(local_socket);
    closesocket(remote_socket);
 
    system("PAUSE");
    return 0;
    
}