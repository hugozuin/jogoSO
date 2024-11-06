#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <locale.h>

#define PORTA 51171
#define TAMANHO_BUFFER 512

// Variáveis globais do jogo
char jogo[3][3];
int l, c;

void inicializarMatriz() {
    for(l = 0; l < 3; l++) {
        for(c = 0; c < 3; c++) {
            jogo[l][c] = ' ';
        }
    }
}

void imprimir() {
    printf("\n\n\t 0   1   2\n\n");
    for(l = 0; l < 3; l++) {
        for(c = 0; c < 3; c++) {
            if(c == 0) printf("\t");
            printf(" %c ", jogo[l][c]);
            if(c < 2) printf("|");
            if(c == 2) printf("   %d", l);
        }
        printf("\n");
        if(l < 2) printf("\t------------\n");
    }
}

// Funções de verificação de vitória
int ganhouPorLinha(int l, char c) { return jogo[l][0] == c && jogo[l][1] == c && jogo[l][2] == c; }
int ganhouPorLinhas(char c) { int ganhou = 0; for(l = 0; l < 3; l++) ganhou += ganhouPorLinha(l, c); return ganhou; }
int ganhouPorColuna(int c, char j) { return jogo[0][c] == j && jogo[1][c] == j && jogo[2][c] == j; }
int ganhouPorColunas(char j) { int ganhou = 0; for(c = 0; c < 3; c++) ganhou += ganhouPorColuna(c, j); return ganhou; }
int ganhouPorDiagonalPrin(char c) { return jogo[0][0] == c && jogo[1][1] == c && jogo[2][2] == c; }
int ganhouPorDiagonalSecun(char c) { return jogo[0][2] == c && jogo[1][1] == c && jogo[2][0] == c; }

// Função para verificar se uma posição é válida
int ehValida(int l, int c) { return l >= 0 && l < 3 && c >= 0 && c < 3 && jogo[l][c] == ' '; }

// Função para criar o estado do tabuleiro como string
void enviarTabuleiro(char *buffer) {
    int i, j;
    char linha[20];
    buffer[0] = '\0';
    strcat(buffer, "\n\n\t 0   1   2\n\n");
    for(i = 0; i < 3; i++) {
        strcat(buffer, "\t");
        for(j = 0; j < 3; j++) {
            sprintf(linha, " %c ", jogo[i][j]);
            strcat(buffer, linha);
            if (j < 2) strcat(buffer, "|");
        }
        sprintf(linha, "   %d\n", i);
        strcat(buffer, linha);
        if (i < 2) strcat(buffer, "\t------------\n");
    }
}

// Função que retorna a quantidade de posições vazias no tabuleiro
int quantVazias() {
    int i, j, quantidade = 0;
    for(i = 0; i < 3; i++) {
        for(j = 0; j < 3; j++) {
            if(jogo[i][j] == ' ') {
                quantidade++;
            }
        }
    }
    return quantidade;
}

int main() {
    setlocale(LC_ALL, "Portuguese");
    
    WSADATA winsocketsDados;
    WSAStartup(MAKEWORD(2, 2), &winsocketsDados);
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORTA);

    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);

    printf("Aguardando jogador...\n");
    SOCKET clientSocket;
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

    inicializarMatriz();
    int vez = 1, linha, coluna, jogando = 1;
    char recvBuffer[TAMANHO_BUFFER], sendBuffer[TAMANHO_BUFFER], jogador;

    while(jogando) {
        jogador = (vez == 1) ? 'X' : 'O';
        
        // Exibir o tabuleiro no servidor
        imprimir();

        if (vez == 1) {
            printf("Digite linha e coluna para o jogador 1: ");
            scanf("%d %d", &linha, &coluna);
        } else {
            // Mensagem informando que está aguardando o jogador 2
            printf("Aguardando jogada do jogador 2...\n");
            recv(clientSocket, recvBuffer, TAMANHO_BUFFER, 0);
            sscanf(recvBuffer, "%d %d", &linha, &coluna);
        }

        if(ehValida(linha, coluna)) {
            jogo[linha][coluna] = jogador;
            int ganhou = ganhouPorLinhas(jogador) || ganhouPorColunas(jogador) || ganhouPorDiagonalPrin(jogador) || ganhouPorDiagonalSecun(jogador);
            int empate = quantVazias() == 0 && !ganhou;

            // Enviar o tabuleiro ao cliente
            enviarTabuleiro(sendBuffer);

            // Define a mensagem final de acordo com o resultado do jogo
            if(ganhou) {
                if (vez == 1) {
                    strcat(sendBuffer, "\nJogador 1 venceu! Você perdeu!\nFim de Jogo.\n");
                    printf("\nParabéns, jogador 1! Você venceu!\nFim de Jogo.\n");
                } else {
                    strcat(sendBuffer, "\nJogador 2 venceu! Você perdeu!\nFim de Jogo.\n");
                    printf("\nJogador 2 venceu! Fim de Jogo. Jogador 1 perdeu!\n");
                }
                jogando = 0;
            } else if (empate) {
                strcat(sendBuffer, "\nEmpate! Fim de Jogo.\n");
                printf("\nJogo terminou em empate! Fim de Jogo.\n");
                jogando = 0;
            } else {
                strcat(sendBuffer, "\nContinuar\n");
            }

            send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
            vez = 3 - vez; // Alterna entre jogador 1 e 2
        } else {
            printf("Coordenadas inválidas. Tente novamente.\n");
        }
    }

    // Mensagem final para indicar que o jogo acabou
    printf("Fim de Jogo! Pressione qualquer tecla para sair...\n");
    getchar();  // Aguarda uma tecla para que o terminal não feche imediatamente
    getchar();  // Segunda chamada para garantir que funcione corretamente no Dev-C++
    
    // Removemos o fechamento do socket e WSACleanup para que o terminal permaneça aberto
    return 0;
}
