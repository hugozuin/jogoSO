#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <locale.h>

#define PORTA 8090
#define TAMANHO_BUFFER 512


char jogo[3][3];
int l, c, i, j;

void inicializarMatriz() {
    for (l = 0; l < 3; l++) {
        for (c = 0; c < 3; c++) {
            jogo[l][c] = ' ';
        }
    }
}

void imprimir() {
    printf("\n\n\t 0   1   2\n\n");
    for (l = 0; l < 3; l++) {
        printf("\t");
        for (c = 0; c < 3; c++) {
            printf(" %c ", jogo[l][c]);
            if (c < 2) printf("|");
        }
        printf("   %d\n", l);
        if (l < 2) printf("\t------------\n");
    }
}

int ganhouPorLinha(int l, char c) { return jogo[l][0] == c && jogo[l][1] == c && jogo[l][2] == c; }
int ganhouPorLinhas(char c) { for (l = 0; l < 3; l++) if (ganhouPorLinha(l, c)) return 1; return 0; }
int ganhouPorColuna(int c, char j) { return jogo[0][c] == j && jogo[1][c] == j && jogo[2][c] == j; }
int ganhouPorColunas(char j) { for (c = 0; c < 3; c++) if (ganhouPorColuna(c, j)) return 1; return 0; }
int ganhouPorDiagonalPrin(char c) { return jogo[0][0] == c && jogo[1][1] == c && jogo[2][2] == c; }
int ganhouPorDiagonalSecun(char c) { return jogo[0][2] == c && jogo[1][1] == c && jogo[2][0] == c; }

int ehValida(int l, int c) {
    return l >= 0 && l < 3 && c >= 0 && c < 3 && jogo[l][c] == ' ';
}

void enviarTabuleiro(char *buffer) {
    buffer[0] = '\0';
    strcat(buffer, "\n\n\t 0   1   2\n\n");
    for (i = 0; i < 3; i++) {
        strcat(buffer, "\t");
        for (j = 0; j < 3; j++) {
            char linha[20];
            sprintf(linha, " %c ", jogo[i][j]);
            strcat(buffer, linha);
            if (j < 2) strcat(buffer, "|");
        }
        char linha[20];
        sprintf(linha, "   %d\n", i);
        strcat(buffer, linha);
        if (i < 2) strcat(buffer, "\t------------\n");
    }
}

int quantVazias() {
    int quantidade = 0;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            if (jogo[i][j] == ' ') quantidade++;
    return quantidade;
}

int exibirMenu() {
    int escolha;
    printf("\n1. Jogar novamente\n2. Sair\nEscolha uma op��o: ");
    while (scanf("%d", &escolha) != 1 || (escolha != 1 && escolha != 2)) {
        printf("Op��o inv�lida. Escolha 1 para jogar novamente ou 2 para sair: ");
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

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        perror("Erro ao criar socket do servidor");
        WSACleanup();
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORTA);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        perror("Erro no bind");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        perror("Erro no listen");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Aguardando jogador...\n");
    SOCKET clientSocket;
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

    if (clientSocket == INVALID_SOCKET) {
        perror("Erro ao aceitar conex�o do cliente");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    int jogando = 1;
    while (jogando) {
        inicializarMatriz();
        int vez = 1, linha, coluna;
        char recvBuffer[TAMANHO_BUFFER], sendBuffer[TAMANHO_BUFFER], jogador;
        int jogoAtivo = 1;

        while (jogoAtivo) {
            jogador = (vez == 1) ? 'X' : 'O';
            imprimir();

            if (vez == 1) {
                printf("Digite linha e coluna para o jogador 1: ");
                while (scanf("%d %d", &linha, &coluna) != 2 || !ehValida(linha, coluna)) {
                    printf("Coordenadas inv�lidas. Tente novamente: ");
                    while (getchar() != '\n'); 
                }
            } else {
                printf("Aguardando jogada do jogador 2...\n");
                int bytesRecebidos = recv(clientSocket, recvBuffer, TAMANHO_BUFFER, 0);
                if (bytesRecebidos <= 0) {
                    printf("Jogador 2 se desconectou. Fim de Jogo.\n");
                    jogoAtivo = 0;
                    break;
                }
                recvBuffer[bytesRecebidos] = '\0';
                sscanf(recvBuffer, "%d %d", &linha, &coluna);
                if (!ehValida(linha, coluna)) {
                    printf("Jogada inv�lida recebida do jogador 2. Fim de Jogo.\n");
                    jogoAtivo = 0;
                    break;
                }
            }

            jogo[linha][coluna] = jogador;
            int ganhou = ganhouPorLinhas(jogador) || ganhouPorColunas(jogador) || ganhouPorDiagonalPrin(jogador) || ganhouPorDiagonalSecun(jogador);
            int empate = quantVazias() == 0 && !ganhou;

            enviarTabuleiro(sendBuffer);
            if (ganhou) {
                strcat(sendBuffer, vez == 1 ? "\nJogador 1 venceu! Fim de Jogo.\n" : "\nJogador 2 venceu! Fim de Jogo.\n");
                printf("%s", sendBuffer);
                jogoAtivo = 0;
            } else if (empate) {
                strcat(sendBuffer, "\nEmpate! Fim de Jogo.\n");
                printf("%s", sendBuffer);
                jogoAtivo = 0;
            } else {
                strcat(sendBuffer, "\nContinuar\n");
            }

            send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
            vez = 3 - vez;
        }

        int escolha = exibirMenu();
        if (escolha == 2) {
            jogando = 0;
        }
    }

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    printf("Fim de Jogo! Pressione qualquer tecla para sair...\n");
    getchar();
    return 0;
}

