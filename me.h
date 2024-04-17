
#ifndef ME_H
#define ME_H

#include <termios.h>

#define CTRL_(k) ((k) &0x1f)

extern struct termios orig_termios;

void disable_raw_mode();
void enable_raw_mode();
int read_key();
void clear_screen();
void process_key();
void panic(const char *msg);


#endif
