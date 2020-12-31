#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "global_const.h"
#include "bitboard.h"
#include "sennichite.h"

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

const int dist_value[10] = { // 相手の王との距離が 1 のときの価値
    10,     // 歩
    40,     // 銀
    0,      // 角
    0,      // 飛
    50,     // 金
    10,     // 王
    40,     // と金
    40,     // 成銀
    0,      // 馬
    0       // 龍
};

const int handpiece_value[6] = { // 持ち駒の評価値（暫定値）
    11, // 歩
    41, // 銀
    81, // 角
    110, // 飛
    51, // 金
    100000 // 王
};

// turn のプレイヤーから見た盤面の評価値（暫定値）
// 勝敗の判定は必要ない
int eval(int turn)
{
    int score = 0;

    // 盤上の駒単独の価値
    for (int p = 0; p < 10; p++) {
        score += popcount(g_board.piecebb[playeridx(turn)][p]) * piece_value[p];
    }
    // 持ち駒単独の価値
    for (int p = 0; p < 6; p++) {
        score += g_board.hand[playeridx(turn)][p] * handpiece_value[p];
    }

    BitBoard opp_OU = g_board.piecebb[playeridx(-turn)][pieceidx(OU)];
    int sq = square(opp_OU);
    int row = sq / 5, col = sq % 5;

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int piece = g_board.state[i][j];
            if (piece == EMPTY || playeridx(piece) != turn) continue;
            int dist = abs(row - i) + abs(col - j); // 相手の王とのマンハッタン距離
            if (dist == 0) continue;
            score += (int) dist_value[pieceidx(piece)] / dist;
        }
    }
    /*
    //しょぼい駒で王を追い詰めてる方がコスパ良いかも？と思ったので、valueを固定値にしたバージョン
    int value = 80; //暫定値

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int piece = g_board.state[i][j];
            if (piece == EMPTY || playeridx(piece) != turn) continue;
            int manhattan_dist = abs(row - i) + abs(col - j); // 相手の王とのマンハッタン距離
            int chebyshev_dist = max(abs(row - i), abs(col - j)); // 相手の王とのチェビシェフ距離
            if (manhattan_dist == 0) continue;
            if (piece == FU) score += (int) value / manhattan_dist;
            else if (piece == KK || piece == promote(KK)){
                if (abs(abs(row - i) - abs(col - j)) <= 1) score += value / 2;
                //評価値が大きくなりすぎないように2で割っとく。
            }
            else if (piece == HI || piece == promote(HI)){
                if (min(abs(row - i),  abs(col - j)) <= 1) score += value / 2;
                //評価値が大きくなりすぎないように2で割っとく
            }
            else if (piece == OU) continue; //王で王を取りに行くのは考えない。
            else score += value / chebyshev_dist;
        }
    }
    BitBoard movable = 0;
    for (int i=0; i<5; i++){
        for (int j=0; j<5; j++){
            int piece = g_board.state[i][j];
            if (piece == EMPTY || playeridx(piece) == turn) continue;
            else{
                //相手のこまが見つかったら、
                movable = movable | get_movable(abs(piece), bitboard(5*i+j), playeridx(-turn));
            }
        }
    }
    for (int i=0; i<5; i++){
        for (int j=0; j<5; j++){
            int piece = g_board.state[i][j];
            if (piece == EMPTY || playeridx(piece) != turn) continue;
            else{
                //自分のこまが見つかったら、
                if (popcount(movable & bitboard(5*i+j)) > 0){
                    //相手の動ける範囲にいたら、
                    score -= piece_value[pieceidx(piece)];
                }
            }
        }
    }*/

    return score;
}
/*
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
*/
int alphabeta(int depth, int maxdepth, int alpha, int beta)
{
    int turn = g_board.turn;
    if (turn == AI && judge_checking(AI)) // AIが必ず勝つ
        return INF - depth;
    if (turn == USER && judge_checking(USER)) // USERが必ず勝つ
        return -INF + depth;
    if (depth >= maxdepth)
        return eval(AI) - eval(USER);

    int lower, upper;
    TTEntry* entry = tt_search();
    if (!entry) { // 未探索
        entry = tt_insert();
        lower = -INF;
        upper = INF;
    }
    else { // 探索済み
        lower = entry->lower;
        upper = entry->upper;
        if (entry->searched_depth == maxdepth - depth) { // 同じ局面を同じ深さで探索済み
            if (lower == upper) return lower;
            else if (lower >= beta) return lower;
            else if (upper <= alpha) return upper;
            else {
                alpha = max(alpha, lower);
                beta = min(beta, upper);
            }
        }
    }

    int bestscore = (turn == AI) ? -INF - 100 : INF + 100;

    move_t movelist[100];
    int n = get_movelist(movelist, 100);
    if (n == 0) return bestscore;

    move_t move;
    int score;
    for (int i = -1; i < n; i++) { // for each move
        move = (i == -1) ? entry->bestmove : movelist[i]; // 最初に最善手で探索して探索範囲を狭める
        do_move(move, 1);
        if (turn == AI)
            score = alphabeta(depth + 1, maxdepth, max(alpha, bestscore), beta);
        else
            score = alphabeta(depth + 1, maxdepth, alpha, min(beta, bestscore));
        undo_move(move, 1);

        if (turn == AI) {
            if (bestscore < score) {
                bestscore = score;
                entry->bestmove = move;
            }
            if (bestscore >= beta) {
                break; // beta cut
            }
        }
        else {
            if (bestscore > score) {
                bestscore = score;
                entry->bestmove = move;
            }
            if (bestscore <= alpha) {
                break; // alpha cut
            }
        }
    }

    if (bestscore <= alpha) {
        entry->lower = lower;
        entry->upper = bestscore;
    }
    else if (bestscore >= beta) {
        entry->lower = bestscore;
        entry->upper = upper;
    }
    else {
        entry->lower = entry->upper = bestscore;
    }

    entry->searched_depth = maxdepth - depth;

    return bestscore;
}

