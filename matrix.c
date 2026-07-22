#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define REALLOC_RATIO 2

typedef struct 
{
    int rows;
    int* used_col_count;   // 実際のデータ数
    int* max_col_count;    // 確保済み容量
    int** data;         // 行の配列
}Table2D;


Table2D* MakeTable(int rows)
{
    Table2D* table = malloc(sizeof(Table2D));
    table->rows = rows;
    table->used_col_count = calloc(rows, sizeof(int));
    table->max_col_count = calloc(rows, sizeof(int));
    table->data = calloc(rows, sizeof(int* ));

    for(int row_index = 0; row_index < rows; row_index++)
    {
        table->max_col_count[row_index] = rows;     // 初期容量は行数と同じにする
        table->data[row_index] = malloc(sizeof(int) * table->max_col_count[row_index]);
        for(int col_index = 0; col_index < table->max_col_count[row_index]; col_index++)
        {
            table->data[row_index][col_index] = -1;
        }
        table->used_col_count[row_index] = 0;
    }
    return table;
}

void AppendRow(Table2D* table, int target_row, int value)
{
    if(target_row >= table->rows)   {return;}

    int used = table->used_col_count[target_row];
    int max  = table->max_col_count[target_row];

    if(used >= max)
    {
        int new_max = max * REALLOC_RATIO;
        int* new_data = realloc(table->data[target_row], sizeof(int) * new_max);

        for(int col_index = max; col_index < new_max; col_index++)
        {
            new_data[col_index] = -1;
        }
        table->data[target_row] = new_data;
        table->max_col_count[target_row] = new_max;
        printf("Realloc: row=%d, max=%d->%d\n", target_row, max, new_max);
    }

    table->data[target_row][used] = value;
    table->used_col_count[target_row] = used + 1;
}

void AddRow(Table2D* table)
{
    table->rows += 1;
    table->used_col_count = realloc(table->used_col_count, sizeof(int) * table->rows);
    table->max_col_count = realloc(table->max_col_count, sizeof(int) * table->rows);
    table->data = realloc(table->data, sizeof(int* ) * table->rows);

    table->used_col_count[table->rows - 1] = 0;
    table->max_col_count[table->rows - 1] = table->rows;
    table->data[table->rows - 1] = malloc(sizeof(int) * table->max_col_count[table->rows - 1]);
    for(int col_index = 0; col_index < table->max_col_count[table->rows - 1]; col_index++)
    {
        table->data[table->rows - 1][col_index] = -1;
    }
}

void PrintTable(Table2D* table)
{
    int Total = 0;
    for(int row_index = 0; row_index < table->rows; row_index++)
    {
        printf("[%2d]", row_index+1);
        printf(" N=%2d/%2d", table->used_col_count[row_index], table->max_col_count[row_index]);
        printf(" | ");

        for(int col_index = 0; col_index < table->used_col_count[row_index]; col_index++)
        {
            if(table->data[row_index][col_index] != -1)
            {
                printf("%d ", table->data[row_index][col_index]);
                Total++;
            }
        }
        printf("\n");
    }
    // for(int row_index = 0; row_index < table->rows; row_index++)
    // {
    //     printf("[%2d]", row_index+1);
    //     printf(" N=%2d/%2d", table->used_col_count[row_index], table->max_col_count[row_index]);
    //     printf(" | ");

    //     for(int col_index = 0; col_index < table->used_col_count[row_index]; col_index++)
    //     {
    //         if(table->data[row_index][col_index] != -1)
    //         {
    //             printf("%X ", &table->data[row_index][col_index]);
    //             Total++;
    //         }
    //     }
    //     printf("\n");
    // }
    printf("Total:%d\n", Total);
}

void FreeTable(Table2D* table)
{
    for(int row_index = 0; row_index < table->rows; row_index++)
    {
        free(table->data[row_index]);
    }
    free(table->data);
    free(table->used_col_count);
    free(table);
}

int* GetRowPtr(Table2D* table, int target_row)
{
    if(target_row >= table->rows)
        return NULL;
    return table->data[target_row];
}

int GetRowCount(Table2D* table, int target_row)
{
    if(target_row >= table->rows)
        return 0;
    return table->used_col_count[target_row];
}

int GetMaxRowValue(Table2D* table, int target_row)
{
    if(target_row >= table->rows)
        return 0;
    int count = table->used_col_count[target_row];
    if(count <= 0)
        return 0;

    int target_value = table->data[target_row][0];
    for(int col_index = 1; col_index < count; col_index++)
    {
        int check_value = table->data[target_row][col_index];
        if(check_value > target_value)
            target_value = check_value;
    }
    return target_value;
}

void Shuffle(int* arr, int count)
{
    if(count <= 1)
        return;

    for(int i = count - 1; i > 0; i--)
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
    AppendRow(table, 4, 1);

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