#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "global_const.h"
#include "sennichite.h"

const BitBoard FU_movable[25] = {
    32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, 0, 0, 0, 0, 0,
};
const BitBoard KI_movable[25] = {
    98, 229, 458, 916, 776, 3137, 7330, 14660, 29320, 24848, 100384, 234560, 469120, 938240, 795136, 3212288, 7505920, 15011840, 30023680, 25444352, 2129920, 5308416, 10616832, 21233664, 8912896,
};
const BitBoard GI_movable[25] = {
    96, 224, 448, 896, 768, 3074, 7173, 14346, 28692, 24584, 98368, 229536, 459072, 918144, 786688, 3147776, 7345152, 14690304, 29380608, 25174016, 65536, 163840, 327680, 655360, 262144,
};
const BitBoard OU_movable[25] = {
    98, 229, 458, 916, 776, 3139, 7335, 14670, 29340, 24856, 100448, 234720, 469440, 938880, 795392, 3214336, 7511040, 15022080, 30044160, 25452544, 2195456, 5472256, 10944512, 21889024, 9175040,
};

// square(0～24) を bitboard に変換
BitBoard bitboard(int square)
{
    return 1 << square;
}

// bitboard(1bitのみ1) を square(0～24) に変換（0のときは-1）
int square(BitBoard bb)
{
    if (bb == 0) return -1;
    int sq = 0;
    if (!(bb & 0xFFFF)) { sq += 16; bb >>= 16; }
    if (!(bb & 0xFF)) { sq += 8; bb >>= 8; }
    if (!(bb & 0xF)) { sq += 4; bb >>= 4; }
    if (!(bb & 3)) { sq += 2; bb >>= 2; }
    if (!(bb & 1)) { sq += 1; }
    return sq;
}

// 盤面の中にあるか
int inboard(int row, int col)
{
    return (0 <= row && row <= 4 && 0 <= col && col <= 4);
}

BitBoard get_KK_movable(int square, BitBoard occupied)
{
    BitBoard bb, res = 0;
    for (int dr = -1; dr <= 1; dr += 2) {
        for (int dc = -1; dc <= 1; dc += 2) {
            for (int row = square / 5 + dr, col = square % 5 + dc; inboard(row, col); row += dr, col += dc) {
                bb = bitboard(5 * row + col);
                res |= bb;
                if (bb & occupied) // 他の駒と衝突
                    break;
            }
        }
    }
    return res;
}

BitBoard get_HI_movable(int square, BitBoard occupied)
{
    BitBoard bb, res = 0;
    int row, col;
    col = square % 5;
    for (int dr = -1; dr <= 1; dr += 2) {
        for (row = square / 5 + dr; inboard(row, col); row += dr) {
            bb = bitboard(5 * row + col);
            res |= bb;
            if (bb & occupied) // 他の駒と衝突
                break;
        }
    }
    row = square / 5;
    for (int dc = -1; dc <= 1; dc += 2) {
        for (col = square % 5 + dc; inboard(row, col); col += dc) {
            bb = bitboard(5 * row + col);
            res |= bb;
            if (bb & occupied) // 他の駒と衝突
                break;
        }
    }
    return res;
}

void printbb(BitBoard bb)
{
    for (int row = 4; row >= 0; row--) {
        for (int col = 0; col < 5; col++) {
            printf("%d", (bb >> (row * 5 + col)) & 1);
        }
        printf("\n");
    }
}

// 上下反転
BitBoard mirrorbb(BitBoard bb)
{
    BitBoard res = 0;
    for (int i = 0; i < 5; i++)
        res |= (bb >> (20 - 5 * i) & 0b11111) << (5 * i);
    return res;
}

// bb の中の1の個数
int popcount(BitBoard bb)
{
    int n = 0;
    for (BitBoard ls = bb & -bb; bb; bb ^= ls, ls = bb & -bb) n++;
    return n;
}

// turn のプレイヤーの駒のある場所
BitBoard occupied(int turn)
{
    BitBoard res = 0;
    for (int piece = 0; piece < 10; piece++) {
        res |= g_board.piecebb[playeridx(turn)][piece];
    }
    return res;
}

// 駒のない場所
BitBoard empty()
{
    return (~(occupied(P1) | occupied(P2))) & ((1 << 25) - 1);
}

