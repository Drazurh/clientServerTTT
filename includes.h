#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

#include <stdlib.h>
#include <string.h>
#include <string>
#include <assert.h>

#include <sstream>

#include <time.h>

#include <ncurses.h>

#include <sys/time.h>

#include <csignal>

#define MAX_BUF 1024
#define READ 0
#define WRITE 1
#define DISPLAY_MSG_TYPE 1
#define HANDSHAKETIMEOUT 15;
enum cellState{EMPTY, PLAYER, SERVER};
enum gameState{PLAYER_TURN, COMP_TURN};

