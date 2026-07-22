#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define REALLOC_RATIO 2

typedef struct {
    int rows;
    int *used_count;   // 実際のデータ数
    int *max_count;    // 確保済み容量
    int   **data;         // 行の配列
} Table2D;


/* 作成 */
Table2D* MakeTable(int rows)
{
    Table2D* table = malloc(sizeof(Table2D));
    table->rows = rows;
    table->used_count = calloc(rows, sizeof(int));
    table->max_count = calloc(rows, sizeof(int));
    table->data = calloc(rows, sizeof(int* ));

    
    /* データなしの行は -1 を入れておく */
    for (int row_index = 0; row_index < rows; row_index++)
    {
        table->max_count[row_index] = rows;
        table->data[row_index] = malloc(sizeof(int) * table->max_count[row_index]);
        for(int col_index = 0; col_index < table->max_count[row_index]; col_index++)
        {
            table->data[row_index][col_index] = -1;
        }
        // table->data[row_index][0] = -1;
        table->used_count[row_index] = 0;
        table->max_count[row_index] = rows;
    }
    return table;
}

/* 行に値を追加（append） */
void AppendRow(Table2D* table, int target_row, int value)
{
    if (target_row >= table->rows)
        return;

    // int count = table->used_count[target_row];

    // /* データなし(-1)だけの行なら置き換える */
    // if (count == 0 && table->data[target_row][0] == -1)
    // {
    //     table->data[target_row][0] = value;
    //     table->used_count[target_row] = 1;
    //     return;
    // }

    // /* 新しいサイズの領域を確保 */
    // int* new_data = realloc(table->data[target_row], sizeof(int) * (table->max_count[target_row]) * REALLOC_RATIO);

    // new_data[count] = value;

    // table->data[target_row] = new_data;
    // table->used_count[target_row] = count + 1;
    // table->max_count[target_row] = table->max_count[target_row] * REALLOC_RATIO;
    int used = table->used_count[target_row];
    int max  = table->max_count[target_row];

    if (used >= max)
    {
        int new_max = max * REALLOC_RATIO;
        int* new_data = realloc(table->data[target_row], sizeof(int) * new_max);

        for(int i = max; i < new_max; i++)
        {
            new_data[i] = -1;
        }
        table->data[target_row] = new_data;
        table->max_count[target_row] = new_max;
    }

    table->data[target_row][used] = value;
    table->used_count[target_row] = used + 1;

    printf("AppendRow: row=%d, value=%d\n", target_row, value);
}

/* 行を増やす */
void AddRow(Table2D* table)
{
    table->rows += 1;
    table->used_count = realloc(table->used_count, sizeof(int) * table->rows);
    table->max_count = realloc(table->max_count, sizeof(int) * table->rows);
    table->data = realloc(table->data, sizeof(int* ) * table->rows);

    /* 新しい行は -1 のみ */
    table->used_count[table->rows - 1] = 0;
    table->max_count[table->rows - 1] = table->rows;
    table->data[table->rows - 1] = malloc(sizeof(int) * table->max_count[table->rows - 1]);
    for (int i = 0; i < table->max_count[table->rows - 1]; i++)
    {
        table->data[table->rows - 1][i] = -1;
    }
}

/* ASCII アート表示 */
void PrintTable(Table2D* table)
{
    int Total = 0;
    for (int row_index = 0; row_index < table->rows; row_index++)
    {
        printf("[%2d]", row_index+1);
        printf(" N=%2d/%2d", table->used_count[row_index], table->max_count[row_index]);
        printf(" | ");

        for (int col_index = 0; col_index < table->used_count[row_index]; col_index++)
        {
            if (table->data[row_index][col_index] != -1)
            {
                printf("%d ", table->data[row_index][col_index]);
                Total++;
            }
        }
        printf("\n");
    }
    for (int row_index = 0; row_index < table->rows; row_index++)
    {
        printf("[%2d]", row_index+1);
        printf(" N=%2d/%2d", table->used_count[row_index], table->max_count[row_index]);
        printf(" | ");

        for (int col_index = 0; col_index < table->used_count[row_index]; col_index++)
        {
            if (table->data[row_index][col_index] != -1)
            {
                printf("%X ", &table->data[row_index][col_index]);
                Total++;
            }
        }
        printf("\n");
    }
    printf("Total:%d\n", Total);
}

/* 解放 */
void FreeTable(Table2D* table)
{
    for (int row_index = 0; row_index < table->rows; row_index++)
    {
        free(table->data[row_index]);
    }
    free(table->data);
    free(table->used_count);
    free(table);
}

int* GetRowPtr(Table2D* table, int target_row)
{
    if (target_row >= table->rows)
        return NULL;
    return table->data[target_row];
}

int GetRowCount(Table2D* table, int target_row)
{
    if (target_row >= table->rows)
        return 0;
    return table->used_count[target_row];
}

int GetMaxRowValue(Table2D* table, int target_row)
{
    if (target_row >= table->rows)
        return 0;
    int count = table->used_count[target_row];
    if (count <= 0)
        return 0;

    int target_value = table->data[target_row][0];
    for (int col_index = 1; col_index < count; col_index++)
    {
        int check_value = table->data[target_row][col_index];
        if (check_value > target_value)
            target_value = check_value;
    }
    return target_value;
}

void Shuffle(int* arr, int count)
{
    if (count <= 1)
        return;

    for (int i = count - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

int main()
{
    Table2D* table = MakeTable(5);
    AppendRow(table, 0, 0);
    AppendRow(table, 0, 0);
    AppendRow(table, 0, 0);
    AppendRow(table, 0, 0);
    AppendRow(table, 0, 0);
    AppendRow(table, 1, 1);
    AppendRow(table, 1, 1);
    AppendRow(table, 1, 1);
    AppendRow(table, 1, 1);
    AppendRow(table, 1, 1);
    AppendRow(table, 0, 2);
    AppendRow(table, 2, 5);
    AppendRow(table, 2, 6);
    AppendRow(table, 2, 7);
    AppendRow(table, 2, 8);
    AppendRow(table, 2, 9);
    // AppendRow(table, 0, 1);
    // AppendRow(table, 0, 1);
    AppendRow(table, 4, 1);
    // AppendRow(table, 0, 1);

    PrintTable(table);
    AddRow(table);
    printf("\n");
    AppendRow(table, 5, 2);
    AppendRow(table, 5, 2);
    AppendRow(table, 5, 2);
    AppendRow(table, 5, 2);
    PrintTable(table);

    printf("\n");
    Shuffle(GetRowPtr(table, 2), GetRowCount(table, 2));
    PrintTable(table);
    printf("Row:%d,Max:%d\n", 2, GetMaxRowValue(table, 2));


    FreeTable(table);
}