// turn のプレイヤーの駒の動ける場所
BitBoard get_movable(int piece, BitBoard place, int turn)
{
    BitBoard occupied_self = occupied(turn);
    BitBoard occupied_all = occupied(P1) | occupied(P2);
    if (turn == P2) {
        place = mirrorbb(place);
        occupied_self = mirrorbb(occupied_self);
        occupied_all = mirrorbb(occupied_all);
    }
    int sq = square(place);

    BitBoard bb = 0;
    switch (abs(piece)) {
    case FU:
        bb = FU_movable[sq];
        break;
    case GI:
        bb = GI_movable[sq];
        break;
    case KK:
        bb = get_KK_movable(sq, occupied_all);
        break;
    case HI:
        bb = get_HI_movable(sq, occupied_all);
        break;
    case KI:
    case promote(FU):
    case promote(GI):
        bb = KI_movable[sq];
        break;
    case OU:
        bb = OU_movable[sq];
        break;
    case promote(KK):
        bb = get_KK_movable(sq, occupied_all) | OU_movable[sq];
        break;
    case promote(HI):
        bb = get_HI_movable(sq, occupied_all) | OU_movable[sq];
        break;
    default:
        break;
    }

    bb = bb & ~occupied_self; // 自分の駒のある場所へは動けない
    return (turn == P2) ? mirrorbb(bb) : bb;
}

// turn のプレイヤーの駒が動ける場所すべて
BitBoard get_all_movable(int turn)
{
    BitBoard res = 0, bb;
    for (int piece = 1; piece <= 10; piece++) {
        bb = g_board.piecebb[playeridx(turn)][pieceidx(piece)];
        for (BitBoard lb = bb & -bb; bb; bb ^= lb, lb = bb & -bb)
            res |= get_movable(piece, lb, turn);
    }
    return res;
}

// bb が相手の陣地にあるか
int in_opp_area(BitBoard bb)
{
    int row = square(bb) / 5;
    return (g_board.turn == P1) ? (row == 4) : (row == 0);
}

// bb がある列
BitBoard columnbb(BitBoard bb)
{
    int col = square(bb) % 5;
    return 0b0000100001000010000100001 << col;
}

// g_board をコピー
board_t copy_board()
{
    board_t copy;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++)
            copy.state[i][j] = g_board.state[i][j];
    }
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++)
            copy.hand[i][j] = g_board.hand[i][j];
        for (int j = 0; j < 10; j++)
            copy.piecebb[i][j] = g_board.piecebb[i][j];
    }
    copy.turn = g_board.turn;
    copy.state_h = g_board.state_h;
    copy.hand_h = g_board.hand_h;
    return copy;
}

// 駒を動かす（g_board を書き換える）
void do_move(move_t move, int update_hash_value)
{
    BitBoard from = move.from, to = move.to;
    int fromsq = square(from), tosq = square(to);
    int turn = g_board.turn;
    int piece = move.piece * turn;
    int target = move.take * -turn;

    if (from) { // 動かす
        g_board.state[fromsq / 5][fromsq % 5] = EMPTY;
        g_board.piecebb[playeridx(turn)][pieceidx(piece)] ^= from;
        if (target != EMPTY) { // 駒を取る
            g_board.piecebb[playeridx(-turn)][pieceidx(target)] ^= to;
            if (ispromoted(target))
                target = unpromote(target);
            g_board.hand[playeridx(turn)][pieceidx(target)] += 1;
        }
        if (move.promoting) { // 成る
            g_board.state[tosq / 5][tosq % 5] = promote(piece);
            g_board.piecebb[playeridx(turn)][pieceidx(promote(piece))] ^= to;
        }
        else { // 成らない
            g_board.state[tosq / 5][tosq % 5] = piece;
            g_board.piecebb[playeridx(turn)][pieceidx(piece)] ^= to;
        }
    }

    else { // 打つ
        g_board.hand[playeridx(turn)][pieceidx(piece)] -= 1;
        g_board.state[tosq / 5][tosq % 5] = piece;
        g_board.piecebb[playeridx(turn)][pieceidx(piece)] ^= to;
    }

    g_board.turn *= -1;

    if (update_hash_value) {
        int state_h = g_board.state_h;
        int hand_h = g_board.hand_h;
        if (from) { // 動かす
            state_h ^= hash_seed[hashidx(piece)][fromsq];
            if (target != EMPTY) { // 駒を取る
                state_h ^= hash_seed[hashidx(target)][tosq];
                if (ispromoted(target))
                    target = unpromote(target);
                hand_h += hand_hash_seed(turn, abs(target));
            }
            if (move.promoting) { // 成る
                state_h ^= hash_seed[hashidx(promote(piece))][tosq];
            }
            else {
                state_h ^= hash_seed[hashidx(piece)][tosq];
            }
        }
        else { // 打つ
            state_h ^= hash_seed[hashidx(piece)][tosq];
            hand_h -= hand_hash_seed(turn, abs(piece));
        }
        state_h ^= 1; // turn を入れ替える
        g_board.state_h = state_h;
        g_board.hand_h = hand_h;
    }
}

