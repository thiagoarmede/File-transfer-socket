#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Winsock2.h>
#include "../types.h"
#include "../server/server.h"
#include <ctype.h>

int remote_server_socket = 0;
int client_message_length = 0;

unsigned short remote_port = 0;
unsigned char lifeTime = '4';

char remote_ip[32];
struct sockaddr_in remote_address;

struct sockaddr_in next_address;

WSADATA wsa_data;

void MenuCliente(){
    printf("-------------------------------------------------------------------------------\n\n");
    printf("------------------------FILE TRANSFER SOCKET - CLIENTE-------------------------\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("------- buscar - para buscar um arquivo\n");
    printf("------- sair - para voltar ao menu anterior\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("-------------------------------------------------------------------------------\n");

}

char *trimwhitespace(char *str)
{
    char *end;
    while (isspace((unsigned char)*str))
        str++;
    if (*str == 0) // All spaces?
        return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;
    end[1] = '\0';
    return str;
}

void msg_err_client_exit(char *msg)
{
    fprintf(stderr, msg);
    system("PAUSE");
    exit(EXIT_FAILURE);
}

int waitForRequisition(char *fileName) {
    char buffer[sizeof(PositiveAnswer)];
    int blocks;
    int counter = 0;
    printf("Recebimento da resposta.\n");
    FILE *fp;
    do{
        if(counter == 0) {
            fp = fopen(trimwhitespace(fileName), "wb+");
        }
        memset(buffer, 0, sizeof(PositiveAnswer));
        int reqMessage = 0;
        while(recv(remote_server_socket, buffer, sizeof(PositiveAnswer), 0) == SOCKET_ERROR);

        PositiveAnswer *resp = malloc(sizeof(PositiveAnswer));
        memcpy(resp, buffer, sizeof(PositiveAnswer));

        if(resp->type == '2') {
            if(!fp) {
                printf("Arquivo nao aberto...\n");
            }
            if(counter == 0){
                blocks = MyAtoi(resp->fileSize, 4)/1024;
                if (MyAtoi(resp->fileSize, 4) % 1024)
                {
                    blocks++;
                }
            }

            fwrite(resp->dataBlock, 1, 1024 - MyAtoi(resp->padding, 2), fp);
            counter++;
            if((counter) == blocks) {
                fclose(fp);
                printf("\n");
                return 1;
            }
        } else if (resp->type == '3') {
            lifeTime--;
            if(lifeTime <= '0') {
                printf("\n-------- Tempo de vida excedido, fim da aplicacao. ------\n");
                exit(1);
            }
            NegativeAnswer *negResp = malloc(sizeof(NegativeAnswer));
            memcpy(negResp, resp, sizeof(NegativeAnswer));
            if(!negResp->nextIp) {
                printf("Sem proximo IP registrado no servidor.\n");
            }else {
                next_address.sin_family = AF_INET;
                next_address.sin_addr.s_addr = negResp->nextIp;
                next_address.sin_port = htons(SERVER_PORT);
                printf("Arquivo nao presente no servidor, IP do proximo: %s\n", inet_ntoa(next_address.sin_addr));
            }
            return 0;
        }
    }while(1);
}

int searchFile() {
    RequisitionBlock *reqBlock;
    do {
        reqBlock = malloc(sizeof(RequisitionBlock));
        memset(reqBlock, 0, sizeof(*reqBlock));
        printf("Digite o nome do arquivo a ser buscado: \n");
        fgets(reqBlock->fileName, 19, stdin);
        fflush(stdin);

        int foundHere = Search_in_File("cache.txt", trimwhitespace(reqBlock->fileName));
        if(foundHere) {
            printf("Arquivo ja presente na STA. \n");
            return 0;
        }

        MyItoa(reqBlock->serverIp, inet_addr(inet_ntoa(remote_address.sin_addr)), 4);
        MyItoa(reqBlock->clientIp, inet_addr(inet_ntoa(remote_address.sin_addr)), 4);
        reqBlock->lifeTime = lifeTime;
        reqBlock->type = '1';
        unsigned char buffer[32];
        memcpy(buffer, reqBlock, sizeof(RequisitionBlock));

        // envia a mensagem para o servidor
        if (send(remote_server_socket, buffer, sizeof(RequisitionBlock), 0) == SOCKET_ERROR){
            WSACleanup();
            closesocket(remote_server_socket);
            msg_err_client_exit("Falha ao enviar.\n");
        } else {
            printf("Mensagem enviada.\n");
            if(waitForRequisition(reqBlock->fileName)){
                addToCache("cache.txt", reqBlock->fileName);
            }
            return 1;
        }
    }while(strcmp((char *)reqBlock, EXIT_STRING));
}

void client()
{
    do {

        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
            msg_err_client_exit("WSAStartup() failed\n");
        printf("\n///////////////INICIANDO CLIENTE/////////////\n");
        printf("Digite um IP de servidor: ");
        scanf("%s", remote_ip);
        fflush(stdin);
        if(strstr(remote_ip, "sair")){
            break;
        }

        remote_server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (remote_server_socket == INVALID_SOCKET)    {
            WSACleanup();
            msg_err_client_exit("Falha ao comunicar com socket\n");
        }
        // preenchendo o remote_address (servidor)
        memset(&remote_address, 0, sizeof(remote_address));
        remote_address.sin_family = AF_INET;
        remote_address.sin_addr.s_addr = inet_addr(remote_ip);
        remote_address.sin_port = htons(SERVER_PORT);

        printf("conectando ao servidor %s...\n", remote_ip);
        while(connect(remote_server_socket, (struct sockaddr *) &remote_address, sizeof(remote_address)) == SOCKET_ERROR) {
            WSACleanup();
            msg_err_client_exit("Falha ao conectar com o servidor, tentando novamente...\n");
        };

        searchFile();

        WSACleanup();
        closesocket(remote_server_socket);

    }while(1);

    printf("Encerrando modo cliente...\n");
    return;
}
