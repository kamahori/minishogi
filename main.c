#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int DEBUG = 0;

//P1: 先手, P2: 後手
const int P1 = 1;
const int P2 = -1; //2;

const int EMPTY = 0;
const int OU = 1; // 王
const int KI = 2; // 金
const int GI = 3; // 銀
const int KK = 4; // 角
const int HI = 5; // 飛
const int FU = 6; // 歩
// P1: 正, P2: 負
// 成り: 10x

typedef struct board {
    int state[5][5]; // 盤面全体の情報
    
    int P1[6]; //P1の駒の場所
    int P2[6]; //P2の駒の場所
} board;

board g_board;
// global variable to contain current state

// P2の持ち駒: -2
//
//     A  B  C  D  E
//   ---------------
// 5| 20 21 22 23 24
// 4| 15 16 17 18 19
// 3| 10 11 12 13 14
// 2|  5  6  7  8  9
// 1|  0  1  2  3  4
//
//      P1の持ち駒: -1

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

    g_board.P1[OU-1] = 0;
    g_board.P1[KI-1] = 1;
    g_board.P1[GI-1] = 2;
    g_board.P1[KK-1] = 3;
    g_board.P1[HI-1] = 4;
    g_board.P1[FU-1] = 5;
    
    g_board.P2[OU-1] = 24;
    g_board.P2[KI-1] = 23;
    g_board.P2[GI-1] = 22;
    g_board.P2[KK-1] = 21;
    g_board.P2[HI-1] = 20;
    g_board.P2[FU-1] = 19;
}

void print_piece(int piece_type){
    if(piece_type == OU){
        printf("OU");
    }else if(piece_type == KI){
        printf("KI");
    }else if(piece_type == GI){
        printf("GI");
    }else if(piece_type == GI*10){
        printf("GI+");
    }else if(piece_type == KK){
        printf("KK");
    }else if(piece_type == KK*10){
        printf("KK+");
    }else if(piece_type == HI){
        printf("HI");
    }else if(piece_type == HI*10){
        printf("HI+");
    }else if(piece_type == FU){
        printf("FU");
    }else if(piece_type == FU*10){
        printf("FU+");
    }
}

void print() {
    if (DEBUG == 0) return;

    printf("\n     A     B     C     D     E\n");
    printf("  _______________________________\n");
    for (int i = 0; i < 5; i++) {
        printf("%d |", 5 - i);
        for (int j = 0; j < 5; j++) {

            int piece = g_board.state[4 - i][j];
            if(piece>0){
                printf("▲ ");
            }else if(piece<0){
                printf("▼ ");
            }
            print_piece(abs(piece));
            if(piece == 0){
                printf("     ");
            }else if(abs(piece) < 10){
                printf(" ");
            }

            printf("|");
        }
        printf("\n");
        printf("  |_____|_____|_____|_____|_____|\n");
    }

    printf("\nP1(▲): ");
    int empty_check = 1;
    for(int i=0;i<6;i++){
        if(g_board.P2[i]==-1){
            empty_check = 0;
            print_piece(i+1);
            printf(" ");
        }
    }
    if(empty_check == 1)
        printf("(no piece)");
        
    empty_check = 1;
    printf("\nP2(▼): ");
    for(int i=0;i<6;i++){
        if(g_board.P1[i]==-2){
            empty_check = 0;
            print_piece(i+1);
            printf(" ");
        }
    }
    if(empty_check == 1)
        printf("(no piece)");

    printf("\n\n");
}

int abs(int x){return (x>0)? x:-x;}
int in_board(int i, int j){return (0 <= i && i <= 4 && 0 <= j && j <= 4)? 1:0;}
int possible(int i, int j, int turn){return (g_board.state[i][j] == 0 || (g_board.state[i][j]*turn < 0))? 1:0;}

