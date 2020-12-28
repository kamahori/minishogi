#pragma once

int DEBUG = 0;

#define max(a, b) (((a)>(b))?(a):(b))
#define min(a, b) (((a)<(b))?(a):(b))
#define promote(p) (((p)>0)?((p)+6):((p)-6)) // 成る
#define unpromote(p) (((p)>0)?((p)-6):((p)+6)) // 成りを解除
#define ispromoted(p) ((p)>=7||(p)<=-7) // 成っているか
#define playeridx(p) ((p>0)?0:1) // P1の駒なら0、P2の駒なら1。hand と piecebb の配列参照用
#define pieceidx(p) (abs(p)-1) // 駒の絶対値-1。hand と piecebb の配列参照用

//P1: 先手, P2: 後手
const int P1 = 1;
const int P2 = -1;
int USER;
int AI;

enum {
    EMPTY = 0,
    FU = 1, // 歩
    GI = 2, // 銀
    KK = 3, // 角
    HI = 4, // 飛
    KI = 5, // 金
    OU = 6  // 王
};
// P1: 正, P2: 負
// 成り: +6 (promote)

typedef int BitBoard;

typedef struct {
    int state[5][5]; // 盤面全体の情報
    int hand[2][6]; // 持ち駒
    BitBoard piecebb[2][10]; // 各コマの位置(bitboard)
    int turn; // 手番
} board_t;

board_t g_board;
// global variable to contain current state

//     A  B  C  D  E
//   ---------------
// 5| 20 21 22 23 24
// 4| 15 16 17 18 19
// 3| 10 11 12 13 14
// 2|  5  6  7  8  9
// 1|  0  1  2  3  4

typedef struct {
    BitBoard from; // 動かす前
    BitBoard to; // 動かした後
    int piece; // 動かす駒（1～6）
    int promoting; // 成るかどうか
} move_t;
