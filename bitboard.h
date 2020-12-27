#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "global_const.h"

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
BitBoard occupied(board_t board, int turn)
{
    BitBoard res = 0;
    for (int piece = 0; piece < 10; piece++) {
        res |= board.piecebb[playeridx(turn)][piece];
    }
    return res;
}

// 駒のない場所
BitBoard empty(board_t board)
{
    return (~(occupied(board, P1) | occupied(board, P2))) & ((1 << 25) - 1);
}

// 駒の動ける場所
BitBoard get_movable(board_t board, int piece, BitBoard place, int turn)
{
    int mir = (turn == -1) ? 1 : 0;
    BitBoard occupied_self = occupied(board, turn);
    BitBoard occupied_all = occupied(board, P1) | occupied(board, P2);
    if (mir) {
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
    return (mir) ? mirrorbb(bb) : bb;
}

// turn のプレイヤーの駒が動ける場所すべて
BitBoard get_all_movable(board_t board, int turn)
{
    BitBoard res = 0, bb;
    for (int piece = 1; piece <= 10; piece++) {
        bb = board.piecebb[playeridx(turn)][pieceidx(piece)];
        for (BitBoard lb = bb & -bb; bb; bb ^= lb, lb = bb & -bb)
            res |= get_movable(board, piece, lb, turn);
    }
    return res;
}

// bb が相手の陣地にあるか
int in_opp_area(BitBoard bb, int turn)
{
    int row = square(bb) / 5;
    return (turn == 1) ? (row == 4) : (row == 0);
}

// bb がある列
BitBoard columnbb(BitBoard bb)
{
    int col = square(bb) % 5;
    return 0b0000100001000010000100001 << col;
}

// board をコピー
board_t copy_board(board_t board)
{
    board_t new;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++)
            new.state[i][j] = board.state[i][j];
    }
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++)
            new.hand[i][j] = board.hand[i][j];
        for (int j = 0; j < 10; j++)
            new.piecebb[i][j] = board.piecebb[i][j];
    }
    return new;
}

// 駒を動かす
board_t do_move(board_t board, move_t move, int turn)
{
    board_t next = copy_board(board);
    BitBoard from = move.from, to = move.to;
    int fromsq = square(from), tosq = square(to);
    int piece = move.piece * turn;

    if (from) { // 動かす
        next.state[fromsq / 5][fromsq % 5] = EMPTY;
        next.piecebb[playeridx(turn)][pieceidx(piece)] ^= from;
        if (move.promoting) {
            next.state[tosq / 5][tosq % 5] = promote(piece);
            next.piecebb[playeridx(turn)][pieceidx(promote(piece))] ^= to;
        }
        else {
            next.state[tosq / 5][tosq % 5] = piece;
            next.piecebb[playeridx(turn)][pieceidx(piece)] ^= to;
        }

        int target = board.state[tosq / 5][tosq % 5];
        if (target != EMPTY) { // 駒を取る
            next.piecebb[playeridx(-turn)][pieceidx(target)] ^= to;
            if (ispromoted(target))
                target = unpromote(target);
            next.hand[playeridx(turn)][pieceidx(target)] += 1;
        }
    }

    else { // 打つ
        next.hand[playeridx(turn)][pieceidx(piece)] -= 1;
        next.state[tosq / 5][tosq % 5] = piece;
        next.piecebb[playeridx(turn)][pieceidx(piece)] ^= to;
    }

    return next;
}

// turnのプレイヤーが王手をしているか
int judge_checkbb(board_t board, int turn)
{
    return board.piecebb[playeridx(-turn)][pieceidx(OU)] & get_all_movable(board, turn);
}

int get_movelist(move_t* movelist, board_t board, int turn);

// turnのプレイヤーが詰んでいるか
int judge_tsumibb(board_t board, int turn)
{
    move_t movelist[200];
    int n = get_movelist(movelist, board, turn);
    for (int i = 0; i < n; i++) {
        if (!judge_checkbb(do_move(board, movelist[i], turn), -turn))
            return 0;
    }
    return 1;
}

// turn のプレイヤーの可能な手を movelist に入れる
// 可能な手の数を返す
int get_movelist(move_t* movelist, board_t board, int turn)
{
    int i = 0;
    BitBoard frombb, tobb;
    move_t move;

    // 動かす
    for (int piece = 1; piece <= 10; piece++) {
        frombb = board.piecebb[playeridx(turn)][pieceidx(piece)];
        for (BitBoard f = frombb & -frombb; frombb; frombb ^= f, f = frombb & -frombb) {
            tobb = get_movable(board, piece, f, turn);
            for (BitBoard t = tobb & -tobb; tobb; tobb ^= t, t = tobb & -tobb) {
                move.from = f;
                move.to = t;
                move.piece = piece;
                if ((piece == FU || piece == KK || piece == HI) && (in_opp_area(f, turn) || in_opp_area(t, turn)))
                    move.promoting = 1; // 歩、角、飛は成れれば必ず成る
                else
                    move.promoting = 0;
                movelist[i++] = move;
                if (piece == GI && (in_opp_area(f, turn) || in_opp_area(t, turn))) { // 銀は成る手と成らない手を両方考慮する
                    move.from = f;
                    move.to = t;
                    move.piece = piece;
                    move.promoting = 1;
                    movelist[i++] = move;
                }
            }
        }
    }

    // 打つ
    BitBoard opp_all_movable = get_all_movable(board, -turn); // 相手の駒の利き
    for (int piece = 1; piece <= 6; piece++) {
        if (board.hand[playeridx(turn)][pieceidx(piece)]) {
            if (piece == FU)
                tobb = empty(board);
            else
                tobb = empty(board) & ~opp_all_movable; // 歩以外は相手に取られる場所には打たない
            for (BitBoard t = tobb & -tobb; tobb; tobb ^= t, t = tobb & -tobb) {
                move.from = 0; // 打つ場合は from = 0
                move.to = t;
                move.piece = piece;
                move.promoting = 0;
                if (piece == FU) {
                    if (in_opp_area(t, turn) // 敵陣に歩
                        || (board.piecebb[playeridx(turn)][pieceidx(FU)] & columnbb(t))) // 二歩
                        continue;
                    if (get_movable(board, piece, t, turn) & board.piecebb[playeridx(-turn)][pieceidx(OU)]) { // 王の前に歩
                        if (judge_tsumibb(do_move(board, move, turn), -turn)) // 打ち歩詰め
                            continue;
                    }
                }
                movelist[i++] = move;
            }
        }
    }

    return i;
}
