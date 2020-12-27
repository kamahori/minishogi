#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "global_const.h"
#include "bitboard.h"

#define ST_SIZE (1 << 10)

typedef struct {
    int state;
    int hand;
    int sennichite;
} STEntry;

STEntry* STable[ST_SIZE]; // 千日手判定用ハッシュテーブル
int hash_seed[21][25];

void st_insert(board_t board);

void sennichite_init()
{
    for (int i = 0; i < ST_SIZE; i++) {
        STable[i] = NULL;
    }
    srand(0);
    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 25; j++) {
            hash_seed[i][j] = (rand() & ((1 << 15) - 1)) << 15 | (rand() & ((1 << 15) - 1)); // 30桁の乱数
        }
    }
    st_insert(g_board);
}

// state を整数に変換 (Zobrist hashing)
int hash_state(board_t board)
{
    int res = 0;
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            res ^= hash_seed[board.state[row][col] + 10][5 * row + col];
        }
    }
    return res;
}
// hand を整数に変換
int hash_hand(board_t board)
{
    int res = 0;
    for (int turn = 0; turn < 2; turn++) {
        for (int piece = 0; piece < 6; piece++) {
            res |= board.hand[turn][piece] << (turn * 12 + piece * 2);
        }
    }
    return res;
}

int st_code(int key)
{
    return key % ST_SIZE;
}

STEntry* st_search(board_t board, int turn)
{
    int state = hash_state(board), hand = hash_hand(board);
    int index = st_code(state);
    while (STable[index]) {
        if (STable[index]->state == state && STable[index]->hand == hand)
            return STable[index];
        index = (index + 1) % ST_SIZE;
    }
    return NULL;
}

void st_insert(board_t board)
{
    int state = hash_state(board), hand = hash_hand(board);
    int index = st_code(state);
    while (STable[index]) {
        if (STable[index]->state == state && STable[index]->hand == hand) {
            STable[index]->sennichite++;
            return;
        }
        index = (index + 1) % ST_SIZE;
    }
    STEntry* entry = (STEntry*)malloc(sizeof(STEntry));
    if (!entry) {
        perror("failed to allocate memory");
        exit(1);
    }
    entry->state = state;
    entry->hand = hand;
    entry->sennichite = 1;
    STable[index] = entry;
}

typedef struct slnode_t {
    int state;
    int hand;
    int ischecking;
    struct slnode_t* next;
}SLNode;

SLNode* SList = NULL; // 連続王手の千日手判定用リスト

void sl_prepend(board_t board, int turn)
{
    SLNode* node = (SLNode*)malloc(sizeof(SLNode));
    if (!node) {
        printf("failed to allocate memory\n");
        exit(1);
    }
    node->state = hash_state(board);
    node->hand = hash_hand(board);
    node->ischecking = judge_checkbb(board, turn);
    node->next = SList;
    SList = node;
}

// 連続王手の千日手が成立しているかを返す
int judge_checking_sennichite(board_t board, int turn)
{
    SLNode* node = SList->next;
    int state = hash_state(board), hand = hash_hand(board);
    int sennichite = 1;
    while (node->ischecking) {
        if (node->state == state && node->hand == hand)
            sennichite++;
        if (sennichite >= 4)
            return 1;
        node = node->next;
        if (!node) break;
        node = node->next;
    }
    return 0;
}

// move で動いた後に千日手で負けるプレイヤーの turn を返す
int judge_sennichite(board_t board, move_t move, int turn)
{
    board = do_move(board, move, turn);
    STEntry* entry = st_search(board, turn);
    if (!entry || entry->sennichite < 3)
        return 0; // 千日手でない
    if (judge_checking_sennichite(board, turn))
        return turn; // 王手をかけている方の負け
    return P1; // 先手の負け
}

void print_hash()
{
    printf("STable:\n");
    for (int i = 0; i < ST_SIZE; i++) {
        if (STable[i])
            printf("state: %d, hand: %d, sennichite: %d\n", STable[i]->state, STable[i]->hand, STable[i]->sennichite);
    }
    SLNode* node = SList;
    printf("SList:\n");
    while (node) {
        printf("state: %d, hand: %d, ischecking: %d\n", node->state, node->hand, node->ischecking);
        node = node->next;
    }
}
