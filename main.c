#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global_const.h"
#include "bitboard.h"
#include "sennichite.h"
#include "ai.h"

void init() {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            g_board.state[i][j] = EMPTY;
        }
    }
    g_board.state[0][0] = OU;
    g_board.state[0][1] = KI;
    g_board.state[0][2] = GI;
    g_board.state[0][3] = KK;
    g_board.state[0][4] = HI;
    g_board.state[1][0] = FU;

    g_board.state[4][4] = OU * (-1);
    g_board.state[4][3] = KI * (-1);
    g_board.state[4][2] = GI * (-1);
    g_board.state[4][1] = KK * (-1);
    g_board.state[4][0] = HI * (-1);
    g_board.state[3][4] = FU * (-1);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            g_board.hand[i][j] = 0;
        }
        for (int j = 0; j < 10; j++) {
            g_board.piecebb[i][j] = 0;
        }
    }
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            int piece = g_board.state[row][col];
            if (piece != EMPTY) {
                g_board.piecebb[playeridx(piece)][pieceidx(piece)] |= bitboard(row * 5 + col);
            }
        }
    }
    g_board.turn = P1;

    hash_init();
    g_board.state_h = hash_state();
    g_board.hand_h = hash_hand();
    st_init();
    st_insert();
    tt_init();
}

void print_piece(int piece_type) {
    if (piece_type == OU) {
        printf("OU");
    }
    else if (piece_type == KI) {
        printf("KI");
    }
    else if (piece_type == GI) {
        printf("GI");
    }
    else if (piece_type == promote(GI)) {
        printf("GI+");
    }
    else if (piece_type == KK) {
        printf("KK");
    }
    else if (piece_type == promote(KK)) {
        printf("KK+");
    }
    else if (piece_type == HI) {
        printf("HI");
    }
    else if (piece_type == promote(HI)) {
        printf("HI+");
    }
    else if (piece_type == FU) {
        printf("FU");
    }
    else if (piece_type == promote(FU)) {
        printf("FU+");
    }
}

void print() {
    printf("\n     A     B     C     D     E\n");
    printf("  _______________________________\n");
    for (int i = 0; i < 5; i++) {
        printf("%d |", 5 - i);
        for (int j = 0; j < 5; j++) {

            int piece = g_board.state[4 - i][j];
            if (piece > 0) {
                printf("  ");
            }
            else if (piece < 0) {
                printf("v ");
            }
            print_piece(abs(piece));
            if (piece == 0) {
                printf("     ");
            }
            else if (!ispromoted(piece)) {
                printf(" ");
            }

            printf("|");
        }
        printf("\n");
        printf("  |_____|_____|_____|_____|_____|\n");
    }

    printf("\nP1: ");
    int empty_check = 1;
    for (int i = 0; i < 6; i++) {
        int n = g_board.hand[playeridx(P1)][i];
        if (n > 0) {
            empty_check = 0;
            for (int j = 0; j < n; j++) {
                print_piece(i + 1);
                printf(" ");
            }
        }
    }
    if (empty_check == 1)
        printf("(no piece)");

    empty_check = 1;
    printf("\nP2: ");
    for (int i = 0; i < 6; i++) {
        int n = g_board.hand[playeridx(P2)][i];
        if (n > 0) {
            empty_check = 0;
            for (int j = 0; j < n; j++) {
                print_piece(i + 1);
                printf(" ");
            }
        }
    }
    if (empty_check == 1)
        printf("(no piece)");

    printf("\n\n");
}

int in_board(int i, int j) { return (0 <= i && i <= 4 && 0 <= j && j <= 4) ? 1 : 0; }
int possible(int i, int j, int turn) { return (g_board.state[i][j] == EMPTY || (g_board.state[i][j] * turn < 0)) ? 1 : 0; }

const int OU_MOVE[8] = { 0,1,2,3,4,5,6,7 };
const int KI_MOVE[6] = { 1,3,4,5,6,7 };
const int GI_MOVE[5] = { 0,2,5,6,7 };
const int KK_MOVE[4] = { 0,2,5,7 };
const int HI_MOVE[4] = { 1,3,4,6 };
const int FU_MOVE[1] = { 6 };
//  数字と方向の対応
//  -------------
//    5   6   7
//      ↖︎ ↑ ↗︎
//    3 ← ● → 4
//      ↙︎ ↓ ↘︎
//    0   1   2
//  -------------

