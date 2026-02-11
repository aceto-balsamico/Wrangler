#include <stdio.h>

#define BUF_NUM 4
#define TOTAL_DATA 10

typedef struct {
    char name;
    int read_count;
    int out_count;
    int buffer[BUF_NUM];   // 0=empty, 1=filled, 2=seq済
    int next_read;
    int next_out;
} Machine;


/* ==== API ==== */

void Read(char m, int b) {
    printf("[%c] Read(%d)\n", m, b);
}

void OutSeq(char m, int b) {
    printf("[%c] OutSeq(%d)\n", m, b);
}

void OutExeStart(char m, int b) {
    printf("[%c] SCE(%d)\n", m, b);
    // printf("---- EXEC START [%c:%d] ----\n", m, b);
}

void OutExe(char m, int b) {
    printf("[%c] Out(%d)\n", m, b);
	printf("---- EXECUTION[%c:%d] ----\n", m, b);
}

void OutExeTerm(char m, int b) {
    // printf("---- EXEC END   [%c:%d] ----\n", m, b);
    printf("[%c] SCT(%d)\n", m, b);
}


/* ==== 初期化 ==== */

void init_machine(Machine *m, char name)
{
    int i;
    m->name = name;
    m->read_count = 0;
    m->out_count = 0;
    m->next_read = 0;
    m->next_out = 0;

    for (i = 0; i < BUF_NUM; i++)
        m->buffer[i] = 0;
}


/* ==== 補助 ==== */

int can_read(Machine *m)
{
    if (m->read_count >= TOTAL_DATA)
        return 0;

    if (m->buffer[m->next_read] != 0)
        return 0;

    return 1;
}

void do_read(Machine *m)
{
    int b = m->next_read;

    Read(m->name, b);

    m->buffer[b] = 1;
    m->read_count++;
    m->next_read = (m->next_read + 1) % BUF_NUM;
}

int can_outseq(Machine *m)
{
    return (m->buffer[m->next_out] == 1);
}

void do_outseq(Machine *m)
{
    int b = m->next_out;

    OutSeq(m->name, b);
    m->buffer[b] = 2;
}

int can_outexe(Machine *m)
{
    return (m->buffer[m->next_out] == 2);
}

void do_outexe(Machine *m)
{
    int b = m->next_out;

    OutExeStart(m->name, b);
    OutExe(m->name, b);
    OutExeTerm(m->name, b);

    m->buffer[b] = 0;
    m->out_count++;
    m->next_out = (m->next_out + 1) % BUF_NUM;
}


/* ==== main ==== */

void main(void)
{
    Machine A, B;
    Machine *current;
    Machine *other;

    init_machine(&A, 'A');
    init_machine(&B, 'B');

    printf("=== INITIAL READ (4 each) ===\n");

    while (A.read_count < 4)
        do_read(&A);

    while (B.read_count < 4)
        do_read(&B);

    printf("\n=== START PIPELINE ===\n\n");

    current = &A;

	int step = 0;
    while (A.out_count < TOTAL_DATA || B.out_count < TOTAL_DATA)
    {
		printf("\n---- STEP %d ----\n", ++step);
        other = (current == &A) ? &B : &A;

        /* まずOutSeqがまだなら撃つ */
        if (can_outseq(current))
        {
            do_outseq(current);
        }

        /* Exe可能なら実行 */
        if (can_outexe(current))
        {
            /* ===== Exe中 ===== */

            OutExeStart(current->name, current->next_out);

            /* 他マシンのOutSeq */
            if (can_outseq(other))
                do_outseq(other);

            /* 他マシンのRead */
            if (can_read(other))
                do_read(other);

            OutExe(current->name, current->next_out);
            OutExeTerm(current->name, current->next_out);

            current->buffer[current->next_out] = 0;
            current->out_count++;
            current->next_out = (current->next_out + 1) % BUF_NUM;
        }

        /* 交互 */
        current = other;
    }

    printf("\n=== FINISHED ===\n");
}
