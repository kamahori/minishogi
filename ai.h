#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "global_const.h"
#include "bitboard.h"
#include "sennichite.h"

void ai_print_move(move_t move)
{
    if (move.from != 0) { // 動かす
        int fromsq = square(move.from), tosq = square(move.to);
        printf("%c%c%c%c", '1' + fromsq / 5, 'A' + fromsq % 5, '1' + tosq / 5, 'A' + tosq % 5);
        if (move.promoting)
            printf("N");
    }
    else { // 打つ
        int tosq = square(move.to);
        printf("%c%c", '1' + tosq / 5, 'A' + tosq % 5);
        switch (move.piece) {
        case FU: printf("FU"); break;
        case GI: printf("GI"); break;
        case KK: printf("KK"); break;
        case HI: printf("HI"); break;
        case KI: printf("KI"); break;
        case OU: printf("OU"); break;
        }
    }
}

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
        mnode_plist[i]->score = 0;
        mnode_plist[i]->next = *mlist;
        *mlist = mnode_plist[i];
    }
    return 1;
}

int alphabeta(int depth, int maxdepth, int alpha, int beta)
{
    int turn = g_board.turn;
    if (turn == AI && judge_checking(AI)) // AIが必ず勝つ
        return INF;
    if (turn == USER && judge_checking(USER)) // USERが必ず勝つ
        return -INF;

    int bestscore = (turn == AI) ? -INF + 10 : INF - 10;
    TTEntry* entry = tt_search();
    if (!entry) { // 未探索
        entry = tt_insert();
        if (depth >= maxdepth)
            return entry->score; // = eval()
    }
    else { // 探索済み
        int searched_score = entry->score;
        if (depth >= maxdepth) {
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

// score が最大となる手を選ぶ
// 返り値　-1：指せる手無し、1：AIの勝利、0：その他
int choose_move(move_t* move, int maxdepth)
{
    // printf("\nmaxdepth = %d\n", maxdepth);
    int bestscore = alphabeta(0, maxdepth, -INF, INF);
    // printf("bestscore: %d\n", bestscore);

    TTEntry* entry = tt_search();
    if (!entry) {
        if (bestscore > INF - 100)
            return 1;
        return 0;
    }
    MNode* mnode = entry->movelist;
    if (!mnode)
        return -1;
    while (mnode) {
        // printf("move: ");
        // ai_print_move(mnode->move);
        // printf(", score: %d\n", mnode->score);
        if (mnode->score == bestscore) {
            *move = mnode->move;
            break;
        }
        mnode = mnode->next;
    }
    if (bestscore > INF - 100) // AIの勝利
        return 1;
    return 0;
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
            entry->score = (loser == USER) ? INF : -INF;
        }
        undo_move(mnode->move, 1);
        mnode = mnode->next;
    }
}

// 指せる手がなければ-1を返す
int compute_output(move_t* move)
{
    int depthlimit = 6;
    int res;
    
    judge_nextmove_sennichite();

    for (int maxdepth = 3; maxdepth <= depthlimit; maxdepth++) {
        res = choose_move(move, maxdepth);
        if (res != 0)
            break;
    }
    if (DEBUG && res == -1) printf("no move\n");
    return res;
}