void print_move(move_t move);

int alphabeta_init(move_t* ret, int maxdepth)
{
    if (judge_checking(AI)) // AIが必ず勝つ
        return INF;
    if (maxdepth <= 0)
        return eval(AI) - eval(USER);

    int lower, upper;
    TTEntry* entry = tt_search();
    if (!entry) { // 未探索
        entry = tt_insert();
    }
    else { // 探索済み
        lower = entry->lower;
        upper = entry->upper;
        if (entry->searched_depth == maxdepth) { // 同じ局面を同じ深さで探索済み
            if (lower == upper)
                return lower;
        }
    }

    int bestscore = -INF - 100;

    move_t movelist[100];
    int n = get_movelist(movelist, 100);
    if (n == 0) return bestscore;

    move_t move;
    int score;
    for (int i = -1; i < n; i++) { // for each move
        move = (i == -1) ? entry->bestmove : movelist[i]; // 最初に最善手で探索して探索範囲を狭める
        do_move(move, 1);
        score = alphabeta(1, maxdepth, -INF, INF);
        undo_move(move, 1);

        // printf("move: ");
        // print_move(move);
        // printf(", score: %d\n", score);
        if (bestscore < score) {
            bestscore = score;
            *ret = entry->bestmove = move;
        }
    }

    entry->lower = entry->upper = bestscore;

    entry->searched_depth = maxdepth;

    return bestscore;
}

// 千日手かどうかの判定（深さ1の手のみ）
void judge_nextmove_sennichite()
{
    TTEntry* entry = tt_search();
    if (!entry) { // 未探索
        entry = tt_insert();
    }

    move_t movelist[200];
    int n = get_movelist(movelist, 200);
    int loser;
    for (int i = 0; i < n; i++) {
        do_move(movelist[i], 1);
        loser = judge_sennichite();
        if (loser) {
            entry = tt_search();
            if (!entry)
                entry = tt_insert();
            entry->lower = entry->upper = (loser == USER) ? INF - 1 : -INF + 1;
        }
        undo_move(movelist[i], 1);
    }
}

void print_move(move_t move);

// 反復深化 alpha-beta 探索（深さ depthlimit まで）による最善手を move に代入
// 返り値　1：成功、-1：指し手無し
int choose_move(move_t* move, int depthlimit)
{
    judge_nextmove_sennichite();

    int bestscore = 0;
    for (int maxdepth = 3; maxdepth <= depthlimit; maxdepth++) {
        // printf("\nmaxdepth = %d\n", maxdepth);
        bestscore = alphabeta_init(move, maxdepth);
        // printf("bestscore: %d\n", bestscore);

        if (bestscore > INF - 1000) // AIの勝利
            break;
    }
    if (bestscore == -INF - 100) return -1; // 指せる手がない

    return 1;
}

// 次の手を求めて表示する
// 返り値　1：成功、-1：指し手無し
int compute_output(move_t* move)
{
    int depthlimit = 7;

    clock_t start, end;
    start = clock();

    int res = choose_move(move, depthlimit);
    if (res == 1) {
        print_move(*move);
        printf("\n");
    }

    end = clock();
    if (DEBUG) printf("time: %f\n", (double)(end - start) / CLOCKS_PER_SEC);

    if (DEBUG && res == -1) printf("no move\n");
    return res;
}
