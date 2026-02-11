#include <stdio.h>

#define BUF_NUM 4
#define TOTAL_DATA 10

typedef enum _e_buffer_state
{
    BUF_EMPTY = 0,
    BUF_FILLED = 1,
    BUF_SEQED = 2
} BufferState;

typedef struct
{
    char name;
    int read_count;
    int out_count;
    int buffer[BUF_NUM]; // 0=empty 1=filled 2=seq済
    int next_read;
    int next_out;
} Machine;

/* ==== API ==== */

void Read(char m, int b)
{
    printf("[%c] Read(%d)\n", m, b);
}
void MultiRead(char m)
{
    printf("[%c] MultiRead(ALL)\n", m);
}

void OutSeq(char m, int b)
{
    printf("[%c] OutSeq(%d)\n", m, b);
}

void OutExeStart(char m, int b)
{
    printf("[%c] OutExeStart(%d)\n", m, b);
}

void OutExe(char m, int b)
{
    printf("[%c] OutExe(%d)\n", m, b);
    printf("---- EXECUTION[%c:%d] ----\n", m, b);
}

void OutExeTerm(char m, int b)
{
    printf("[%c] OutExeTerm(%d)\n", m, b);
}

/* ==== 基本操作 ==== */

void init_machine(Machine *m, char name)
{
    int i;
    m->name = name;
    m->read_count = 0;
    m->out_count = 0;
    m->next_read = 0;
    m->next_out = 0;

    for (i = 0; i < BUF_NUM; i++)
        m->buffer[i] = BUF_EMPTY;
}

int can_read(Machine *m)
{
    return (m->read_count < TOTAL_DATA &&
            m->buffer[m->next_read] == BUF_EMPTY);
}
int can_multiread(Machine *m)
{
    int i;

    if (m->read_count >= TOTAL_DATA)
        return 0;

    for (i = 0; i < BUF_NUM; i++)
    {
        if (m->buffer[i] != BUF_EMPTY)
            return 0;
    }

    return 1;
}

int is_all_empty(Machine *m)
{
    int i;
    for (i = 0; i < BUF_NUM; i++)
    {
        if (m->buffer[i] != BUF_EMPTY)
            return 0;
    }
    return 1;
}

int can_out(Machine *m)
{
    return (m->buffer[m->next_out] == BUF_FILLED);
}

void do_read(Machine *m)
{
    int b = m->next_read;
    Read(m->name, b);
    m->buffer[b] = BUF_FILLED;
    m->read_count++;
    m->next_read = (m->next_read + 1) % BUF_NUM;
}

void do_multiread(Machine *m)
{
    int i;

    MultiRead(m->name);

    for (i = 0; i < BUF_NUM; i++)
        m->buffer[i] = BUF_FILLED;

    m->read_count += BUF_NUM;

    m->next_read = 0;
}


void do_outseq(Machine *m)
{
    int b = m->next_out;
    OutSeq(m->name, b);
    m->buffer[b] = 2;
}

/* ==== 1回分のExe ==== */
void execute_one(Machine *cur, Machine *other)
{
    int b = cur->next_out;

    if (cur->buffer[b] == BUF_FILLED)
        do_outseq(cur);

    OutExeStart(cur->name, b);

    /* ==== Exe中 ==== */

    /* ===== 他マシン Read処理 ===== */

    /* BだけMultiRead可能 */
    if (other->name == 'B')
    {
        if (can_multiread(other))
        {
            do_multiread(other);
        }
        else
        {
            //なにもしない
        }
    }
    else if (can_read(other))
    {
        do_read(other);
    }

    /* OutSeq */
    if (other->buffer[other->next_out] == BUF_FILLED)
        do_outseq(other);

    /* ==== Exe ==== */

    OutExe(cur->name, b);
    OutExeTerm(cur->name, b);

    cur->buffer[b] = BUF_EMPTY;
    cur->out_count++;
    cur->next_out = (cur->next_out + 1) % BUF_NUM;
}



/* ==== main ==== */
void main(void)
{
    Machine A, B;
    Machine *current;
    Machine *other;

    init_machine(&A, 'A');
    init_machine(&B, 'B');

    printf("=== INITIAL FILL ===\n");

    /* 初期4充填 */
    while (A.read_count < 4)
        do_read(&A);

    while (B.read_count < 4)
        do_multiread(&B);

    printf("\n=== PIPELINE START ===\n\n");

    current = &A;

    int step = 0;
    // while (A.out_count < TOTAL_DATA || B.out_count < TOTAL_DATA)
    while (1)
    {
        printf("\n---- STEP %d ----\n", ++step);

        other = (current == &A) ? &B : &A;

        /* ==== 実行可能ならExe ==== */
        if (current->buffer[current->next_out] != BUF_EMPTY)
        {
            execute_one(current, other);
        }

        /* ==== 終了判定 ==== */
        if (A.read_count >= TOTAL_DATA && B.read_count >= TOTAL_DATA && is_all_empty(&A) && is_all_empty(&B))
        {
            break;
        }

        /* ==== machine切り替え処理 */
        if (!is_all_empty(&A) && !is_all_empty(&B))
        {
            current = other;  /* 交互 */
        }
        /* Aだけ残っている */
        else if (!is_all_empty(&A))
        {
            current = &A;
        }
        /* Bだけ残っている */
        else if (!is_all_empty(&B))
        {
            current = &B;
        }
    }
    
    printf("\n=== FINISHED ===\n");
    
    
    printf("checker: A read %d times, out %d times\n", A.read_count, A.out_count);
    printf("checker: B read %d times, out %d times\n", B.read_count, B.out_count);

}