void make_vector(int vec[2], int direction) {
    //数字を方向ベクトルに対応させる。
    int i_, j_;
    switch (direction) {
    case 0: i_ = -1; j_ = -1; break;
    case 1: i_ = -1; j_ = 0; break;
    case 2: i_ = -1; j_ = 1; break;
    case 3: i_ = 0; j_ = -1; break;
    case 4: i_ = 0; j_ = 1; break;
    case 5: i_ = 1; j_ = -1; break;
    case 6: i_ = 1; j_ = 0; break;
    case 7: i_ = 1; j_ = 1; break;
    }
    vec[0] = i_;
    vec[1] = j_;
}

void get_possible_movement(int movelist[13], int piece_type, int place, int turn)
{
    //ある駒に対して可能な動きをmovelistに格納する関数。
    //全ての駒の場合にある、
    //     if (turn==1) make_vector(vec, ??_MOVE[d]);
    //     else make_vector(vec, 7 - ??_MOVE[d]);
    //の部分は、相手ターンだったら、make_vectorで作る方向ベクトルをπ回転。

    int i, j;
    i = place / 5;
    j = place % 5;
    //printf("%d, %d\n", i, j);
    int cnt = 0;
    if (piece_type == OU) {
        for (int d = 0; d < 8; d++) {
            int vec[2];
            if (turn == P1) make_vector(vec, OU_MOVE[d]);
            else make_vector(vec, 7 - OU_MOVE[d]);
            if (in_board(i + vec[0], j + vec[1]) && possible(i + vec[0], j + vec[1], turn)) {
                movelist[cnt++] = 5 * (i + vec[0]) + j + vec[1];
            }
        }
    }
    else if (piece_type == KI || piece_type == promote(GI) || piece_type == promote(FU)) {
        for (int d = 0; d < 6; d++) {
            int vec[2];
            if (turn == P1) make_vector(vec, KI_MOVE[d]);
            else make_vector(vec, 7 - KI_MOVE[d]);
            if (in_board(i + vec[0], j + vec[1]) && possible(i + vec[0], j + vec[1], turn)) {
                movelist[cnt++] = 5 * (i + vec[0]) + j + vec[1];
            }
        }
    }
    else if (piece_type == GI) {
        for (int d = 0; d < 5; d++) {
            int vec[2];
            if (turn == P1) make_vector(vec, GI_MOVE[d]);
            else make_vector(vec, 7 - GI_MOVE[d]);
            if (in_board(i + vec[0], j + vec[1]) && possible(i + vec[0], j + vec[1], turn)) {
                movelist[cnt++] = 5 * (i + vec[0]) + j + vec[1];
            }
        }
    }
    else if (piece_type == KK || piece_type == promote(KK)) {
        for (int d = 0; d < 4; d++) {
            int vec[2];
            if (turn == P1) make_vector(vec, KK_MOVE[d]);
            else make_vector(vec, 7 - KK_MOVE[d]);
            int n = 1;
            while (in_board(i + n * vec[0], j + n * vec[1]) && possible(i + n * vec[0], j + n * vec[1], turn)) {
                movelist[cnt++] = 5 * (i + n * vec[0]) + j + n * vec[1];
                if (g_board.state[i + n * vec[0]][j + n * vec[1]] != 0) break;
                n++;
            }
        }
        if (piece_type == promote(KK)) {
            for (int d = 0; d < 4; d++) {
                int vec[2];
                if (turn == P1) make_vector(vec, HI_MOVE[d]);
                else make_vector(vec, 7 - HI_MOVE[d]);
                if (in_board(i + vec[0], j + vec[1]) && possible(i + vec[0], j + vec[1], turn)) {
                    movelist[cnt++] = 5 * (i + vec[0]) + j + vec[1];
                }
            }
        }
    }
    else if (piece_type == HI || piece_type == promote(HI)) {
        for (int d = 0; d < 4; d++) {
            int vec[2];
            if (turn == P1) make_vector(vec, HI_MOVE[d]);
            else make_vector(vec, 7 - HI_MOVE[d]);
            int n = 1;
            while (in_board(i + n * vec[0], j + n * vec[1]) && possible(i + n * vec[0], j + n * vec[1], turn)) {
                movelist[cnt++] = 5 * (i + n * vec[0]) + j + n * vec[1];
                if (g_board.state[i + n * vec[0]][j + n * vec[1]] != 0) break;
                n++;
            }
        }
        if (piece_type == promote(HI)) {
            for (int d = 0; d < 4; d++) {
                int vec[2];
                if (turn == P1) make_vector(vec, KK_MOVE[d]);
                else make_vector(vec, 7 - KK_MOVE[d]);
                if (in_board(i + vec[0], j + vec[1]) && possible(i + vec[0], j + vec[1], turn)) {
                    movelist[cnt++] = 5 * (i + vec[0]) + j + vec[1];
                }
            }
        }
    }
    else if (piece_type == FU) {
        int vec[2];
        if (turn == P1) make_vector(vec, FU_MOVE[0]);
        else make_vector(vec, 7 - FU_MOVE[0]);
        if (in_board(i + vec[0], j + vec[1]) && possible(i + vec[0], j + vec[1], turn)) {
            movelist[cnt++] = 5 * (i + vec[0]) + j + vec[1];
        }
    }
    movelist[cnt] = -1;
}

