#ifndef TTT_H
#define TTT_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "agents/mcts.h"
#include "agents/negamax.h"
#include "game.h"

extern int move_record[N_GRIDS];
extern int move_count;

// Declaration of functions
void record_move(int move);
void print_moves();
int get_input(char player);

#endif