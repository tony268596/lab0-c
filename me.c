// me.c
#include "me.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

struct termios orig_termios;

void clear_screen()
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
}

void panic(const char *s)
{
    clear_screen();
    perror(s);
    puts("\r\n");
    exit(1);
}

void disable_raw_mode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        panic("Failed to disable raw mode");
}

void enable_raw_mode()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        panic("Failed to get current terminal state");
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        panic("Failed to set raw mode");
}

int read_key()
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if ((nread == -1) && (errno != EAGAIN) && (errno != EINTR))
            perror("Error reading input");
    }
    return c;
}

void process_key()
{
    int c = read_key();
    switch (c) {
    case CTRL_('q'):

        printf("Exiting program.\n");
        exit(0);
        break;
    case CTRL_('p'):

        printf("CTRL+P is pressed.\n");
        break;
    case CTRL_('x'):
        printf("hello\n");
        break;
    default:
        break;
    }
}
