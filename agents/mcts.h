#pragma once

#define ITERATIONS 100000
#define EXPLORATION_FACTOR_FP (int) (sqrt(2) * 1000)

int mcts(char *table, char player);
