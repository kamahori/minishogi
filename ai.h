#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "global_const.h"
#include "bitboard.h"
#include "sennichite.h"

#define INF 1000000000

const int piece_value[10] = { // 盤面上の駒の評価値（暫定値）
    10,     // 歩
    40,     // 銀
    80,     // 角
    100,    // 飛
    50,     // 金
    100000, // 王
    40,     // と金
    40,     // 成銀
    100,    // 馬
    120     // 龍
};
const int handpiece_value[6] = { // 持ち駒の評価値（暫定値）
    11, // 歩
    41, // 銀
    81, // 角
    110, // 飛
    51, // 金
    100000 // 王
};

// AIから見た盤面の評価値（暫定値）改良が必要
int eval()
{
    if (g_board.turn == AI && judge_checking(AI))
        return INF;
    if (g_board.turn == USER && judge_checking(USER))
        return -INF;

    int score = 0;
    for (int p = 0; p < 10; p++) {
        score += popcount(g_board.piecebb[playeridx(AI)][p]) * piece_value[p];
        score -= popcount(g_board.piecebb[playeridx(USER)][p]) * piece_value[p];
    }
    for (int p = 0; p < 6; p++) {
        score += g_board.hand[playeridx(AI)][p] * handpiece_value[p];
        score -= g_board.hand[playeridx(USER)][p] * handpiece_value[p];
    }
    return score;
}

#define NOT_SEARCHED (INF + 10000)

// 次の手をリストにして格納
// 返り値　1：成功、0：失敗
int expand_node(MNode** mlist)
{
    move_t movelist[200];
    int n = get_movelist(movelist);
    if (n == 0)
        return 0;
    MNode** mnode_plist = (MNode**)malloc(sizeof(MNode*) * n);
    for (int i = 0; i < n; i++) {
        mnode_plist[i] = (MNode*)malloc(sizeof(MNode));
        mnode_plist[i]->move = movelist[i];
        mnode_plist[i]->score = NOT_SEARCHED; // 未探索時の値
        mnode_plist[i]->next = *mlist;
        *mlist = mnode_plist[i];
    }
    return 1;
}

// movelist の要素を score の順に並び替える（挿入ソート）
// AIの手番では大きい順、USERの手番では小さい順
void sort_movelist(MNode** movelist)
{
    int is_ai = (g_board.turn == AI);
    if (!*movelist) return;
    MNode* mnode, * next_mn, * mn, * prev_mn;
    mnode = (*movelist)->next;
    (*movelist)->next = NULL;
    while (mnode) {
        if (mnode->score == NOT_SEARCHED) { // 以降は未探索
            mn = *movelist;
            while (mn->next) mn = mn->next;
            mn->next = mnode;
            return;
        }
        next_mn = mnode->next;
        prev_mn = mn = *movelist;
        while (mn) {
            if (is_ai ? (mn->score <= mnode->score) : (mn->score >= mnode->score)) {
                if (mn == *movelist) *movelist = mnode;
                else prev_mn->next = mnode;
                mnode->next = mn;
                break;
            }
            prev_mn = mn;
            mn = mn->next;
        }
        if (!mn) {
            prev_mn->next = mnode;
            mnode->next = NULL;
        }
        mnode = next_mn;
    }
}

