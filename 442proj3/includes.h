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

#define MAX_BUF 1024
#define READ 0
#define WRITE 1
#define DISPLAY_MSG_TYPE 1
enum cellState{EMPTY, PLAYER, SERVER};
enum gameState{PLAYER_TURN, COMP_TURN, PLAYER_WIN, COMP_WIN};

