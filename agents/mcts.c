#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../game.h"
#include "mcts.h"
#include "time.h"
#include "util.h"

#define SCALE_FACTOR 1000

struct node {
    int move;
    char player;
    int n_visits;
    int score;
    struct node *parent;
    struct node *children[N_GRIDS];
};

static struct node *new_node(int move, char player, struct node *parent)
{
    struct node *node = malloc(sizeof(struct node));
    node->move = move;
    node->player = player;
    node->n_visits = 0;
    node->score = 0;
    node->parent = parent;
    memset(node->children, 0, sizeof(node->children));
    return node;
}

static void free_node(struct node *node)
{
    for (int i = 0; i < N_GRIDS; i++)
        if (node->children[i])
            free_node(node->children[i]);
    free(node);
}

static inline int uct_score(int n_total, int n_visits, int score)
{
    if (n_visits == 0)
        return INT_MAX;
    int score_per_visit = (score / n_visits);
    int total_factor = (int) (log(n_total) * SCALE_FACTOR);
    int sqrt_factor = (int) (sqrt(total_factor / n_visits));
    return score_per_visit +
           (EXPLORATION_FACTOR_FP * sqrt_factor / SCALE_FACTOR);
}

static struct node *select_move(struct node *node)
{
    struct node *best_node = NULL;
    int best_score = -1;
    for (int i = 0; i < N_GRIDS; i++) {
        if (!node->children[i])
            continue;
        int score = uct_score(node->n_visits, node->children[i]->n_visits,
                              node->children[i]->score);
        if (score > best_score) {
            best_score = score;
            best_node = node->children[i];
        }
    }
    return best_node;
}

static int simulate(char *table, char player)
{
    srand(time(0));
    char current_player = player;
    char temp_table[N_GRIDS];
    memcpy(temp_table, table, N_GRIDS);
    while (1) {
        int *moves = available_moves(temp_table);
        if (moves[0] == -1) {
            free(moves);
            break;
        }
        int n_moves = 0;
        while (n_moves < N_GRIDS && moves[n_moves] != -1)
            ++n_moves;
        int move = moves[rand() % n_moves];
        free(moves);
        temp_table[move] = current_player;
        char win = check_win(temp_table);
        if (win != ' ')
            return calculate_win_value(win, player);
        current_player ^= 'O' ^ 'X';
    }
    return SCALE_FACTOR / 2;
}

static void backpropagate(struct node *node, int score)
{
    while (node) {
        node->n_visits++;
        node->score += score;
        node = node->parent;
        score = 10 - score;
    }
}

static void expand(struct node *node, char *table)
{
    int *moves = available_moves(table);
    int n_moves = 0;
    while (n_moves < N_GRIDS && moves[n_moves] != -1)
        ++n_moves;
    for (int i = 0; i < n_moves; i++) {
        node->children[i] = new_node(moves[i], node->player ^ 'O' ^ 'X', node);
    }
    free(moves);
}

int mcts(char *table, char player)
{
    char win;
    struct node *root = new_node(-1, player, NULL);
    for (int i = 0; i < ITERATIONS; i++) {
        struct node *node = root;
        char temp_table[N_GRIDS];
        memcpy(temp_table, table, N_GRIDS);
        while (1) {
            win = check_win(temp_table);
            if (win != ' ') {
                int score = calculate_win_value(win, node->player ^ 'O' ^ 'X');
                backpropagate(node, score);
                break;
            }
            if (node->n_visits == 0) {
                int score = simulate(temp_table, node->player);
                backpropagate(node, score);
                break;
            }
            if (node->children[0] == NULL)
                expand(node, temp_table);
            node = select_move(node);
            assert(node);
            temp_table[node->move] = node->player ^ 'O' ^ 'X';
        }
    }
    struct node *best_node = NULL;
    int most_visits = -1;
    for (int i = 0; i < N_GRIDS; i++) {
        if (root->children[i] && root->children[i]->n_visits > most_visits) {
            most_visits = root->children[i]->n_visits;
            best_node = root->children[i];
        }
    }
    int best_move;
    if (best_node)
        best_move = best_node->move;
    else
        best_move = 0;
    free_node(root);
    return best_move;
}
