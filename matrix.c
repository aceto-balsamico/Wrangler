#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define REALLOC_RATIO 2
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define INITIAL_VALUE -1
#define DEAD_VALUE 999

typedef struct
{
    int rows;
    int *used_col_count; // 実際のデータ数
    int *max_col_count;  // 確保済み容量
    int **data_array;    // 行の配列
} Table2D;

Table2D *MakeTable2D(int rows)
{
    Table2D *table = malloc(sizeof(Table2D));
    table->rows = rows;
    table->used_col_count = calloc(rows, sizeof(int));
    table->max_col_count = calloc(rows, sizeof(int));
    table->data_array = calloc(rows, sizeof(int *));

    for(int row_index = 0; row_index < rows; row_index++)
    {
        table->max_col_count[row_index] = rows; // 初期容量は行数と同じにする
        table->data_array[row_index] = malloc(sizeof(int) * table->max_col_count[row_index]);
        for(int col_index = 0; col_index < table->max_col_count[row_index]; col_index++)
        {
            table->data_array[row_index][col_index] = INITIAL_VALUE;
        }
        table->used_col_count[row_index] = 0;
    }
    return table;
}

void AppendRow(Table2D *table, int target_row, int value)
{
    if(target_row >= table->rows)
    {
        return;
    }

    int used = table->used_col_count[target_row];
    int max = table->max_col_count[target_row];

    if(used >= max)
    {
        int new_max = max * REALLOC_RATIO;
        int *new_data = realloc(table->data_array[target_row], sizeof(int) * new_max);

        for(int col_index = max; col_index < new_max; col_index++)
        {
            new_data[col_index] = INITIAL_VALUE;
        }
        table->data_array[target_row] = new_data;
        table->max_col_count[target_row] = new_max;
        printf("Realloc: row=%d, max=%d->%d\n", target_row, max, new_max);
    }

    table->data_array[target_row][used] = value;
    table->used_col_count[target_row] = used + 1;
}
void AppendColumn(Table2D *table, int *values, int value_count)
{
    for(int row_index = 0; row_index < table->rows; row_index++)
    {
        int value = DEAD_VALUE;

        if(row_index < value_count)
        {
            value = values[row_index]; // 値がある
        }

        AppendRow(table, row_index, value);
    }
}

void AddRow(Table2D *table)
{
    table->rows += 1;
    table->used_col_count = realloc(table->used_col_count, sizeof(int) * table->rows);
    table->max_col_count = realloc(table->max_col_count, sizeof(int) * table->rows);
    table->data_array = realloc(table->data_array, sizeof(int *) * table->rows);

    table->used_col_count[table->rows - 1] = 0;
    table->max_col_count[table->rows - 1] = table->rows;
    table->data_array[table->rows - 1] = malloc(sizeof(int) * table->max_col_count[table->rows - 1]);
    for(int col_index = 0; col_index < table->max_col_count[table->rows - 1]; col_index++)
    {
        table->data_array[table->rows - 1][col_index] = INITIAL_VALUE;
    }
}


void FreeTable(Table2D *table)
{
    for(int row_index = 0; row_index < table->rows; row_index++)
    {
        free(table->data_array[row_index]);
    }
    free(table->data_array);
    free(table->used_col_count);
    free(table);
}

int *GetRowPtr(Table2D *table, int target_row)
{
    if(target_row >= table->rows)
        return NULL;
    return table->data_array[target_row];
}

int GetRowCount(Table2D *table, int target_row)
{
    if(target_row >= table->rows)
        return 0;
    return table->used_col_count[target_row];
}

int GetMaxRowValue(Table2D *table, int target_row)
{
    if(target_row >= table->rows)
        return 0;
    int count = table->used_col_count[target_row];
    if(count <= 0)
        return 0;

    int target_value = table->data_array[target_row][0];
    for(int col_index = 1; col_index < count; col_index++)
    {
        int check_value = table->data_array[target_row][col_index];
        if(check_value > target_value)
            target_value = check_value;
    }
    return target_value;
}
int GetMinRowValue(Table2D *table, int target_row)
{
    if(target_row >= table->rows)
        return 0;
    int count = table->used_col_count[target_row];
    if(count <= 0)
        return 0;

    int target_value = table->data_array[target_row][0];
    for(int col_index = 1; col_index < count; col_index++)
    {
        int check_value = table->data_array[target_row][col_index];
        if(check_value < target_value)
            target_value = check_value;
    }
    return target_value;
}
int SearchRowValueIndex(Table2D *table, int target_row, int value)
{
    if(target_row >= table->rows)
        return -1;
    int count = table->used_col_count[target_row];
    for(int col_index = 0; col_index < count; col_index++)
    {
        if(table->data_array[target_row][col_index] == value)
            return col_index;
    }
    return -1;
}
int GetTotalTableElements(Table2D *table)
{
    int total = 0;
    for(int row_index = 0; row_index < table->rows; row_index++)
    {
        total += table->used_col_count[row_index];
    }
    return total;
}
void Shuffle(int *arr, int count)
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

