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
    
    int P1OU;
    int P1KI;
    int P1GI;
    int P1KK;
    int P1HI;
    int P1FU;
    
    int P2OU;
    int P2KI;
    int P2GI;
    int P2KK;
    int P2HI;
    int P2FU;
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

    g_board.P1OU = 0;
    g_board.P1KI = 1;
    g_board.P1GI = 2;
    g_board.P1KK = 3;
    g_board.P1HI = 4;
    g_board.P1FU = 5;
    
    g_board.P2OU = 24;
    g_board.P2KI = 23;
    g_board.P2GI = 22;
    g_board.P2KK = 21;
    g_board.P2HI = 20;
    g_board.P2FU = 19;
}

void print() {
    if (DEBUG == 0) return;

    printf("   A B C D E\n");
    printf("  ----------\n");
    for (int i = 0; i < 5; i++) {
        printf("%d| ", 5 - i);
        for (int j = 0; j < 5; j++) {
            printf("%d ", g_board.state[4 - i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int is_finished(board b) {
    if (b.P1OU < 0 || b.P2OU < 0) return 1;
    return 0;
}

int validate_move(char input[], int turn){
    
}

int move_piece(char input[], int turn) {
    int mode = 0, //0:動かす, 1:駒を打つ
    piece_put = 0; // 打つ駒(あれば)

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

    // 移動元を指定できていなければ反則
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
            piece_put = HI;
        }else if(input[2]=='K'&&input[3]=='K'){
            piece_put = KK;
        }else if(input[2]=='K'&&input[3]=='I'){
            piece_put = KI;
        }else if(input[2]=='G'&&input[3]=='I'){
            piece_put = GI;
        }else if(input[2]=='F'&&input[3]=='U'){
            piece_put = FU;
        }else{
            printf("error: wrong input\n");
            return -1;
        }

        //【未実装】打つ際の反則
        //持ち駒にない駒を打とうとする, 打ち歩詰め, 敵陣に持ち駒の歩を打つ
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

        
        //駒の種類からしてアウトな動かし方なら反則、そうでなければ動かす
        int piece_type = abs(piece);
        int diff_row = next_row - prev_row, diff_col = next_col - prev_col;
        int diff_max = abs(diff_col)>abs(diff_row)?abs(diff_col):abs(diff_row);

        // 【未実装】以下の「実際に駒を動かす」部分
        if(piece_type == OU){
            //全方向1マスずつ
            if(diff_max != 1){
                printf("violation: You can't move OU in this way.\n");
                return -1;
            }
        }else if(piece_type == KI || piece_type == GI*10 || piece_type == FU*10){
            //斜後ろを除き1マスずつ
            if(diff_max != 1 || ( (diff_row * turn < 0) && (diff_col != 1) )){
                printf("violation: You can't move KI(or promoted GI/FU) in this way.\n");
                return -1;
            }
        }else if(piece_type == GI){
            //横と真後を除き1マスずつ
            if(diff_max != 1 || diff_row == 0 || diff_row * turn < 0){
                printf("violation: You can't move GI in this way.\n");
                return -1;
            }
        }else if(piece_type == KK){
            //斜めだけ
            if(abs(diff_row) != abs(diff_col)){
                printf("violation: You can't move KK in this way.\n");
                return -1;
            }
        }else if(piece_type == KK*10){
            //斜め + 周囲1マスずつ
            if(abs(diff_row) != abs(diff_col) && diff_max != 1){
                printf("violation: You can't move promoted KK in this way.\n");
                return -1;
            }
        }else if(piece_type == HI){
            //縦横だけ
            if(diff_row * diff_col != 0){
                printf("violation: You can't move HI in this way.\n");
                return -1;
            }
        }else if(piece_type == HI*10){
            //縦横 + 周囲1マスずつ
            if(diff_row * diff_col != 0 && diff_max != 1){
                printf("violation: You can't move promoted HI in this way.\n");
                return -1;
            }
        }else if(piece_type == FU){
            //1つ前のみ
            if(diff_row * turn != 1){
                printf("violation: You can't move FU in this way.\n");
                return -1;
            }
            //歩が成れる状況で成らなかった場合、反則
            if(is_promotable == 1 && input_length == 4){
                printf("violation: You must promote FU when possible.\n");
                return -1;
            }
        }

        // 【未実装】駒を取る


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
            //g_board.state[next_row][next_col] *= 10;
        }

    }else{ // 打つ場合
        int drop_row = input[0] - '1';
        int drop_col = input[1] - 'A';

        // 打つ先に駒があるなら反則
        if (g_board.state[drop_row][drop_col] != EMPTY){
            printf("violation: Don't drop a piece where there is one.\n");
            return -1;
        }

        // 【未実装】持っていない駒を打つと反則

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
            
            // 敵陣に持ち駒の歩を打つ
            if((turn==P1 && drop_row>2) || (turn==P2 && drop_row<2)){
                printf("violation: Don't drop FU in opponent's field.\n");
                return -1;
            }

            // 【未実装】打ち歩詰め
        }
    }
    
    //【未実装】王手放置

    // returnの値は現状は1で固定
    return 1;
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
            res = get_input(P1);
            if(res==-1) youLose();
            cnt++;
            print();
            if(is_finished(g_board)) youWin();

            // computer's turn
            res = compute_output(P2);
            if(res==-1) youWin();
            cnt++;
            if(is_finished(g_board)) youLose();

            if(cnt >= 150) Even();
        }
    } else {
        // computer plays first
        while (1) {
            print();
            // computer's turn
            res = compute_output(P1);
            if(res==-1) youWin();
            cnt++;
            print();
            if(is_finished(g_board)) youLose();

            // human's turn
            res = get_input(P2);
            if(res==-1) youLose();
            cnt++;
            if(is_finished(g_board)) youWin();

            if(cnt >= 150) Even();
        }
    }

    return 0;
}