const int OU_MOVE[8] = {0,1,2,3,4,5,6,7};
const int KI_MOVE[6] = {1,3,4,5,6,7};
const int GI_MOVE[5] = {0,2,5,6,7};
const int KK_MOVE[4] = {0,2,5,7};
const int HI_MOVE[4] = {1,3,4,6};
const int FU_MOVE[1] = {6};
//  数字と方向の対応
//  -------------
//    5   6   7
//      ↖︎ ↑ ↗︎
//    3 ← ● → 4
//      ↙︎ ↓ ↘︎
//    0   1   2
//  -------------

void make_vector(int vec[2], int direction){
    //数字を方向ベクトルに対応させる。
    int i_, j_;
    switch (direction){
        case 0 : i_=-1; j_=-1; break;
        case 1 : i_=-1; j_= 0; break;
        case 2 : i_=-1; j_= 1; break;
        case 3 : i_= 0; j_=-1; break;
        case 4 : i_= 0; j_= 1; break;
        case 5 : i_= 1; j_=-1; break;
        case 6 : i_= 1; j_= 0; break;
        case 7 : i_= 1; j_= 1; break;
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
    if (piece_type == OU){
        for (int d=0; d<8; d++){
            int vec[2];
            if (turn==1) make_vector(vec, OU_MOVE[d]);
            else make_vector(vec, 7 - OU_MOVE[d]);
            if(in_board(i+vec[0], j+vec[1]) && possible(i+vec[0], j+vec[1], turn)){
                movelist[cnt++] = 5*(i+vec[0]) +j+vec[1];
            }
        }
    }
    else if (piece_type == KI || piece_type == GI*10||piece_type == FU*10){
        for (int d=0; d<6; d++){
            int vec[2];
            if (turn==1) make_vector(vec, KI_MOVE[d]);
            else make_vector(vec, 7 - KI_MOVE[d]);
            if(in_board(i+vec[0], j+vec[1]) && possible(i+vec[0], j+vec[1], turn)){
                movelist[cnt++] = 5*(i+vec[0]) +j+vec[1];
            }
        }
    }
    else if (piece_type == GI){
        for (int d=0; d<5; d++){
            int vec[2];
            if (turn==1) make_vector(vec, GI_MOVE[d]);
            else make_vector(vec, 7 - GI_MOVE[d]);
            if(in_board(i+vec[0], j+vec[1]) && possible(i+vec[0], j+vec[1], turn)){
                movelist[cnt++] = 5*(i+vec[0]) +j+vec[1];
            }
        }
    }
    else if (piece_type == KK || piece_type == KK*10){
        for (int d=0; d<4; d++){
            int vec[2];
            if (turn==1) make_vector(vec, KK_MOVE[d]);
            else make_vector(vec, 7 - KK_MOVE[d]);
            int n=1;
            while (in_board(i+n*vec[0], j+n*vec[1]) && possible(i+n*vec[0], j+n*vec[1], turn)){
                movelist[cnt++] = 5*(i+n*vec[0]) +j+n*vec[1];
                if (g_board.state[i+n*vec[0]][j+n*vec[1]] != 0) break;
                n++;
            }
        }
        if (piece_type == KK*10){
            for (int d=0; d<4; d++){
                int vec[2];
                if (turn==1) make_vector(vec, HI_MOVE[d]);
                else make_vector(vec, 7 - HI_MOVE[d]);
                if(in_board(i+vec[0], j+vec[1]) && possible(i+vec[0], j+vec[1], turn)){
                    movelist[cnt++] = 5*(i+vec[0]) +j+vec[1];
                }
            }
        }
    }
    else if (piece_type == HI || piece_type == HI*10){
        for (int d=0; d<4; d++){
            int vec[2];
            if (turn==1) make_vector(vec, HI_MOVE[d]);
            else make_vector(vec, 7 - HI_MOVE[d]);
            int n=1;
            while (in_board(i+n*vec[0], j+n*vec[1]) && possible(i+n*vec[0], j+n*vec[1], turn)){
                movelist[cnt++] = 5*(i+n*vec[0]) +j+n*vec[1];
                if (g_board.state[i+n*vec[0]][j+n*vec[1]] != 0) break;
                n++;
            }
        }
        if (piece_type == HI*10){
            for (int d=0; d<4; d++){
                int vec[2];
                if (turn==1) make_vector(vec, KK_MOVE[d]);
                else make_vector(vec, 7 - KK_MOVE[d]);
                if(in_board(i+vec[0], j+vec[1]) && possible(i+vec[0], j+vec[1], turn)){
                    movelist[cnt++] = 5*(i+vec[0]) +j+vec[1];
                }
            }
        }
    }
    else if (piece_type == FU){
        int vec[2];
        if (turn==1) make_vector(vec, FU_MOVE[0]);
        else make_vector(vec, 7 - FU_MOVE[0]);
        if(in_board(i+vec[0], j+vec[1]) && possible(i+vec[0], j+vec[1], turn)){
            movelist[cnt++] = 5*(i+vec[0]) +j+vec[1];
        }
    }
    movelist[cnt] = '\0';
}

int judge_check(int turn){
    //王手かどうかを判定する関数
    //王手でなければ0、王手であれば1、を返す。
    //turn のプレイヤーが王手をしているか。
    int opposite_ou_place;
    opposite_ou_place = (turn == 1)? g_board.P2[OU-1]: g_board.P1[OU-1]; //相手の王の場所
    for (int i=0; i<5; i++){
        for (int j=0; j<5; j++){
            if (g_board.state[i][j]*turn > 0){
                //自分の駒があったら、
                int movelist[13];
                get_possible_movement(movelist, abs(g_board.state[i][j]), 5*i+j, turn);
                int index=0;
                while (movelist[index] != '\0'){
                    if  (opposite_ou_place == movelist[index]){
                        printf("CHECKMATE!!");
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
    //相手が詰みだったら1, そうでなければ0、を返す。
    int res = 1;
    //どの駒を動かしても王手。
    for(int i=0; i<5; i++){
        for (int j=0; j<5; j++){
            if (g_board.state[i][j]*turn < 0){
                //相手の駒が見つかったら、
                int movelist[13];
                get_possible_movement(movelist, abs(g_board.state[i][j]), 5*i+j, -turn);
                //可能な動きを求めて、
                int k=0;
                while (movelist[k] != '\0'){
                    //それぞれの動きに対して、王手かどうかを判定。
                    int temp;
                    int i_, j_;
                    i_ = movelist[k] / 5;
                    j_ = movelist[k] % 5;
                    temp = g_board.state[i_][j_];
                    g_board.state[i_][j_] = abs(g_board.state[i][j]);
                    g_board.state[i][j] = 0;
                    //動かしてみて
                    if (!judge_check(-turn)){
                        //相手からみて王手でないなら
                        res=0;
                        //詰みでない。
                        
                    }
                    g_board.state[i][j] = g_board.state[i_][j_];
                    g_board.state[i_][j_] = temp;
                    if (res==0) return 0;
                    k++;
                }
            }
        }
    }
    //どの持ち駒を配置しても王手。
    if (turn == 1){ //プレイヤー１のターンだったら
        for (int k=0; k<6; k++){
            if (g_board.P1[k] == -2 || g_board.P2[k] == -2){
                //piece_typeがk+1の駒が、k+1がP2の持ち駒にあったら、
                int piecetype = k+1;
                for (int i=0; i<5; i++){
                    for (int j=0; j<5; j++){
                        if (g_board.state[i][j] == EMPTY){
                            //EMPTYの場所があったら、
                            g_board.state[i][j] = -piecetype;
                            //相手(P2)のコマをおいてみて
                            if (!judge_check(-turn)){
                                //もし王手でなかったら詰みでない。
                                res = 0;
                            }
                            g_board.state[i][j] = 0;
                            //盤面を元に戻す。
                        }
                    }
                }
            }
        }
    }
    else{ // プレイヤー２のターンだったら
        for (int k=0; k<6; k++){
            if (g_board.P1[k] == -1 || g_board.P2[k] == -1){
                //piece_typeがk+1の駒が、P1の持ち駒にあったら、
                int piecetype = k+1;
                for (int i=0; i<5; i++){
                    for (int j=0; j<5; j++){
                        if (g_board.state[i][j] == 0){
                            //EMPTYの場所があったら、
                            g_board.state[i][j] = piecetype;
                            //相手(P1)のコマをおいてみて
                            if (!judge_check(-turn)){
                                //もし王手でなかったら詰みでない。
                                res = 0;
                            }
                            g_board.state[i][j] = 0;
                            //盤面を元に戻す。
                        }
                    }
                }
            }
        }
    }
    //どのパターンでも王手でなかったら詰み。
    return res;
}

int move_piece(char input[], int turn) {
    int mode = 0, //0:動かす, 1:駒を打つ
    drop = 0; // 打つ駒(あれば)
    int* piece_position = turn==P1?g_board.P1:g_board.P2;
    int* opp_piece_position = turn==P1?g_board.P2:g_board.P1;
    int wasChecked = judge_check(-turn);

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
    if (input_length < 4 || 5 < input_length){
        printf("error: wrong input\n");
        return -1;
    }

    // 移動元or打つ先を指定できていなければ反則
    if (!('1' <= input[0] && input[0] <= '5' && 'A' <= input[1] && input[1] <= 'E')){
        printf("error: wrong input\n");
        return -1;
    }

    //3,4文字目のチェック
    if (!('1' <= input[2] && input[2] <= '5' && 'A' <= input[3] && input[3] <= 'E')){
        mode = 1; //3,4文字目が「2A」のようになっていない場合、打つモード

        //駒を打ちながら成ったら反則
        if(input_length == 5){
            printf("violation: Don't promote a piece when you drop it.\n");
            return -1;
        }

        //打つ駒の指定ができていなければ反則
        if(input[2]=='H'&&input[3]=='I'){
            drop = HI;
        }else if(input[2]=='K'&&input[3]=='K'){
            drop = KK;
        }else if(input[2]=='K'&&input[3]=='I'){
            drop = KI;
        }else if(input[2]=='G'&&input[3]=='I'){
            drop = GI;
        }else if(input[2]=='F'&&input[3]=='U'){
            drop = FU;
        }else{
            printf("error: wrong input\n");
            return -1;
        }
    }

    //5文字目が存在したとしたら、Nでなければ反則
    if(input_length == 5 && input[4] != 'N'){
        printf("error: wrong input\n");
        return -1;
    }


    /* inputの内容に関するチェック */
    if(mode == 0){ // 動かす場合
        int prev_row = input[0] - '1';
        int prev_col = input[1] - 'A';
        int next_row = input[2] - '1';
        int next_col = input[3] - 'A';
        int piece = g_board.state[prev_row][prev_col];
        int target = g_board.state[next_row][next_col];

        //成ることができるかどうか
        int is_promotable = 0;
        if((turn==P1 && (prev_row>2 || next_row>2)) || (turn==P2 && (prev_row<2 || next_row<2)))
            is_promotable = 1;

        // 移動元にコマがないなら反則
        if (g_board.state[prev_row][prev_col] == EMPTY){
            printf("violation: Don't choose an empty square.\n");
            return -1;
        }

        // コマを動かさないなら反則
        if (prev_row == next_row && prev_col == next_col){
            printf("violation: You can't choose do-nothing move.\n");
            return -1;
        }

        // 相手のコマを動かそうとしたら反則
        if(turn * piece < 0){
            printf("violation: Don't move your opponent's piece.\n");
            return -1;
        }

        // 動かした先が自分の駒なら反則
        if(turn * target > 0){
            printf("violation: Don't take your own piece.\n");
            return -1;
        }

        // 動かす経路に駒が存在したら反則
        int diff_row = next_row - prev_row, diff_col = next_col - prev_col; //縦横それぞれの差分
        if(diff_row!=0 && diff_col!=0 && abs(diff_row)!=abs(diff_col)){ //縦横斜め以外(桂馬飛びなど)を除外
            printf("violation: You can't move a piece in this way.\n");
            return -1;
        }
        int dist = abs(diff_col)>abs(diff_row)?abs(diff_col):abs(diff_row); //移動量
        int e_row = diff_row/dist, e_col = diff_col/dist; //移動する向きへの単位成分
        for(int i=1; i<dist; i++){
            if(g_board.state[prev_row+e_row*i][prev_col+e_col*i] != EMPTY){
                printf("violation: Don't move a piece in the way it jumps over another.\n");
                return -1;
            }
        }

        
        //駒の種類からしてアウトな動かし方なら反則、そうでなければ動かす
        int piece_type = abs(piece);
        int next_square = abs(diff_row)*5 + abs(diff_col);
        if(piece_type == OU){ //全方向1マスずつ
            if(dist != 1){
                printf("violation: You can't move OU in this way.\n");
                return -1;
            }
            piece_position[OU-1] = next_square;
        }else if(piece_type == KI){ //斜後ろを除き1マスずつ
            if(dist != 1 || ( (diff_row * turn < 0) && (diff_col != 1) )){
                printf("violation: You can't move KI in this way.\n");
                return -1;
            }
            piece_position[KI-1] = next_square;
        }else if(piece_type == GI){ //横と真後を除き1マスずつ
            if(dist != 1 || diff_row == 0 || diff_row * turn < 0){
                printf("violation: You can't move GI in this way.\n");
                return -1;
            }
            piece_position[GI-1] = next_square;
        }else if(piece_type == GI*10){ //斜後ろを除き1マスずつ
            if(dist != 1 || ( (diff_row * turn < 0) && (diff_col != 1) )){
                printf("violation: You can't move promoted GI in this way.\n");
                return -1;
            }
            piece_position[GI-1] = next_square;
        }else if(piece_type == KK){ //斜めだけ
            if(abs(diff_row) != abs(diff_col)){
                printf("violation: You can't move KK in this way.\n");
                return -1;
            }
            piece_position[KK-1] = next_square;
        }else if(piece_type == KK*10){ //斜め + 周囲1マスずつ
            if(abs(diff_row) != abs(diff_col) && dist != 1){
                printf("violation: You can't move promoted KK in this way.\n");
                return -1;
            }
            piece_position[KK-1] = next_square;
        }else if(piece_type == HI){ //縦横だけ
            if(diff_row * diff_col != 0){
                printf("violation: You can't move HI in this way.\n");
                return -1;
            }
            piece_position[HI-1] = next_square;
        }else if(piece_type == HI*10){ //縦横 + 周囲1マスずつ
            if(diff_row * diff_col != 0 && dist != 1){
                printf("violation: You can't move promoted HI in this way.\n");
                return -1;
            }
            piece_position[HI-1] = next_square;
        }else if(piece_type == FU){ //1つ前のみ
            if(diff_row * turn != 1){
                printf("violation: You can't move FU in this way.\n");
                return -1;
            }
            //歩が成れる状況で成らなかった場合、反則
            if(is_promotable == 1 && input_length == 4){
                printf("violation: You must promote FU when possible.\n");
                return -1;
            }
            piece_position[FU-1] = next_square;
        }else if(piece_type == FU*10){
            if(dist != 1 || ( (diff_row * turn < 0) && (diff_col != 1) )){
                printf("violation: You can't move promoted FU in this way.\n");
                return -1;
            }
            piece_position[FU-1] = next_square;
        }

        //元々の場所を空にする
        g_board.state[prev_row][prev_col] = EMPTY;
        g_board.state[next_row][next_col] = piece;

        //駒を取る
        int target_type = abs(target);
        if(target_type>=10) target_type /= 10;
        /*
        if(target != EMPTY){
            opp_piece_position[target_type-1] = turn==P1?-1:-2;
        }*/
        for (int k=0; k<6; k++){
            if (g_board.P1[k] == abs(target)){
                g_board.P1[k] = turn==P1?-1:-2;
            }
        }
        for (int k=0; k<6; k++){
            if (g_board.P2[k] == abs(target)){
                g_board.P2[k] = turn==P1?-1:-2;
            }
        }


        /* 成る場合 */
        if(input_length == 5){
            //成れない手の場合は反則
            if(is_promotable == 0){
                printf("violation: You can't promote the piece in this move.\n");
                return -1;
            }

            //既に成っている場合は反則
            if(piece>=10){
                printf("violation: The piece has already been promoted.\n");
                return -1;
            }

            //実際に成る
            g_board.state[next_row][next_col] *= 10;
        }

    }else{ // 打つ場合
        int drop_row = input[0] - '1';
        int drop_col = input[1] - 'A';

        // 打つ先に駒があるなら反則
        if(g_board.state[drop_row][drop_col] != EMPTY){
            printf("violation: Don't drop a piece where there is one.\n");
            return -1;
        }

        // 持っていない駒を打つと反則
        if((turn==P1 && opp_piece_position[drop-1]!=-1)||(turn==P2 && opp_piece_position[drop-1]!=-2)){
            printf("violation: You can't drop a piece which you haven't captured.\n");
            return -1;
        }

        // 歩関連の反則
        if(input[2]=='F'&&input[3]=='U'){
            // 二歩
            int double_FU = 0;
            for(int i=0;i<4;i++){
                if(g_board.state[i][drop_col] == FU)
                    double_FU = 1;
            }
            if(double_FU == 1){
                printf("violation: Don't drop FU in the column with your unpromoted FU.\n");
                return -1;
            }
            
            // 敵陣に持ち駒の歩を打つと反則
            if((turn==P1 && drop_row>2) || (turn==P2 && drop_row<2)){
                printf("violation: Don't drop FU in opponent's field.\n");
                return -1;
            }

            // 【未実装】打ち歩詰め
            if (drop == FU){
                //FUを打ったときに、
                g_board.state[drop_row][drop_col] = FU;
                if (judge_tsumi(turn)) {
                    printf("violation: Don't drop FU to TSUMI.\n");
                    return -1;
                }
            }
        }

        //実際に打つ
        g_board.state[drop_row][drop_col] = drop * turn;
        opp_piece_position[drop-1] = drop_row*5 + drop_col;
    }
    
    //【未実装】王手放置
    if (wasChecked){
        printf("wasChecked");
        //王手だったとき、
        if (judge_check(-turn)) {
            //王手を放置したら
            printf("violation: Don't neglect checkmate.\n");
            return -1;
        }
    }
    //勝敗が決まれば1, そうでなければ0
    if (g_board.P1[OU-1] < 0 || g_board.P2[OU-1] < 0) return 1;
    return 0;
}

int get_input(int turn) {
    char input[100];
    scanf("%s", input);
    return move_piece(input, turn);
}

int compute_output(int turn) {
    //現状はユーザによる手入力で代替
    char input[100];
    scanf("%s", input);
    return move_piece(input, turn);
}

void youWin(){
    printf("You Win\n");
    exit(0);
}

void youLose(){
    printf("You Lose\n");
    exit(0);
}

void Even(){
    printf("Even\n");
    exit(0);
}



int main(int argc, char* argv[]) {
    init();
    int cnt = 0, res;

    if (argc >= 3 && !strcmp(argv[2], "1")) DEBUG = 1;

    if (!strcmp(argv[1], "0")) {
        // human plays first
        while (1) {
            print();
            // human's turn
            printf("P1's turn: ");
            res = get_input(P1);
            if(res == 1){
                print();
                youWin();
            }else if(res == -1){
                youLose();
            }
            cnt++;

            print();
            // computer's turn
            printf("P2's turn: ");
            res = compute_output(P2);
            if(res == 1){
                print();
                youLose();
            }else if(res == -1){
                youWin();
            }
            cnt++;

            if(cnt >= 150) Even();
        }
    } else {
        // computer plays first
        while (1) {
            print();
            // computer's turn
            printf("P2's turn: ");
            res = compute_output(P1);
            if(res == 1){
                print();
                youLose();
            }else if(res == -1){
                youWin();
            }
            cnt++;

            print();
            // human's turn
            printf("P1's turn: ");
            res = get_input(P2);
            if(res == 1){
                print();
                youWin();
            }else if(res == -1){
                youLose();
            }
            cnt++;

            if(cnt >= 150) Even();
        }
    }

    return 0;
}
