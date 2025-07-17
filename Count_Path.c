#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH_LEN 1024
#define MAX_ENTRIES 100

char call_path[MAX_PATH_LEN] = "";

// SOA構造
char *path_array[MAX_ENTRIES]; // 経路文字列群
int count_array[MAX_ENTRIES];  // カウント群
int path_count_size = 0;       // 登録数

void append_to_path(const char *func_name)
{
    strcat(call_path, func_name);
    strcat(call_path, "→");
}

void count_path()
{
    for (int i = 0; i < path_count_size; i++)
    {
        if (strcmp(path_array[i], call_path) == 0)
        {
            count_array[i]++;
            return;
        }
    }
    // 新規経路
    path_array[path_count_size] = malloc(strlen(call_path) + 1);
    strcpy(path_array[path_count_size], call_path);
    count_array[path_count_size] = 1;
    path_count_size++;
}

void reset_path()
{
    call_path[0] = '\0';
}

// SOA対応 swap
void swap(int i, int j)
{
    char *temp_path = path_array[i];
    path_array[i] = path_array[j];
    path_array[j] = temp_path;

    int temp_count = count_array[i];
    count_array[i] = count_array[j];
    count_array[j] = temp_count;
}

// SOA対応 QuickSort
int partition(int low, int high)
{
    char *pivot = path_array[high];
    int i = low - 1;
    for (int j = low; j < high; j++)
    {
        if (strcmp(path_array[j], pivot) < 0)
        {
            i++;
            swap(i, j);
        }
    }
    swap(i + 1, high);
    return i + 1;
}

void quicksort(int low, int high)
{
    if (low < high)
    {
        int pi = partition(low, high);
        quicksort(low, pi - 1);
        quicksort(pi + 1, high);
    }
}

// 任意の関数群
void A();
void B();
void C();
void D();
void TestFunc();

void A()
{
    append_to_path("A");
    B();
}

void B()
{
    append_to_path("B");
    C();
}

void C()
{
    append_to_path("C");
    TestFunc();
}

void D()
{
    append_to_path("D");
    TestFunc();
}

void TestFunc()
{
    append_to_path("TestFunc");
    count_path();
    reset_path();
}

int main()
{
    // 呼び出し例
    A();
    A();
    B();
    C();
    D();
    append_to_path("A");
    count_path(); // A→
    append_to_path("A");
    append_to_path("B");
    append_to_path("C");
    append_to_path("D");
    count_path(); // A→B→C→D→

    // ソート
    quicksort(0, path_count_size - 1);

    // 結果表示
    printf("呼び出し経路と回数（SOA + 手動ソート）:\n");
    for (int i = 0; i < path_count_size; i++)
    {
        printf("%s : %d 回\n", path_array[i], count_array[i]);
    }

    // メモリ解放
    for (int i = 0; i < path_count_size; i++)
    {
        free(path_array[i]);
    }

    return 0;
}