#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include <locale.h>

#define PORTA 51171
#define TAMANHO_BUFFER 512

int main() {
    setlocale(LC_ALL, "Portuguese");
    
    WSADATA winsocketsDados;
    WSAStartup(MAKEWORD(2, 2), &winsocketsDados);

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(PORTA);

    connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    char sendBuffer[TAMANHO_BUFFER], recvBuffer[TAMANHO_BUFFER];
    int linha, coluna;

    while (1) {
        // Exibe uma mensagem informando que está aguardando a jogada do servidor
        printf("Vez do jogador 1. Aguardando jogada...\n");

        // Receber e imprimir o tabuleiro atualizado
        int bytesRecebidos = recv(clientSocket, recvBuffer, TAMANHO_BUFFER, 0);
        recvBuffer[bytesRecebidos] = '\0';
        printf("%s", recvBuffer);

        // Verifica se houve vitória ou empate
        if (strstr(recvBuffer, "Jogador 1 venceu!") || strstr(recvBuffer, "Jogador 2 venceu!") || strstr(recvBuffer, "Empate")) {
            printf("Fim de Jogo! Pressione qualquer tecla para sair...\n");
            getchar();  // Espera para evitar fechamento imediato
            break;
        }

        // Exibe uma mensagem informando que é a vez do jogador
        printf("É sua vez, jogador 2! Digite linha e coluna para sua jogada: ");
        scanf("%d %d", &linha, &coluna);

        // Envia a jogada do cliente
        snprintf(sendBuffer, TAMANHO_BUFFER, "%d %d", linha, coluna);
        send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
