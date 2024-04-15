#include "ttt.h"

int move_record[N_GRIDS];
int move_count = 0;
int move_record_arr[50][N_GRIDS];
int record_arr_len = 0;

/**/
void task_add(struct task *task)
{
    list_add_tail(&task->list, &tasklist);
}

void task_switch()
{
    if (!list_empty(&tasklist)) {
        struct task *t = list_first_entry(&tasklist, struct task, list);
        list_del(&t->list);
        cur_task = t;
        longjmp(t->env, 1);
    }
}

void schedule(void)
{
    static int i;
    setjmp(sched);

    while (ntasks-- > 0) {
        struct arg arg = args[i];
        tasks[i++](&arg);
        printf("Never reached\n");
    }

    task_switch();
}

void task0(void *arg)
{
    printf("\t\t\t111\t\t\t");
    struct task *task = malloc(sizeof(struct task));
    strncpy(task->task_name, ((struct arg *) arg)->task_name, 6);
    task->table = ((struct arg *) arg)->table;
    task->turn = ((struct arg *) arg)->turn;
    INIT_LIST_HEAD(&task->list);

    printf("%s: n = %c\n", task->task_name, task->turn);

    if (setjmp(task->env) == 0) {
        task_add(task);
        longjmp(sched, 1);
    }
    while (1) {
        task = cur_task;
        if (setjmp(task->env) == 0) {
            char win = check_win(task->table);
            if (win == 'D') {
                draw_board(task->table);
                printf("It is a draw!\n");
                break;
            }

            printf("%s: n = %c\n", task->task_name, task->turn);
            draw_board(task->table);
            print_current_time();
            int move = negamax_predict(task->table, task->turn).move;
            if (move != -1) {
                task->table[move] = task->turn;
                record_move(move);
            }

            task_add(task);
            task_switch();
        }
    }

    printf("%s: complete\n", task->task_name);
    longjmp(sched, 1);
}

void task1(void *arg)
{
    struct task *task = malloc(sizeof(struct task));
    strncpy(task->task_name, ((struct arg *) arg)->task_name, 6);
    task->table = ((struct arg *) arg)->table;
    task->turn = ((struct arg *) arg)->turn;
    INIT_LIST_HEAD(&task->list);

    printf("%s: n = %c\n", task->task_name, task->turn);

    if (setjmp(task->env) == 0) {
        task_add(task);
        longjmp(sched, 1);
    }
    while (1) {
        task = cur_task;
        if (setjmp(task->env) == 0) {
            char win = check_win(task->table);
            if (win == 'D') {
                draw_board(task->table);
                printf("It is a draw!\n");
                break;
            }

            printf("%s: n = %c\n", task->task_name, task->turn);
            draw_board(task->table);
            print_current_time();
            int move = mcts(task->table, task->turn);
            if (move != -1) {
                task->table[move] = task->turn;
                record_move(move);
            }

            task_add(task);
            task_switch();
        }
    }

    printf("%s: complete\n", task->task_name);
    longjmp(sched, 1);
}

/**/
void record_move(int move)
{
    move_record[move_count++] = move;
}

void print_moves()
{
    printf("Moves: ");
    for (int i = 0; i < move_count; i++) {
        printf("%c%d", 'A' + GET_COL(move_record[i]),
               1 + GET_ROW(move_record[i]));
        if (i < move_count - 1) {
            printf(" -> ");
        }
    }
    printf("\n");
}

void print_current_time()
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    printf("Current Time: %s\n", buffer);
}

int get_input(char player)
{
    char *line = NULL;
    size_t line_length = 0;
    int parseX = 1;

    int x = -1, y = -1;
    while (x < 0 || x > (BOARD_SIZE - 1) || y < 0 || y > (BOARD_SIZE - 1)) {
        printf("%c> ", player);
        int r = getline(&line, &line_length, stdin);
        if (r == -1)
            exit(1);
        if (r < 2)
            continue;
        x = 0;
        y = 0;
        parseX = 1;
        for (int i = 0; i < (r - 1); i++) {
            if (isalpha(line[i]) && parseX) {
                x = x * 26 + (tolower(line[i]) - 'a' + 1);
                if (x > BOARD_SIZE) {
                    x = BOARD_SIZE + 1;
                    printf("Invalid operation: index exceeds board size\n");
                    break;
                }
                continue;
            }
            if (x == 0) {
                printf("Invalid operation: No leading alphabet\n");
                y = 0;
                break;
            }
            parseX = 0;
            if (isdigit(line[i])) {
                y = y * 10 + line[i] - '0';
                if (y > BOARD_SIZE) {
                    y = BOARD_SIZE + 1;
                    printf("Invalid operation: index exceeds board size\n");
                    break;
                }
                continue;
            }
            printf("Invalid operation\n");
            x = y = 0;
            break;
        }
        x -= 1;
        y -= 1;
    }
    free(line);
    return GET_INDEX(y, x);
}

void main_ttt(int mode)
{
    negamax_init();  // Initialize Negamax agent

    srand(time(NULL));
    char table[N_GRIDS];

    void (*registered_task[])(void *) = {task0, task1};
    struct arg arg0 = {.table = table, .turn = 'X', .task_name = "Task 0"};
    struct arg arg1 = {.table = table, .turn = 'O', .task_name = "Task 1"};
    struct arg registered_arg[] = {arg0, arg1};

    while (1) {
        memset(table, ' ', N_GRIDS);  // Reset the board
        char turn = 'X';

        while (1) {
            char win = check_win(table);
            if (win == 'D') {
                draw_board(table);
                printf("It is a draw!\n");
                break;
            } else if (win != ' ') {
                draw_board(table);
                printf("%c won!\n", win);
                break;
            }

            if (mode) {  // Computer vs. Computer
                tasks = registered_task;
                args = registered_arg;
                ntasks = ARRAY_SIZE(registered_task);

                schedule();
                printf("\t\t\tout\t\t\n");
                for (int i = 0; i < N_GRIDS; i++) {
                    table[i] = ' ';
                }

                for (int i = 0; i < move_count; i++) {
                    move_record_arr[record_arr_len][i + 1] = move_record[i];
                    move_record[i] = 0;
                }
                move_record_arr[record_arr_len][0] = move_count;
                move_count = 0;
                record_arr_len++;
            } else {
                if (turn == 'X') {
                    draw_board(table);
                    int move;
                    while (1) {
                        move = get_input(turn);
                        if (table[move] == ' ') {
                            break;
                        }
                        printf(
                            "Invalid operation: the position has been "
                            "marked\n");
                    }
                    table[move] = turn;
                    record_move(move);
                } else {
                    draw_board(table);
                    print_current_time();
                    int move = mcts(table, turn);
                    if (move != -1) {
                        table[move] = turn;
                        record_move(move);
                    }
                }
            }

            turn = turn == 'X' ? 'O' : 'X';
        }
        print_moves();
    }
}