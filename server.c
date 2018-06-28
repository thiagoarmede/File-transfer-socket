#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>
#define BACKLOG_MAX 5
#define BUFFER_SIZE 128
#define EXIT_CALL_STRING "#quit"
#define LOCAL_PORT 6000
    
int local_socket = 0;
int remote_socket = 0;

int remote_length = 0;
int message_length = 0;

unsigned short local_port = 0;
unsigned short remote_port = 0;

char fileName[BUFFER_SIZE];

struct sockaddr_in local_address;
struct sockaddr_in remote_address;

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

/* Exibe uma mensagem de erro e termina o programa */
void msg_err_exit(char *msg)
{
    fprintf(stderr, msg);
    system("PAUSE");
    exit(EXIT_FAILURE);
}

void sendFile(char *fileName) {
    
}

int main(int argc, char **argv)
{
    // inicia o Winsock 2.0 (DLL), Only for Windows
    if (WSAStartup(MAKEWORD(2, 0), &wsa_data) != 0)
        msg_err_exit("WSAStartup() failed\n");

    // criando o socket local para o servidor
    local_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (local_socket == INVALID_SOCKET)
    {
        WSACleanup();
        msg_err_exit("socket() failed\n");
    }

    memset(&local_address, 0, sizeof(local_address));

    local_address.sin_family = AF_INET;
    local_address.sin_port = htons(LOCAL_PORT);
    local_address.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr("127.0.0.1")

    if (bind(local_socket, (struct sockaddr *) &local_address, sizeof(local_address)) == SOCKET_ERROR)
    {
        WSACleanup();
        closesocket(local_socket);
        msg_err_exit("bind() failed\n");
    }

    if (listen(local_socket, BACKLOG_MAX) == SOCKET_ERROR)
    {
        WSACleanup();
        closesocket(local_socket);
        msg_err_exit("listen() failed\n");
    }

    remote_length = sizeof(remote_address);

    printf("aguardando alguma conexao...\n");
    remote_socket = accept(local_socket, (struct sockaddr *) &remote_address, &remote_length);

    if(remote_socket == INVALID_SOCKET)
    {
        WSACleanup();
        closesocket(local_socket);
        msg_err_exit("accept() failed\n");
    }

    printf("Conexao estabelecida com %s\n", inet_ntoa(remote_address.sin_addr));
    printf("aguardando busca de arquivo...\n");
    
    do
    {  
        memset(&fileName, 0, BUFFER_SIZE);
        // recebe o ip do cliente
        message_length = recv(remote_socket, fileName, BUFFER_SIZE, 0);
        if(message_length == SOCKET_ERROR)
            msg_err_exit("Falha no recebimento de nome do arquivo.\n");
        else {
            printf("%s buscou: %s\n", inet_ntoa(remote_address.sin_addr), fileName);
            printf("consultando cache de arquivos...\n");

            FILE *fd = fopen("cache.txt", "r+");
            char *foundFileText;

            if(fd == NULL) {
                printf("Erro ao abrir arquivo.");
                exit(1);
            }

            while(!feof(fd)) {
                fgets(foundFileText, strlen(fileName), fd);

                if(!strcmp(fileName, foundFileText)) {
                    printf("Arquivo presente na STA, iniciando processo de envio...");
                    break;
                }

            }
           
        }
    }
    while(strcmp(fileName, EXIT_CALL_STRING)); 
    // sai quando receber um "#quit" do cliente
 
    printf("Encerrando aplicacao\n");

    WSACleanup();
    closesocket(local_socket);
    closesocket(remote_socket);
 
    system("PAUSE");
    return 0;
}