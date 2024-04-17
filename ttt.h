#ifndef TTT_H
#define TTT_H

#include <ctype.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "agents/mcts.h"
#include "agents/negamax.h"
#include "game.h"
#include "list.h"
#include "me.h"
#include "string.h"

extern int move_record[N_GRIDS];
extern int move_count;
extern int move_record_arr[50][N_GRIDS];
extern int record_arr_len;

#define BOARD_SIZE 4
#define GOAL 3
#define ALLOW_EXCEED 1
#define N_GRIDS (BOARD_SIZE * BOARD_SIZE)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/**/
struct task {
    jmp_buf env;
    struct list_head list;
    char task_name[10];
    char *table;
    char turn;
};

struct arg {
    char *task_name;
    char *table;
    char turn;
};

static LIST_HEAD(tasklist);
static struct task *cur_task;
static void (**tasks)(void *);
static struct arg *args;
static int ntasks;
static jmp_buf sched;

void task_add(struct task *task);
void task_switch();
void schedule(void);
void task0(void *arg);
void task1(void *arg);
/**/

// Declaration of functions
void record_move(int move);
void print_moves();
int get_input(char player);
void print_current_time();
void main_ttt(int mode);

#endif