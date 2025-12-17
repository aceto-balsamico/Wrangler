#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// 縦型ヒストグラムを表示する関数
void print_vertical_histogram(const int arr[], int size)
{
    // 最大値を求める
    int max = 0;
    for (int i = 0; i < size; i++)
    {
        if (arr[i] > max)
            max = arr[i];
    }

    // 上から下へ行ごとに描画
    for (int level = max; level > 0; level--)
    {
        for (int i = 0; i < size; i++)
        {
            if (arr[i] >= level)
            {
                printf(" * ");
            }
            else
            {
                printf("   ");
            }
        }
        printf("\n");
    }

    // 下に区切り線
    for (int i = 0; i < size; i++)
    {
        printf("---");
    }
    printf("\n");

    // インデックスを表示（列ラベル）
    for (int i = 0; i < size; i++)
    {
        printf("%2d ", i);
    }
    printf("\n");
}
// 横向きヒストグラムを表示する関数
void print_horizontal_histogram(const int values[], const char *labels[], int size)
{
    // 最大値を求める
    int max = 0;
    for (int i = 0; i < size; i++)
    {
        if (values[i] > max)
            max = values[i];
    }

    // ターミナルの横幅を取得
    int term_width = 80; // デフォルト

    // ラベル部分の幅を決める（最大ラベル長＋余白）
    int label_width = 0;
    for (int i = 0; i < size; i++)
    {
        int len = strlen(labels[i]);
        if (len > label_width)
            label_width = len;
    }
    label_width += 2; // 余白

    // 棒の最大長さをターミナル幅に合わせる
    int bar_max = term_width - label_width - 5; // 余裕を持たせる
    if (bar_max < 10)
        bar_max = 10; // 最低限

    // スケーリング係数
    double scale = (max > 0) ? (double)bar_max / max : 1.0;

    // 出力
    for (int i = 0; i < size; i++)
    {
        int bar_len = (int)(values[i] * scale);
        printf("%-*s | ", label_width, labels[i]);
        for (int j = 0; j < bar_len; j++)
        {
            putchar('*');
        }
        printf(" (%d)\n", values[i]);
    }
}

int main(void)
{
    int data[] = {3, 7, 1, 5, 9, 4};
    const char *labels[] = {"A", "B", "C", "D", "E", "F"};
    int size = sizeof(data) / sizeof(data[0]);

    print_horizontal_histogram(data, labels, size);
    print_vertical_histogram(data, size);

    return 0;
}