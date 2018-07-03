#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Winsock2.h>
#include "client\client.h"
#include "server\server.h"

void menu(){
    printf("-------------------------------------------------------------------------------\n\n");
    printf("-----------------------------FILE TRANSFER SOCKET------------------------------\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("-----  cliente - inicia modo cliente da aplicacao.\n");
    printf("-----  servidor - inicia modo servidor da aplicacao.\n");
    printf("-----  proximoip - para adicionar proximo ip de busca de arquivos.\n");
    printf("-----  addarquivo - para adicionar um arquivo ao cache.\n");
    printf("-----  sair - para voltar ao menu anterior.\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("-------------------------------------------------------------------------------\n\n");
    printf("-------------------------------------------------------------------------------\n");
}


void addToNextIp(char *nextIp) {
    FILE *f = fopen("nextip.txt", "w");
    if(f != NULL){
        fprintf(f, "%s", nextIp);
        printf("Ip do próximo servidor adicionado!\n");    
    }
    else printf("Erro ao abrir cache.\n");
    fclose(f);
}

int main(int argc, char **argv){
    char command[20], nome[21], nextIp[25];
    int n = 0;
    menu();
    do{
        printf("\n\n\n -->");
        fgets(command, 19, stdin);
        if(strstr(command, "cliente")) client();
        else if(strstr(command, "addarquivo")){
            printf("Nome do arquivo: ");
            fgets(nome, 20, stdin);
            addToCache("cache.txt", nome);
        }
        else if(strstr(command, "servidor")){
            server();
        }else if(strstr(command, "proximoip")){
            printf("IP do proximo servidor: ");
            fgets(nextIp, 24, stdin);
            addToNextIp(nextIp);
        }
        else if(strstr(command, "sair")){
            break;
        }
        else{            
            system("cls");
            menu();
        }
        printf("\n\n\n -->");
    }while(1);
    printf("Encerrando Station ...\n");
    return 0;
}