int judge_check(int turn) {
    //王手かどうかを判定する関数
    //王手でなければ0、王手であれば1、を返す。
    //turn のプレイヤーが王手をしているか。
    int opposite_ou_place; //相手の王の場所
    // judge_tsumi() では g_board.piecebb の値は更新しないため g_board.piecebb から直接王の位置を求めることはできない
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (g_board.state[i][j] == OU * -turn) {
                opposite_ou_place = 5 * i + j;
            }
        }
    }
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (g_board.state[i][j] * turn > 0) {
                //自分の駒があったら、
                int movelist[13];
                get_possible_movement(movelist, abs(g_board.state[i][j]), 5 * i + j, turn);
                int index = 0;
                while (movelist[index] != -1) {
                    if (opposite_ou_place == movelist[index]) {
                        //printf("CHECKMATE!!");
                        return 1;
                    }
                    index++;
                }
            }
        }
    }
    return 0;
}

int judge_tsumi(int turn)
{
    //自分が詰みだったら1, そうでなければ0、を返す。
    int res = 1;
    //どの駒を動かしても王手。
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (g_board.state[i][j] * turn > 0) {
                //自分の駒が見つかったら、
                int movelist[13];
                get_possible_movement(movelist, abs(g_board.state[i][j]), 5 * i + j, turn);
                //可能な動きを求めて、
                int k = 0;
                while (movelist[k] != -1) {
                    //それぞれの動きに対して、王手かどうかを判定。
                    int temp;
                    int i_, j_;
                    i_ = movelist[k] / 5;
                    j_ = movelist[k] % 5;
                    temp = g_board.state[i_][j_];
                    g_board.state[i_][j_] = g_board.state[i][j];
                    g_board.state[i][j] = EMPTY;
                    //動かしてみて
                    if (!judge_check(-turn)) {
                        //相手からみて王手でないなら
                        res = 0;
                        //詰みでない。
                    }
                    g_board.state[i][j] = g_board.state[i_][j_];
                    g_board.state[i_][j_] = temp;
                    if (res == 0) return 0;
                    k++;
                }
            }
        }
    }
    //どの持ち駒を配置しても王手。
    for (int k = 0; k < 6; k++) {
        if (g_board.hand[playeridx(turn)][k] > 0) {
            //piece_typeがk+1の駒が、自分の持ち駒にあったら、
            int piecetype = k + 1;
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 5; j++) {
                    if (g_board.state[i][j] == EMPTY) {
                        //EMPTYの場所があったら、
                        g_board.state[i][j] = piecetype * turn;
                        //自分のコマをおいてみて
                        if (!judge_check(-turn)) {
                            //もし相手から見て王手でなかったら詰みでない。
                            res = 0;
                        }
                        g_board.state[i][j] = EMPTY;
                        //盤面を元に戻す。
                        if (res == 0) return 0;
                    }
                }
            }
        }
    }

    //どのパターンでも王手でなかったら詰み。
    return 1;
}