int alphabeta(int depth, int maxdepth, int alpha, int beta)
{
    int turn = g_board.turn;
    if (turn == AI && judge_checking(AI)) // AIが必ず勝つ
        return INF - depth;
    if (turn == USER && judge_checking(USER)) // USERが必ず勝つ
        return -INF + depth;

    int bestscore = (turn == AI) ? -INF - 100 : INF + 100;
    TTEntry* entry = tt_search();
    if (!entry) { // 未探索
        entry = tt_insert();
        if (depth >= maxdepth) // 末端局面
            return entry->score; // = eval()
    }
    else { // 探索済み
        int searched_score = entry->score;
        if (depth >= maxdepth || abs(searched_score) > INF - 1000) { // 末端局面か勝敗の確定している局面
            return searched_score;
        }
        if (entry->searched_depth >= maxdepth - depth) { // より深くまで探索しているため利用する
            if (!(entry->ispruned)) {
                return searched_score; // 枝刈りされていない正確な評価値
            }
            if (turn == AI) {
                if (searched_score >= beta) {
                    return searched_score; // beta cut
                }
                bestscore = searched_score;
                alpha = max(alpha, searched_score);
            }
            else {
                if (searched_score <= alpha) {
                    return searched_score; // alpha cut
                }
                bestscore = searched_score;
                beta = min(beta, searched_score);
            }
        }
    }

    if (!(entry->movelist)) {
        int res = expand_node(&(entry->movelist));
        if (!res)
            return bestscore;
    }
    else {
        sort_movelist(&(entry->movelist)); // 評価値の順に並び替える
    }

    MNode* mnode = entry->movelist;
    int score;
    int ispruned = 0;
    while (mnode) { // for each move
        do_move(mnode->move, 1);
        score = alphabeta(depth + 1, maxdepth, alpha, beta);
        undo_move(mnode->move, 1);
        mnode->score = score;
        if (turn == AI) {
            bestscore = max(bestscore, score);
            if (bestscore >= beta) {
                ispruned = 1;
                break; // beta cut
            }
            alpha = max(alpha, bestscore);
        }
        else {
            bestscore = min(bestscore, score);
            if (bestscore <= alpha) {
                ispruned = 1;
                break; // alpha cut
            }
            beta = min(beta, bestscore);
        }

        mnode = mnode->next;
    }

    entry->score = bestscore;
    entry->ispruned = ispruned;
    entry->searched_depth = maxdepth - depth;

    return bestscore;
}

// 千日手かどうかの判定（深さ1の手のみ）
void judge_nextmove_sennichite()
{
    TTEntry* entry = tt_search();
    if (!entry) { // 未探索
        entry = tt_insert();
    }
    if (!(entry->movelist)) {
        int res = expand_node(&(entry->movelist));
        if (!res) return;
    }
    MNode* mnode = entry->movelist;
    int loser;
    while (mnode) {
        do_move(mnode->move, 1);
        loser = judge_sennichite();
        if (loser) {
            entry = tt_search();
            if (!entry)
                entry = tt_insert();
            entry->score = (loser == USER) ? INF - 1 : -INF + 1;
        }
        undo_move(mnode->move, 1);
        mnode = mnode->next;
    }
}

// 反復深化 alpha-beta 探索（深さ depthlimit まで）による最善手を move に代入
// 返り値　1：成功、-1：指し手無し
int choose_move(move_t* move, int depthlimit)
{
    judge_nextmove_sennichite();

    int bestscore = 0;
    for (int maxdepth = 3; maxdepth <= depthlimit; maxdepth++) {
        // printf("\nmaxdepth = %d\n", maxdepth);
        bestscore = alphabeta(0, maxdepth, -INF, INF);
        // printf("bestscore: %d\n", bestscore);
        if (bestscore > INF - 1000) // AIの勝利
            break;
    }

    TTEntry* entry = tt_search();
    if (!entry) {
        TTEntry* entry = tt_insert();
        expand_node(&(entry->movelist));
    }
    if (!(entry->movelist)) // 指せる手がない
        return -1;

    // 王を取る手を探す
    MNode* mnode = entry->movelist;
    while (mnode) {
        if (mnode->move.take == OU) {
            *move = mnode->move;
            return 1;
        }
        mnode = mnode->next;
    }

    // score が最大となる手をランダムに選ぶ
    mnode = entry->movelist;
    move_t bestmoves[50];
    int n = 0;
    while (mnode) {
        if (mnode->score == bestscore) {
            bestmoves[n++] = mnode->move;
        }
        mnode = mnode->next;
    }
    srand(0);
    *move = bestmoves[rand() % n];
    return 1;
}

void print_move(move_t move);

// 次の手を求めて表示する
// 返り値　1：成功、-1：指し手無し
int compute_output(move_t* move)
{
    int depthlimit = 7;

    clock_t start, end;
    start = clock();

    int res = choose_move(move, depthlimit);
    if (res == 1) print_move(*move);

    end = clock();
    if (DEBUG) printf("time: %f\n", (double)(end - start) / CLOCKS_PER_SEC);

    if (DEBUG && res == -1) printf("no move\n");
    return res;
}
