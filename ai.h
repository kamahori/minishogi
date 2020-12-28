#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global_const.h"
#include "bitboard.h"
#include "sennichite.h"

// ランダムな動き
void compute_output(move_t* move)
{
    move_t movelist[150];
    int n = get_movelist(movelist, g_board);
    *move = movelist[rand() % n];
}