int is_finished() {
    return g_board.hand[0][pieceidx(OU)] > 0 || g_board.hand[1][pieceidx(OU)] > 0;
}

// 無効な動きなら-1、有効な動きなら move に動きを書きこんで0を返す
int move_piece(move_t* move, char input[]) {
    int mode = 0, //0:動かす, 1:駒を打つ
        drop = 0; // 打つ駒(あれば)
    int turn = g_board.turn;

    /* 課題の説明で挙げられていた条件
    - 動かすとき
    -- 駒の動かし方のルール違反
    -- 存在しない駒を動かそうとする
    -- 歩を成らない
    -- 駒を動かしたことで自らの王が王手状態になる
    - 打つとき
    -- 持ち駒にない駒を打とうとする
    -- 二歩
    -- 打ち歩詰め
    -- 敵陣に持ち駒の歩を打つ
    - 両方
    -- 王手放置
    */

    // 4or5文字以外の入力の場合反則
    int input_length = strlen(input);
    if (input_length < 4 || 5 < input_length) {
        printf("error: wrong input\n");
        return -1;
    }

    // 移動元or打つ先を指定できていなければ反則
    if (!('1' <= input[0] && input[0] <= '5' && 'A' <= input[1] && input[1] <= 'E')) {
        printf("error: wrong input\n");
        return -1;
    }

    //3,4文字目のチェック
    if (!('1' <= input[2] && input[2] <= '5' && 'A' <= input[3] && input[3] <= 'E')) {
        mode = 1; //3,4文字目が「2A」のようになっていない場合、打つモード

        //駒を打ちながら成ったら反則
        if (input_length == 5) {
            printf("violation: Don't promote a piece when you drop it.\n");
            return -1;
        }

        //打つ駒の指定ができていなければ反則
        if (input[2] == 'H' && input[3] == 'I') {
            drop = HI;
        }
        else if (input[2] == 'K' && input[3] == 'K') {
            drop = KK;
        }
        else if (input[2] == 'K' && input[3] == 'I') {
            drop = KI;
        }
        else if (input[2] == 'G' && input[3] == 'I') {
            drop = GI;
        }
        else if (input[2] == 'F' && input[3] == 'U') {
            drop = FU;
        }
        else {
            printf("error: wrong input\n");
            return -1;
        }
    }

    //5文字目が存在したとしたら、Nでなければ反則
    if (input_length == 5 && input[4] != 'N') {
        printf("error: wrong input\n");
        return -1;
    }


    /* inputの内容に関するチェック */
    if (mode == 0) { // 動かす場合
        int prev_row = input[0] - '1';
        int prev_col = input[1] - 'A';
        int next_row = input[2] - '1';
        int next_col = input[3] - 'A';
        int prev_square = prev_row * 5 + prev_col;
        int next_square = next_row * 5 + next_col;
        int piece = g_board.state[prev_row][prev_col];
        int target = g_board.state[next_row][next_col];

        //成ることができるかどうか
        int is_promotable = 0;
        if ((turn == P1 && (prev_row == 4 || next_row == 4)) || (turn == P2 && (prev_row == 0 || next_row == 0)))
            is_promotable = 1;

        // 移動元にコマがないなら反則
        if (g_board.state[prev_row][prev_col] == EMPTY) {
            printf("violation: Don't choose an empty square.\n");
            return -1;
        }

        // コマを動かさないなら反則
        if (prev_row == next_row && prev_col == next_col) {
            printf("violation: You can't choose do-nothing move.\n");
            return -1;
        }

        // 相手のコマを動かそうとしたら反則
        if (turn * piece < 0) {
            printf("violation: Don't move your opponent's piece.\n");
            return -1;
        }

        // 動かした先が自分の駒なら反則
        if (turn * target > 0) {
            printf("violation: Don't take your own piece.\n");
            return -1;
        }

        // 動かす経路に駒が存在したら反則
        int diff_row = next_row - prev_row, diff_col = next_col - prev_col; //縦横それぞれの差分
        if (diff_row != 0 && diff_col != 0 && abs(diff_row) != abs(diff_col)) { //縦横斜め以外(桂馬飛びなど)を除外
            printf("violation: You can't move a piece in this way.\n");
            return -1;
        }
        int dist = max(abs(diff_col), abs(diff_row)); //移動量
        int e_row = diff_row / dist, e_col = diff_col / dist; //移動する向きへの単位成分
        for (int i = 1; i < dist; i++) {
            if (g_board.state[prev_row + e_row * i][prev_col + e_col * i] != EMPTY) {
                printf("violation: Don't move a piece in the way it jumps over another.\n");
                return -1;
            }
        }

        //駒の種類からしてアウトな動かし方なら反則、そうでなければ動かす
        int piece_type = abs(piece);
        if (piece_type == OU) { //全方向1マスずつ
            if (dist != 1) {
                printf("violation: You can't move OU in this way.\n");
                return -1;
            }
        }
        else if (piece_type == KI) { //斜後ろを除き1マスずつ
            if (dist != 1 || ((diff_row * turn < 0) && (diff_col != 0))) {
                printf("violation: You can't move KI in this way.\n");
                return -1;
            }
        }
        else if (piece_type == GI) { //横と真後を除き1マスずつ
            if (dist != 1 || diff_row == 0 || (diff_row * turn < 0 && diff_col == 0)) {
                printf("violation: You can't move GI in this way.\n");
                return -1;
            }
        }
        else if (piece_type == promote(GI)) { //斜後ろを除き1マスずつ
            if (dist != 1 || ((diff_row * turn < 0) && (diff_col != 0))) {
                printf("violation: You can't move promoted GI in this way.\n");
                return -1;
            }
        }
        else if (piece_type == KK) { //斜めだけ
            if (abs(diff_row) != abs(diff_col)) {
                printf("violation: You can't move KK in this way.\n");
                return -1;
            }
        }
        else if (piece_type == promote(KK)) { //斜め + 周囲1マスずつ
            if (abs(diff_row) != abs(diff_col) && dist != 1) {
                printf("violation: You can't move promoted KK in this way.\n");
                return -1;
            }
        }
        else if (piece_type == HI) { //縦横だけ
            if (diff_row * diff_col != 0) {
                printf("violation: You can't move HI in this way.\n");
                return -1;
            }
        }
        else if (piece_type == promote(HI)) { //縦横 + 周囲1マスずつ
            if (diff_row * diff_col != 0 && dist != 1) {
                printf("violation: You can't move promoted HI in this way.\n");
                return -1;
            }
        }
        else if (piece_type == FU) { //1つ前のみ
            if (diff_row * turn != 1 || diff_col != 0) {
                printf("violation: You can't move FU in this way.\n");
                return -1;
            }
            //歩が成れる状況で成らなかった場合、反則
            if (is_promotable == 1 && input_length == 4) {
                printf("violation: You must promote FU when possible.\n");
                return -1;
            }
        }
        else if (piece_type == promote(FU)) {
            if (dist != 1 || ((diff_row * turn < 0) && (diff_col != 0))) {
                printf("violation: You can't move promoted FU in this way.\n");
                return -1;
            }
        }

        move->from = bitboard(prev_square);
        move->to = bitboard(next_square);
        move->piece = abs(piece);
        move->take = abs(target);

        /* 成る場合 */
        if (input_length == 5) {
            //成れない手の場合は反則
            if (is_promotable == 0) {
                printf("violation: You can't promote the piece in this move.\n");
                return -1;
            }

            //既に成っている場合は反則
            if (piece >= 10) {
                printf("violation: The piece has already been promoted.\n");
                return -1;
            }

            move->promoting = 1;
        }
        else {
            move->promoting = 0;
        }

    }
    else { // 打つ場合
        int drop_row = input[0] - '1';
        int drop_col = input[1] - 'A';
        int drop_square = drop_row * 5 + drop_col;

        // 打つ先に駒があるなら反則
        if (g_board.state[drop_row][drop_col] != EMPTY) {
            printf("violation: Don't drop a piece where there is one.\n");
            return -1;
        }

        // 持っていない駒を打つと反則
        if (g_board.hand[playeridx(turn)][pieceidx(drop)] == 0) {
            printf("violation: You can't drop a piece which you haven't captured.\n");
            return -1;
        }

        // 歩関連の反則
        if (drop == FU) {
            // 二歩
            int double_FU = 0;
            for (int i = 0; i < 4; i++) {
                if (g_board.state[i][drop_col] == FU * turn)
                    double_FU = 1;
            }
            if (double_FU == 1) {
                printf("violation: Don't drop FU in the column with your unpromoted FU.\n");
                return -1;
            }

            // 敵陣に持ち駒の歩を打つと反則
            if ((turn == P1 && drop_row == 4) || (turn == P2 && drop_row == 0)) {
                printf("violation: Don't drop FU in opponent's field.\n");
                return -1;
            }

            // 打ち歩詰めの判定は動かした後で
        }

        move->from = 0;
        move->to = bitboard(drop_square);
        move->piece = drop;
        move->take = EMPTY;
        move->promoting = 0;
    }

    // 王手放置の判定は外で

    return 0;
}

