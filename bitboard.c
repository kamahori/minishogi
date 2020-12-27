#include <stdio.h>
#include <stdlib.h>

#include "global_const.h"

// square(0�`24) �� bitboard �ɕϊ�
BitBoard bitboard(int square)
{
    return 1 << square;
}

// �ȉ��̊֐��� square �̈ʒu�ɂ����̓�����ꏊ��\�� bitboard ��Ԃ��i���̋�Ƃ̏Փ˂͂����ł͖����j
BitBoard generate_FU_movable(int square)
{
    if (square >= 20) return 0;
    return bitboard(square + 5);
}

BitBoard generate_KI_movable(int square)
{
    BitBoard res = 0;
    int row = square / 5, col = square % 5;
    if (row >= 1) res |= bitboard(square - 5);
    if (row <= 3) res |= bitboard(square + 5);
    if (col >= 1) res |= bitboard(square - 1);
    if (col <= 3) res |= bitboard(square + 1);
    if (row <= 3 && col >= 1) res |= bitboard(square + 4);
    if (row <= 3 && col <= 3) res |= bitboard(square + 6);
    return res;
}

BitBoard generate_GI_movable(int square)
{
    BitBoard res = 0;
    int row = square / 5, col = square % 5;
    if (row <= 3) res |= bitboard(square + 5);
    if (row >= 1 && col >= 1) res |= bitboard(square - 6);
    if (row >= 1 && col <= 3) res |= bitboard(square - 4);
    if (row <= 3 && col >= 1) res |= bitboard(square + 4);
    if (row <= 3 && col <= 3) res |= bitboard(square + 6);
    return res;
}

BitBoard generate_OU_movable(int square)
{
    BitBoard res = 0;
    int row = square / 5, col = square % 5;
    if (row >= 1) res |= bitboard(square - 5);
    if (row <= 3) res |= bitboard(square + 5);
    if (col >= 1) res |= bitboard(square - 1);
    if (col <= 3) res |= bitboard(square + 1);
    if (row >= 1 && col >= 1) res |= bitboard(square - 6);
    if (row >= 1 && col <= 3) res |= bitboard(square - 4);
    if (row <= 3 && col >= 1) res |= bitboard(square + 4);
    if (row <= 3 && col <= 3) res |= bitboard(square + 6);
    return res;
}
int main()
{
    printf("const int FU_movable[25] = { \n\t");
    for (int square = 0; square < 25; square++) {
        printf("%d, ", generate_FU_movable(square));
    }
    printf("\n};\n");
    printf("const int KI_movable[25] = { \n\t");
    for (int square = 0; square < 25; square++) {
        printf("%d, ", generate_KI_movable(square));
    }
    printf("\n};\n");
    printf("const int GI_movable[25] = { \n\t");
    for (int square = 0; square < 25; square++) {
        printf("%d, ", generate_GI_movable(square));
    }
    printf("\n};\n");
    printf("const int OU_movable[25] = { \n\t");
    for (int square = 0; square < 25; square++) {
        printf("%d, ", generate_OU_movable(square));
    }
    printf("\n};\n");

    return 0;
}