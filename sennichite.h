#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "global_const.h"
#include "bitboard.h"

int hash_seed[21][25];
#define hashidx(p) ((p)+10) // 駒に対応する配列の index。hash_seed[10] は使わない
#define hand_hash_seed(turn, piece) (1<<((turn)*12+(piece)*2))

// state のハッシュ値を計算 (Zobrist hashing)
int hash_state(board_t board)
{
    int hash = (board.turn == 1) ? 0 : 1; // 最下位ビットは turn を表す
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            if (board.state[row][col] != EMPTY)
                hash ^= hash_seed[hashidx(board.state[row][col])][5 * row + col];
        }
    }
    return hash;
}
// hand のハッシュ値を計算
int hash_hand(board_t board)
{
    int hash = 0;
    for (int turn = 0; turn < 2; turn++) {
        for (int piece = 0; piece < 6; piece++) {
            hash += board.hand[turn][piece] * hand_hash_seed(turn, piece);
        }
    }
    return hash;
}

// move で動いた後の state のハッシュ値を計算
int hash_state_aftermove(int hash, move_t move, int turn)
{
    BitBoard from = move.from;
    int fromsq = square(from), tosq = square(move.to);
    int piece = move.piece * turn;
    int target = move.take * turn;

    if (from) { // 動かす
        hash ^= hash_seed[hashidx(piece)][fromsq];
        if (target != EMPTY) { // 駒を取る
            hash ^= hash_seed[hashidx(target)][tosq];
        }
        if (move.promoting) { // 成る
            hash ^= hash_seed[hashidx(promote(piece))][tosq];
        }
        else {
            hash ^= hash_seed[hashidx(piece)][tosq];
        }
    }
    else { // 打つ
        hash ^= hash_seed[hashidx(piece)][tosq];
    }
    hash ^= 1; // turn を入れ替える
    return hash;
}
// move で動いた後の hand のハッシュ値を計算
int hash_hand_aftermove(int hash, move_t move, int turn)
{
    BitBoard from = move.from;
    int piece = move.piece;
    int target = move.take;

    if (from) { // 動かす
        if (target != EMPTY) { // 駒を取る
            if (ispromoted(target))
                target = unpromote(target);
            hash += hand_hash_seed(turn, target);
        }
    }
    else { // 打つ
        hash -= hand_hash_seed(turn, piece);
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

int st_code(int key)
{
    return key % ST_SIZE;
}

STEntry* st_search(board_t board)
{
    int state = hash_state(board), hand = hash_hand(board);
    int index = st_hash(state);
    while (STable[index]) {
        if (STable[index]->state == state && STable[index]->hand == hand)
            return STable[index];
        index = st_rehash(index);
    }
    return NULL;
}

void st_insert(board_t board)
{
    int state = hash_state(board), hand = hash_hand(board);
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

void sennichite_init()
{
    for (int i = 0; i < ST_SIZE; i++) {
        STable[i] = NULL;
    }
    srand(0);
    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 25; j++) {
            hash_seed[i][j] = (rand() & ((1 << 15) - 1)) << 16 | (rand() & ((1 << 15) - 1)) << 1; // 31桁の乱数（最下位ビットは0）
        }
    }
    st_insert(g_board);
}


typedef struct slnode_t {
    int state;
    int hand;
    int ischecking;
    struct slnode_t* next;
}SLNode; // node of sennichite list

SLNode* SList = NULL; // 連続王手の千日手判定用リスト

void sl_prepend(board_t board)
{
    SLNode* node = (SLNode*)malloc(sizeof(SLNode));
    if (!node) {
        printf("failed to allocate memory\n");
        exit(1);
    }
    node->state = hash_state(board);
    node->hand = hash_hand(board);
    node->ischecking = judge_checking(board, -board.turn); // 手を指したプレイヤー（現在の手番の逆）が王手を掛けていたか
    node->next = SList;
    SList = node;
}

// 連続王手の千日手が成立しているかを返す
int judge_checking_sennichite(board_t board)
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
int judge_sennichite(board_t board, move_t move)
{
    board = do_move(board, move);
    STEntry* entry = st_search(board);
    if (!entry || entry->sennichite < 3)
        return 0; // 千日手でない
    if (judge_checking_sennichite(board))
        return -board.turn; // 王手をかけている方の負け
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