// 王手放置判定
// 現在の手番のプレイヤーが王手をしているか
int judge_outehochi() {
    if (judge_check(g_board.turn)) {
        //王手を放置したら、あるいは自ら自分の王を王手にしたら
        printf("violation: Don't neglect checkmate.\n");
        return 1;
    }
    return 0;
}

// 打ち歩詰め判定
// 歩を打った後で現在の手番のプレイヤーが詰んでいるか
int judge_uchifuzume(move_t move) {
    if (move.from == 0 && move.piece == FU && judge_tsumi(g_board.turn)) {
        // 歩を打って相手が詰んでいたら
        printf("violation: Don't drop FU to TSUMI.\n");
        return 1;
    }
    return 0;
}

void print_move(move_t move)
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

void youWin() {
    printf("You Win\n");
    exit(0);
}

void youLose() {
    printf("You Lose\n");
    exit(0);
}

void Even() {
    printf("Even\n");
    exit(0);
}


int main(int argc, char* argv[]) {
    init();
    int cnt = 0, res, USER_turn, loser;
    char input[100];
    move_t move;

    if (argc >= 3 && !strcmp(argv[2], "1")) DEBUG = 1;

    if (!strcmp(argv[1], "0")) {
        // human plays first
        USER = P1;
        AI = P2;
        USER_turn = 0;
    }
    else {
        // computer plays first
        AI = P1;
        USER = P2;
        USER_turn = 1;
    }

    if (DEBUG) print();
    while (1) {
        if (cnt % 2 == USER_turn) {
            // human's turn
            if (DEBUG) printf("human(P%d)'s turn: ", (USER == P1) ? 1 : 2);
            scanf("%s", input);
            res = move_piece(&move, input);
            if (res == -1) youLose();
            do_move(move, 1);
            if (DEBUG) print();
            loser = judge_sennichite();
            if (loser) {
                if (DEBUG) printf("sennichite\n");
                if (loser == USER) youLose();
                else youWin();
            }
            st_insert();
            sl_prepend();
            if (judge_outehochi()) youLose();
            if (judge_uchifuzume(move)) youLose();
            if (judge_tsumi(AI)) youWin();
            if (is_finished()) youWin();
        }
        else {
            // computer's turn
            if (DEBUG) printf("computer(P%d)'s turn: ", (AI == P1) ? 1 : 2);
            res = compute_output(&move);
            if (res == -1) youWin();
            do_move(move, 1);
            if (DEBUG) print();
            loser = judge_sennichite();
            if (loser) {
                if (DEBUG) printf("sennichite\n");
                if (loser == USER) youLose();
                else youWin();
            }
            st_insert();
            sl_prepend();
            if (judge_outehochi()) youWin();
            if (judge_uchifuzume(move)) youWin();
            if (judge_tsumi(USER)) youLose();
            if (is_finished()) youLose();
        }
        cnt++;
        if (cnt >= 150) Even();
    }

    return 0;
}