// 駒を戻す（g_board を書き換える）
void undo_move(move_t move, int restore_hash_value)
{
    BitBoard from = move.from, to = move.to;
    int fromsq = square(from), tosq = square(to);
    int turn = -g_board.turn;
    int piece = move.piece * turn;
    int target = move.take * -turn;

    if (from) { // 動いた
        g_board.state[tosq / 5][tosq % 5] = target;
        if (target != EMPTY) { // 駒を取った
            g_board.piecebb[playeridx(-turn)][pieceidx(target)] ^= to;
            if (ispromoted(target))
                target = unpromote(target);
            g_board.hand[playeridx(turn)][pieceidx(target)] -= 1;
        }
        if (move.promoting) { // 成った
            g_board.piecebb[playeridx(turn)][pieceidx(promote(piece))] ^= to;
        }
        else { // 成らなかった
            g_board.piecebb[playeridx(turn)][pieceidx(piece)] ^= to;
        }
        g_board.state[fromsq / 5][fromsq % 5] = piece;
        g_board.piecebb[playeridx(turn)][pieceidx(piece)] ^= from;
    }

    else { // 打った
        g_board.hand[playeridx(turn)][pieceidx(piece)] += 1;
        g_board.state[tosq / 5][tosq % 5] = EMPTY;
        g_board.piecebb[playeridx(turn)][pieceidx(piece)] ^= to;
    }

    g_board.turn *= -1;

    if (restore_hash_value) {
        int state_h = g_board.state_h;
        int hand_h = g_board.hand_h;
        if (from) { // 動いた
            if (target != EMPTY) { // 駒を取った
                state_h ^= hash_seed[hashidx(target)][tosq];
                if (ispromoted(target))
                    target = unpromote(target);
                hand_h -= hand_hash_seed(turn, abs(target));
            }
            if (move.promoting) { // 成った
                state_h ^= hash_seed[hashidx(promote(piece))][tosq];
            }
            else { // 成らなかった
                state_h ^= hash_seed[hashidx(piece)][tosq];
            }
            state_h ^= hash_seed[hashidx(piece)][fromsq];
        }

        else { // 打った
            state_h ^= hash_seed[hashidx(piece)][tosq];
            hand_h += hand_hash_seed(turn, abs(piece));
        }
        state_h ^= 1; // turn を入れ替える
        g_board.state_h = state_h;
        g_board.hand_h = hand_h;
    }
}

// turn のプレイヤーが相手に王手を掛けているか
int judge_checking(int turn)
{
    return g_board.piecebb[playeridx(-turn)][pieceidx(OU)] & get_all_movable(turn);
}

int get_movelist(move_t* movelist, int maxi);

// 現在の手番のプレイヤーが詰んでいるか
int judge_tsumibb()
{
    move_t movelist[200];
    int n = get_movelist(movelist, 200);
    for (int i = 0; i < n; i++) {
        do_move(movelist[i], 0);
        if (!judge_checking(g_board.turn)) { // 現在の手番のプレイヤーが手を指して相手が勝たない
            undo_move(movelist[i], 0);
            return 0;
        }
        undo_move(movelist[i], 0);
    }
    return 1;
}

// 現在の局面での可能な手を movelist に入れる（最大 maxi 手）
// 可能な手の数を返す
int get_movelist(move_t* movelist, int maxi)
{
    int i = 0;
    BitBoard frombb, tobb;
    move_t move;
    int turn = g_board.turn;
    BitBoard self_all_movable = get_all_movable(turn); // 自分の駒の利き
    BitBoard opp_all_movable = get_all_movable(-turn); // 相手の駒の利き

    // 動かす
    for (int piece = 1; piece <= 10; piece++) {
        frombb = g_board.piecebb[playeridx(turn)][pieceidx(piece)];
        for (BitBoard f = frombb & -frombb; frombb; frombb ^= f, f = frombb & -frombb) {
            tobb = get_movable(piece, f, turn);
            if (piece == OU)
                tobb &= ~opp_all_movable; // 王は相手の利きがある場所には動かせない
            for (BitBoard t = tobb & -tobb; tobb; tobb ^= t, t = tobb & -tobb) {
                move.from = f;
                move.to = t;
                move.piece = piece;
                move.take = abs(g_board.state[square(t) / 5][square(t) % 5]);
                if ((piece == FU || piece == KK || piece == HI) && (in_opp_area(f) || in_opp_area(t)))
                    move.promoting = 1; // 歩、角、飛は成れれば必ず成る
                else
                    move.promoting = 0;
                movelist[i++] = move;
                if (i >= maxi) return i;
                if (piece == GI && (in_opp_area(f) || in_opp_area(t))) { // 銀は成る手と成らない手を両方考慮する
                    move.from = f;
                    move.to = t;
                    move.piece = piece;
                    move.promoting = 1;
                    move.take = abs(g_board.state[square(t) / 5][square(t) % 5]);
                    movelist[i++] = move;
                    if (i >= maxi) return i;
                }
            }
        }
    }

    // 打つ
    for (int piece = 1; piece <= 6; piece++) {
        if (g_board.hand[playeridx(turn)][pieceidx(piece)]) {
            tobb = empty();
            if (piece != FU)
                // 歩以外は相手に取られない場所か相手に取られてもその駒を取り返せる場所に打つ
                tobb &= ~opp_all_movable | (self_all_movable & opp_all_movable);
            for (BitBoard t = tobb & -tobb; tobb; tobb ^= t, t = tobb & -tobb) {
                move.from = 0; // 打つ場合は from = 0
                move.to = t;
                move.piece = piece;
                move.take = EMPTY;
                move.promoting = 0;
                if (piece == FU) {
                    if (in_opp_area(t) // 敵陣に歩
                        || (g_board.piecebb[playeridx(turn)][pieceidx(FU)] & columnbb(t))) // 二歩
                        continue;
                    if (get_movable(piece, t, turn) & g_board.piecebb[playeridx(-turn)][pieceidx(OU)]) { // 王の前に歩
                        do_move(move, 0);
                        if (judge_tsumibb()) { // 打ち歩詰め
                            undo_move(move, 0);
                            continue;
                        }
                        undo_move(move, 0);
                    }
                }
                movelist[i++] = move;
                if (i >= maxi) return i;
            }
        }
    }

    return i;
}

