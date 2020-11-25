#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int DEBUG = 0;

const int P1 = 1;
const int P2 = 2;

const int EMPTY = 0;
const int OU = 1; // 王
const int KI = 2; // 金
const int GI = 3; // 銀
const int KK = 4; // 角
const int HI = 5; // 飛
// P1: 正, P2: 負
// 成り: 10x

typedef struct board {
    int state[5][5]; // 盤面全体の情報
    
    int P1OU;
    int P1KI;
    int P1GI;
    int P1KK;
    int P1HI;
    int P1FU;
    
    int P2OU;
    int P2KI;
    int P2GI;
    int P2KK;
    int P2HI;
    int P2FU;
} board

board g_board;
// global variable to contain current state

// P2の持ち駒: -2
//
//     A  B  C  D  E
//   ---------------
// 5| 20 21 22 23 24
// 4| 15 16 17 18 19
// 3| 10 11 12 13 14
// 2|  5  6  7  8  9
// 1|  0  1  2  3  4
//
//      P1の持ち駒: -1

void init() {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            g_board.state[i][j] = EMPTY;
        }
    }
    g_board.state[0][0] = OU;
    g_board.state[0][1] = KI;
    g_board.state[0][2] = GI;
    g_board.state[0][3] = KK;
    g_board.state[0][4] = HI;
    g_board.state[1][0] = FU;

    g_board.state[4][4] = OU * (-1);
    g_board.state[4][3] = KI * (-1);
    g_board.state[4][2] = GI * (-1);
    g_board.state[4][1] = KK * (-1);
    g_board.state[4][0] = HI * (-1);
    g_board.state[3][4] = FU * (-1);

    g_board.P1OU = 0;
    g_board.P1KI = 1;
    g_board.P1GI = 2;
    g_board.P1KK = 3;
    g_board.P1HI = 4;
    g_board.P1FU = 5;
    
    g_board.P2OU = 24;
    g_board.P2KI = 23;
    g_board.P2GI = 22;
    g_board.P2KK = 21;
    g_board.P2HI = 20;
    g_board.P2FU = 19;
}

void print() {
    if (DEBUG == 0) return;

    printf("   A B C D E\n");
    printf("  ----------\n");
    for (int i = 0; i < 5; i++) {
        printf("%d| ", 5 - i);
        for (int j = 0; j < 5; j++) {
            printf("%d ", g_board.state[4 - i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int is_finished(board b) {
    if (b.P1OU < 0 || b.P2OU < 0) return 1;
    return 0;
}

void get_input(int turn) {
    //
}

void compute_output(int turn) {
    //
}

int main(int argc, char* argv[]) {
    init();
    int cnt = 0;

    if (argc >= 3 && !strcmp(argv[2], "1")) DEBUG = 1;

    if (!strcmp(argv[1], "0")) {
        // human plays first
        while (1) {
            print();
            // human's turn
            get_input(P1);
            cnt++;
            print();
            if (is_finished(g_board)) {
                printf("You Win\n");
                exit(0);
            }

            // computer's turn
            compute_output(P2);
            cnt++;
            if (is_finished(g_board)) {
                print();
                printf("You Lose\n");
                exit(0);
            }

            if (cnt >= 150) {
                // stop after 150 turns
                print();
                printf("Even\n");
                exit(0);
            }
        }
    } else {
        // computer plays first
        while (1) {
            print();
            // computer's turn
            compute_output(P1);
            cnt++;
            print();
            if (is_finished(g_board)) {
                printf("You Lose\n");
                exit(0);
            }

            // human's turn
            get_input(P2);
            cnt++;
            if (is_finished(g_board)) {
                print();
                printf("You Win\n");
                exit(0);
            }

            if (cnt >= 150) {
                // stop after 150 turns
                print();
                printf("Even\n");
                exit(0);
            }
        }
    }

    return 0;
}