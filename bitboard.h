#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "global_const.h"

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