// 相手の王を取る手の一つを move に代入
// 返り値　1：成功、0：失敗
int capture_OU(move_t* move)
{
    int turn = g_board.turn;
    BitBoard frombb, tobb;
    BitBoard opp_OU = g_board.piecebb[playeridx(turn)][pieceidx(OU)]; // 相手の王の場所

    move->to = opp_OU;
    move->take = OU;
    move->promoting = 0;
    for (int piece = 1; piece <= 10; piece++) {
        frombb = g_board.piecebb[playeridx(turn)][pieceidx(piece)];
        for (BitBoard f = frombb & -frombb; frombb; frombb ^= f, f = frombb & -frombb) {
            tobb = get_movable(piece, f, turn);
            if (tobb & opp_OU) { // 王が取れる
                move->from = f;
                move->piece = piece;
                return 1;
            }
        }
    }
    return 0;
}

// 現在の局面での可能な手のうち適当な一つを move に代入（entry->bestmove の初期化用）
// 返り値　1：成功、0：失敗
int get_onemove(move_t* move)
{
    BitBoard frombb, tobb;
    int turn = g_board.turn;
    BitBoard opp_all_movable = get_all_movable(-turn); // 相手の駒の利き

    // 動かす
    for (int piece = 1; piece <= 10; piece++) {
        frombb = g_board.piecebb[playeridx(turn)][pieceidx(piece)];
        for (BitBoard f = frombb & -frombb; frombb; frombb ^= f, f = frombb & -frombb) {
            tobb = get_movable(piece, f, turn);
            if (piece == OU)
                tobb &= ~opp_all_movable; // 王は相手の利きがある場所には動かせない
            for (BitBoard t = tobb & -tobb; tobb; tobb ^= t, t = tobb & -tobb) {
                move->from = f;
                move->to = t;
                move->piece = piece;
                move->take = abs(g_board.state[square(t) / 5][square(t) % 5]);
                if ((piece == FU || piece == KK || piece == HI) && (in_opp_area(f) || in_opp_area(t)))
                    move->promoting = 1; // 歩、角、飛は成れれば必ず成る
                else
                    move->promoting = 0;
                return 1;
            }
        }
    }

    // 打つ
    for (int piece = 5; piece > 0; piece--) {
        if (g_board.hand[playeridx(turn)][pieceidx(piece)]) {
            tobb = empty();
            for (BitBoard t = tobb & -tobb; tobb; tobb ^= t, t = tobb & -tobb) {
                move->from = 0; // 打つ場合は from = 0
                move->to = t;
                move->piece = piece;
                move->take = EMPTY;
                move->promoting = 0;
                if (piece == FU) {
                    if (in_opp_area(t) // 敵陣に歩
                        || (g_board.piecebb[playeridx(turn)][pieceidx(FU)] & columnbb(t))) // 二歩
                        continue;
                    if (get_movable(piece, t, turn) & g_board.piecebb[playeridx(-turn)][pieceidx(OU)]) { // 王の前に歩
                        do_move(*move, 0);
                        if (judge_tsumibb()) { // 打ち歩詰め
                            undo_move(*move, 0);
                            continue;
                        }
                        undo_move(*move, 0);
                    }
                }
                return 1;
            }
        }
    }

    return 0;
}
