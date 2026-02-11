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

    /* OutSeq（まだなら） */
    if (cur->buffer[b] == 1)
        do_outseq(cur);

    /* ==== Exe開始 ==== */
    OutExeStart(cur->name, b);

    /* ==== Exe中 ==== */

    /* ② 他マシンのRead */
    if (can_read(other))
        do_read(other);

    /* ① 他マシンのOutSeq */
    if (other->buffer[other->next_out] == BUF_FILLED)
        do_outseq(other);

    /* ==== Exe本体 ==== */

    OutExe(cur->name, b);
    OutExeTerm(cur->name, b);

    /* ==== 完了処理 ==== */

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
        do_read(&B);

    printf("\n=== PIPELINE START ===\n\n");

    current = &A;

    int step = 0;
    while (A.out_count < TOTAL_DATA || B.out_count < TOTAL_DATA)
    {
        printf("\n---- STEP %d ----\n", ++step);
        other = (current == &A) ? &B : &A;

        /* 出力可能なときのみ実行 */
        if (current->out_count < TOTAL_DATA && current->buffer[current->next_out] != 0)
        {
            execute_one(current, other);
        }

        /* 交互 */
        current = other;
    }

    printf("\n=== FINISHED ===\n");
}