void PrintTable(Table2D *table)
{
    for(int row_index = 0; row_index < table->rows; row_index++)
    {
        printf("[%2d]", row_index);
        printf(" N=%2d/%2d", table->used_col_count[row_index], table->max_col_count[row_index]);
        printf(" | ");

        for(int col_index = 0; col_index < table->used_col_count[row_index]; col_index++)
        {
            if(table->data_array[row_index][col_index] != INITIAL_VALUE)
            {
                printf("%4d,", table->data_array[row_index][col_index]);
            }
        }
        printf("\n");
    }
    printf("\n");
}
void QuickSort(int *arr, int left, int right, int flag_ascending)
{
    if(left >= right)
        return;

    int pivot = arr[right];
    int i = left - 1;

    for(int j = left; j < right; j++)
    {
        if((flag_ascending && arr[j] <= pivot) || (!flag_ascending && arr[j] >= pivot))
        {
            i++;
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }

    int temp = arr[i + 1];
    arr[i + 1] = arr[right];
    arr[right] = temp;

    QuickSort(arr, left, i, flag_ascending);
    QuickSort(arr, i + 2, right, flag_ascending);
}

void SortRow(Table2D *table, int target_row, int flag_ascending)
{
    if(target_row >= table->rows)
        return;

    int count = table->used_col_count[target_row];
    if(count <= 1)
        return;

    QuickSort(table->data_array[target_row], 0, count - 1, flag_ascending);
}
int RowNext(Table2D *table, int row, int *out)
{
    // static を関数内に閉じる
    static int *ptr = NULL;
    static int index = 0;
    static int count = 0;
    static int last_row = (int)-1;

    // 初回呼び出し or 行が変わったときに初期化
    if (row != last_row || ptr == NULL)
    {
        ptr = table->data_array[row];
        count = table->used_col_count[row];
        index = 0;
        last_row = row;
    }

    // 終了判定
    if (index >= count)
    {
        ptr = NULL; // 次回呼び出しで再初期化される
        return 0;
    }

    // 値を返す
    *out = ptr[index++];
    return 1;
}

int main()
{
    Table2D *table = MakeTable2D(5);
    int array_A_1[] = {1, 2, 3, 4, 5};
    int array_A_2[] = {6, 7, 8, 9, 10};
    AppendColumn(table, array_A_1, ARRAY_SIZE(array_A_1));
    AppendColumn(table, array_A_2, ARRAY_SIZE(array_A_2));
    AppendRow(table, 0, 0);
    AppendRow(table, 0, 0);
    AppendRow(table, 0, 0);
    AppendRow(table, 0, 0);
    AppendRow(table, 1, 1);
    AppendRow(table, 1, 1);
    AppendRow(table, 2, 55);
    AppendRow(table, 2, 66);
    AppendRow(table, 2, 77);
    AppendRow(table, 2, 88);
    AppendRow(table, 2, 99);
    AppendRow(table, 4, 1);

    PrintTable(table);
    AddRow(table);
    printf("\n");
    AppendRow(table, 5, 22);
    AppendRow(table, 5, 22);
    AppendRow(table, 5, 22);
    AppendRow(table, 5, 22);
    PrintTable(table);

    printf("\n");
    Shuffle(GetRowPtr(table, 2), GetRowCount(table, 2));
    PrintTable(table);
    
    int array_B[] = {10, 20, 30, 40, 50};
    AppendColumn(table, array_B, ARRAY_SIZE(array_B));
    PrintTable(table);
    
    SortRow(table, 2, 1);
    PrintTable(table);
    
    printf("Row:%d,Max:%d\n", 2, GetMaxRowValue(table, 2));
    printf("Row:%d,Min:%d\n", 2, GetMinRowValue(table, 2));
    printf("Row:%d,Search(55):%d\n", 2, SearchRowValueIndex(table, 2, 55));
    printf("Total Elements: %d\n", GetTotalTableElements(table));

    int row_value = 0;
    printf("\nRow 2 Next Values->");
    while(RowNext(table, 2, &row_value))
    {
        printf("%d,", row_value);
    }
    printf("\nRow 3 Next Values->");
    while(RowNext(table, 3, &row_value))
    {
        printf("%d,", row_value);
    }
    printf("\n");
    FreeTable(table);
}