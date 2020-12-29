#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "global_const.h"
#include "bitboard.h"

int hash_seed[21][25];
#define hashidx(p) ((p)+10) // 駒に対応する配列の index。hash_seed[10] は使わない
#define hand_hash_seed(turn, piece) (1<<((turn)*12+(piece)*2))

void hash_init()
{
    srand(0);
    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 25; j++) {
            hash_seed[i][j] = (rand() & ((1 << 15) - 1)) << 16 | (rand() & ((1 << 15) - 1)) << 1; // 31桁の乱数（最下位ビットは0）
        }
    }
}

// state のハッシュ値を計算 (Zobrist hashing)
int hash_state()
{
    int hash = (g_board.turn == 1) ? 0 : 1; // 最下位ビットは turn を表す
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            if (g_board.state[row][col] != EMPTY)
                hash ^= hash_seed[hashidx(g_board.state[row][col])][5 * row + col];
        }
    }
    return hash;
}
// hand のハッシュ値を計算
int hash_hand()
{
    int hash = 0;
    for (int turn = 0; turn < 2; turn++) {
        for (int piece = 0; piece < 6; piece++) {
            hash += g_board.hand[turn][piece] * hand_hash_seed(turn, piece);
        }
    }
    return hash;
}


#define ST_SIZE (1 << 10)
#define st_hash(key) ((key)&(ST_SIZE-1))
#define st_rehash(key) ((key+1)&(ST_SIZE-1))

typedef struct {
    int state;
    int hand;
    int sennichite;
} STEntry; // entry of sennichite table

STEntry* STable[ST_SIZE]; // 千日手判定用ハッシュテーブル

void st_init()
{
    for (int i = 0; i < ST_SIZE; i++) {
        STable[i] = NULL;
    }
}

STEntry* st_search()
{
    int state = g_board.state_h, hand = g_board.hand_h;
    int index = st_hash(state);
    while (STable[index]) {
        if (STable[index]->state == state && STable[index]->hand == hand)
            return STable[index];
        index = st_rehash(index);
    }
    return NULL;
}

void st_insert()
{
    int state = g_board.state_h, hand = g_board.hand_h;
    int index = st_hash(state);
    while (STable[index]) {
        if (STable[index]->state == state && STable[index]->hand == hand) {
            STable[index]->sennichite++;
            return;
        }
        index = st_rehash(index);
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

void print_st()
{
    printf("STable:\n");
    for (int i = 0; i < ST_SIZE; i++) {
        if (STable[i])
            printf("state: %d, hand: %d, sennichite: %d\n", STable[i]->state, STable[i]->hand, STable[i]->sennichite);
    }
}


typedef struct slnode_t {
    int state;
    int hand;
    int ischecking;
    struct slnode_t* next;
}SLNode; // node of sennichite list

SLNode* SList = NULL; // 連続王手の千日手判定用リスト

int judge_checking(int turn);

void sl_prepend()
{
    SLNode* node = (SLNode*)malloc(sizeof(SLNode));
    if (!node) {
        printf("failed to allocate memory\n");
        exit(1);
    }
    node->state = g_board.state_h;
    node->hand = g_board.hand_h;
    node->ischecking = judge_checking(-g_board.turn); // 手を指したプレイヤー（現在の手番の逆）が王手を掛けているか
    node->next = SList;
    SList = node;
}

void print_sl()
{
    SLNode* node = SList;
    printf("SList:\n");
    while (node) {
        printf("state: %d, hand: %d, ischecking: %d\n", node->state, node->hand, node->ischecking);
        node = node->next;
    }
}

// 連続王手の千日手が成立しているかを返す
int judge_checking_sennichite()
{
    SLNode* node = SList->next;
    int state = g_board.state_h, hand = g_board.hand_h;
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

// 現在の局面が千日手かどうか
// 千日手の場合、負けるプレイヤーの turn を返す
int judge_sennichite()
{
    STEntry* entry = st_search();
    if (!entry || entry->sennichite < 3)
        return 0; // 千日手でない
    if (judge_checking_sennichite())
        return -g_board.turn; // 王手をかけている方の負け
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


typedef struct mnode_t {
    move_t move;
    int score;
    struct mnode_t* next;
} MNode;

typedef struct {
    int state; // state のハッシュ値
    int hand; // hand のハッシュ値
    int score; // 局面の評価値
    int ispruned; // 枝刈りされたかどうか
    int searched_depth; // 探索した深さ
    MNode* movelist; // 次の手のリスト
} TTEntry; // entry of transposition table

#define TT_SIZE (1 << 27)
#define tt_hash(key) ((key)&(TT_SIZE-1))
#define tt_rehash(key) ((key+1)&(TT_SIZE-1))

TTEntry* TTable[TT_SIZE]; // 局面記録用テーブル

void tt_init()
{
    for (int i = 0; i < TT_SIZE; i++) {
        TTable[i] = NULL;
    }
}

TTEntry* tt_search()
{
    int state_h = g_board.state_h, hand_h = g_board.hand_h;
    int index = tt_hash(state_h);
    while (TTable[index]) {
        if (TTable[index]->state == state_h && TTable[index]->hand == hand_h)
            return TTable[index];
        index = tt_rehash(index);
    }
    return NULL;
}

int eval();

TTEntry* tt_insert()
{
    int state_h = g_board.state_h, hand_h = g_board.hand_h;
    int index = tt_hash(state_h);
    while (TTable[index]) {
        index = tt_rehash(index);
    }
    TTEntry* entry = (TTEntry*)malloc(sizeof(TTEntry));
    if (!entry) {
        perror("failed to allocate memory");
        exit(1);
    }
    entry->state = state_h;
    entry->hand = hand_h;
    entry->score = 0;// eval();
    entry->ispruned = 0;
    entry->searched_depth = 0;
    entry->movelist = NULL;
    TTable[index] = entry;
    return entry;
}

void print_tt()
{
    printf("TTable:\n");
    for (int i = 0; i < TT_SIZE; i++) {
        if (TTable[i])
            printf("state: %d, hand: %d, score: %d, searched_depth: %d\n", TTable[i]->state, TTable[i]->hand, TTable[i]->score, TTable[i]->searched_depth);
    }
}

double tt_usage_rate()
{
    int cnt = 0;
    for (int i = 0; i < TT_SIZE; i++) {
        if (TTable[i]) cnt++;
    }
    return (double)cnt / TT_SIZE;
}
