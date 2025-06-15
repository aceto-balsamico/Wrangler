#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

void OperationFunction(); // 前方宣言

#define MAX_FUNCS 100
#define MAX_RECURSION 2

#define DEBUG_MODE

#ifdef DEBUG_MODE
#define TRACK_FUNC_CALL(label) trackFunctionCall(__func__, label)
#else
#define TRACK_FUNC_CALL(label)
#endif

typedef struct
{
    char *name;
    char *label;
    int index;
} FuncInfo;

FuncInfo funcRegistry[MAX_FUNCS];
int funcCount = 0;
int callMatrix[MAX_FUNCS][MAX_FUNCS] = {0};

int RecursionLevel = 0;
const char *lastFuncLabel = NULL; // **コール元のラベル**
const char *lastFuncName = NULL;  // **コール元の関数名**

int getFuncIndex(const char *name, const char *label, int isCaller)
{
    char identifier[256] = {};

    if (isCaller && label)
    {
        snprintf(identifier, sizeof(identifier), "%s_%s", name, label); // **呼び出し元（ラベル付き）**
    }
    else
    {
        snprintf(identifier, sizeof(identifier), "%s", name); // **呼び出し先（関数名のみ）**
    }

    for (int i = 0; i < funcCount; i++)
    {
        if (strcmp(funcRegistry[i].name, identifier) == 0)
            return funcRegistry[i].index;
    }

    funcRegistry[funcCount].name = strdup(identifier);
    funcRegistry[funcCount].index = funcCount;
    return funcCount++;
}

// **コール先の関数でカウントを行う**
void trackFunctionCall(const char *callerFunc, const char *callerLabel, const char *targetFunc)
{
    if (callerFunc && callerLabel)
    {
        int prevIndex = getFuncIndex(callerFunc, callerLabel, 1); // **呼び出し元（ラベル付き）**
        int currIndex = getFuncIndex(targetFunc, NULL, 0);        // **呼び出し先（関数名のみ）**

        if (prevIndex >= 0 && currIndex >= 0 && prevIndex != currIndex)
        {
            callMatrix[prevIndex][currIndex]++;
        }
    }
}
// **コール元の情報を記憶**
void setLastFunction(const char *currentFunc, const char *label)
{
    lastFuncLabel = label;      // **呼び出し元のラベル情報を維持**
    lastFuncName = currentFunc; // **呼び出し元の関数名を維持**
}
void FuncA()
{
    setLastFunction(__func__, "Entry");
    OperationFunction(); // **FuncA_Entry → 次の関数**
    setLastFunction(__func__, "Mid");
    OperationFunction(); // **FuncA_Mid → 次の関数**
    setLastFunction(__func__, "Exit");
    OperationFunction(); // **FuncA_Exit → 次の関数**
}

void FuncB()
{
    setLastFunction(__func__, "Entry");
    OperationFunction();
    setLastFunction(__func__, "Mid");
    OperationFunction();
    setLastFunction(__func__, "Exit");
    OperationFunction();
}

void FuncC()
{
    setLastFunction(__func__, "Entry");
    OperationFunction();
    setLastFunction(__func__, "Mid");
    OperationFunction();
    setLastFunction(__func__, "Exit");
    OperationFunction();
}
void OperationFunction()
{
    if (RecursionLevel >= MAX_RECURSION)
        return;

    RecursionLevel++;

    int randomFunc = rand() % 3;
    switch (randomFunc)
    {
    case 0:
        trackFunctionCall(lastFuncName, lastFuncLabel, "FuncA");
        FuncA();
        break;
    case 1:
        trackFunctionCall(lastFuncName, lastFuncLabel, "FuncB");
        FuncB();
        break;
    case 2:
        trackFunctionCall(lastFuncName, lastFuncLabel, "FuncC");
        FuncC();
        break;
    }

    RecursionLevel--;
}

void printMatrix()
{
    printf("\nCall Transition Counts:\n");

    printf("             ");
    for (int i = 0; i < funcCount; i++)
    {
        if (!strchr(funcRegistry[i].name, '_'))
        { // **横軸は関数名のみ**
            printf("%12s ", funcRegistry[i].name);
        }
    }
    printf("\n");

    for (int i = 0; i < funcCount; i++)
    {
        if (strchr(funcRegistry[i].name, '_'))
        { // **縦軸はラベル付き**
            printf("%12s ", funcRegistry[i].name);
            for (int j = 0; j < funcCount; j++)
            {
                if (!strchr(funcRegistry[j].name, '_'))
                { // **横軸は関数名のみ**
                    printf("%12d ", callMatrix[i][j]);
                }
            }
            printf("\n");
        }
    }
}

double get_time()
{
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)freq.QuadPart;
}

int main()
{
    srand((unsigned int)time(NULL));

    int LoopCount = 1000;
    double start = get_time();
    for (int TestLoop = 0; TestLoop < LoopCount; TestLoop++)
    {
        RecursionLevel = 0;
        lastFuncLabel = NULL;
        lastFuncName = NULL;
        OperationFunction();
    }
    double end = get_time();
    printf("Total time for %d operations: %.6f s, %.6f ms\n", LoopCount, end - start, (end - start) * 1000);

    printMatrix();
    return 0;
}