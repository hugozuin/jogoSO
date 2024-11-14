#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include <locale.h>

#define PORTA 8090
#define TAMANHO_BUFFER 512

int linha, coluna;

int exibirMenu() {
    int escolha;
    printf("\n1. Jogar novamente\n2. Sair\nEscolha uma opção: ");
    while (scanf("%d", &escolha) != 1 || (escolha != 1 && escolha != 2)) {
        printf("Opção inválida. Escolha 1 para jogar novamente ou 2 para sair: ");
        while (getchar() != '\n');
    }
    return escolha;
}

int main() {
    setlocale(LC_ALL, "Portuguese");

    WSADATA winsocketsDados;
    if (WSAStartup(MAKEWORD(2, 2), &winsocketsDados) != 0) {
        perror("Erro ao inicializar o Winsock");
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        perror("Erro ao criar socket do cliente");
        WSACleanup();
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(PORTA);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        perror("Erro ao conectar ao servidor");
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    char sendBuffer[TAMANHO_BUFFER], recvBuffer[TAMANHO_BUFFER];
    int jogando = 1;

    while (jogando) {
        while (1) {
            printf("Vez do jogador 1. Aguardando jogada...\n");

            int bytesRecebidos = recv(clientSocket, recvBuffer, TAMANHO_BUFFER, 0);
            if (bytesRecebidos <= 0) {
                printf("Conexão com o servidor perdida. Fim de Jogo.\n");
                jogando = 0;
                break;
            }
            recvBuffer[bytesRecebidos] = '\0';
            printf("%s", recvBuffer);

            if (strstr(recvBuffer, "Fim de Jogo")) {
                printf("Pressione qualquer tecla para sair...\n");
                getchar();
                break;
            }

            printf("É sua vez, jogador 2! Digite linha e coluna para sua jogada: ");
            while (scanf("%d %d", &linha, &coluna) != 2 || linha < 0 || linha >= 3 || coluna < 0 || coluna >= 3) {
                printf("Coordenadas inválidas. Tente novamente: ");
                while (getchar() != '\n'); 
            }

            snprintf(sendBuffer, TAMANHO_BUFFER, "%d %d", linha, coluna);
            send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
        }

        int escolha = exibirMenu();
        if (escolha == 2) {
            jogando = 0;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}

