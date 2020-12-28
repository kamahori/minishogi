#pragma once

int DEBUG = 0;

#define max(a, b) (((a)>(b))?(a):(b))
#define min(a, b) (((a)<(b))?(a):(b))
#define promote(p) (((p)>0)?((p)+6):((p)-6)) // ����
#define unpromote(p) (((p)>0)?((p)-6):((p)+6)) // ���������
#define ispromoted(p) ((p)>=7||(p)<=-7) // �����Ă��邩
#define playeridx(p) ((p>0)?0:1) // P1�̋�Ȃ�0�AP2�̋�Ȃ�1�Bhand �� piecebb �̔z��Q�Ɨp
#define pieceidx(p) (abs(p)-1) // ��̐�Βl-1�Bhand �� piecebb �̔z��Q�Ɨp

//P1: ���, P2: ���
const int P1 = 1;
const int P2 = -1;
int USER;
int AI;

enum {
    EMPTY = 0,
    FU = 1, // ��
    GI = 2, // ��
    KK = 3, // �p
    HI = 4, // ��
    KI = 5, // ��
    OU = 6  // ��
};
// P1: ��, P2: ��
// ����: +6 (promote)

typedef int BitBoard;

typedef struct {
    int state[5][5]; // �ՖʑS�̂̏��
    int hand[2][6]; // ������
    BitBoard piecebb[2][10]; // �e�R�}�̈ʒu(bitboard)
    int turn; // ���
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
    BitBoard from; // �������O
    BitBoard to; // ����������
    int piece; // ��������i1�`6�j
    int promoting; // ���邩�ǂ���
} move_t;
