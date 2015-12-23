#ifndef _INCL_GUARD
#define _INCL_GUARD
#include "includes.h"
#endif

#include "message.h"
#include "fifos.h"


using namespace std;


class TTTGame{

  public:
    char* writefifo;
    char* readfifo;
    int writefd;
    int readfd;
    char readbuf[MAX_BUF];
    char lastmsg[MAX_BUF];

    Fifos fifos;

    gameState state;
    cellState board[3][3];
    int cursorX;
    int cursorY;

    TTTGame(){
        connect();
        initscr();
        cbreak();
        keypad(stdscr, TRUE);
        noecho();
        curs_set(0);
    }

    void reset(){
        cursorX=2;
        cursorY=2;
        state = PLAYER_TURN;
        for(int i=0; i<3; i++)
            for(int j=0; j<3; j++)
                board[i][j]=EMPTY;

        clearScreen();
        displayBoardBackground();
        moveCursor(cursorX, cursorY);
        sendMessage(NEWGAME,"please start a new game");
    }

    void welcomeMessage(){
        mvprintw(4,15,"TIC*TAC*TOE");
        mvprintw(6,5,"Welcome to the tictactoe game!");
        mvprintw(7,5,"To play, move the cursor to an");
        mvprintw(8,5,"unlayed position and press the");
        mvprintw(9,5,"spacebar. You always go first");
        mvprintw(10,5,"and play x's. I play o's! Good");
        mvprintw(11,5,"luck!");
        mvprintw(14,1,"Enter any key to start the game. Press q anytime to quit.");
        refresh();
        getch();
    }

    void startGame(){
        welcomeMessage();
        reset();
        gameLoop();
        exitGame();
    }

    void exitGame(){
        close(writefd);
        close(readfd);
        endwin();
    }

    void displayBoardBackground(){
        mvprintw(7 ,1,"------------ Current Game -----------------");
        mvprintw(9 ,1,"               |      |                       ");
        mvprintw(10,1,"            --------------                    ");
        mvprintw(11,1,"               |      |                       ");
        mvprintw(12,1,"            --------------                    ");
        mvprintw(13,1,"               |      |                       ");
    }

    void gameLoop(){
        static int input;
        static bool valid;
        if(state == PLAYER_TURN){
            input = getch();

            switch(input){
                case 'q':
                    exitGame();
                break;
                case KEY_UP:
                    moveCursor(cursorY-1, cursorX);
                break;
                case KEY_DOWN:
                    moveCursor(cursorY+1, cursorX);
                break;
                case KEY_LEFT:
                    moveCursor(cursorY, cursorX-1);
                break;
                case KEY_RIGHT:
                    moveCursor(cursorY, cursorX+1);
                break;
                case ' ':
                    makeMove(cursorY, cursorX);
                break;

            }
        }
        else if(state == COMP_TURN){

        }
        else if(state == PLAYER_WIN){

        }
        else if(state == COMP_WIN){

        }
        gameLoop();
    }

    void clearScreen(){
        for(int y=5; y<20; y++){
            mvprintw(y,0,"                                                                   ");
        }
        refresh();
    }

    void moveCursor(int _y, int _x){
        if(_x<1 || _x >3 || _y<1 || _y>3)
            return;
        displayToBoard(cursorY,cursorX,-1, ' ');
        cursorY = _y;
        cursorX = _x;
        displayToBoard(cursorY,cursorX,-1, '*');
    }

    void displayToBoard(int _y, int _x, int xOffset, int ch){
        int screenX;
        int screenY;
        switch(_y){
            case 1: screenY = 9; break;
            case 2: screenY = 11; break;
            case 3: screenY = 13; break;
            default: return;
        }
        switch(_x){
            case 1: screenX = 14; break;
            case 2: screenX = 19; break;
            case 3: screenX = 25; break;
            default: return;
        }
        screenX += xOffset;
        mvaddch(screenY,screenX, ch);
    }

    void makeMove(int _y, int _x){
        string msg;
        msg += patch::to_string(_y);
        msg += " ";
        msg += patch::to_string(_x);

        sendMessage(MOVE,msg);
        msg = getNewMessage();
        if(strcmp(msg.c_str(), "valid") == 0){
            board[_y][_x] = PLAYER;
            displayToBoard(_y,_x, 0, 'X');
        }
        else if(strcmp(msg.c_str(), "invalid") == 0){
            displayError("Invalid move. Choose an empty space.");
        }
    }

    void displayError(char * str){
        mvprintw(16,1,str);
        mvprintw(17,1,"press any key to continue...");
        getch();
        mvprintw(16,1,"                                                                      ");

    }

    string getNewMessage(){
        read(readfd, readbuf, MAX_BUF);
        while(strcmp(readbuf, lastmsg) == 0){
            read(readfd, readbuf, MAX_BUF);
        }
        memcpy(lastmsg,readbuf,MAX_BUF);
        return string(lastmsg);
    }

    void sendMessage(messageType msgType, string _msg){
        string msg;
        msg += _msg;
        msg += " ";
        msg += patch::to_string(static_cast<int>(time(NULL)));

        message tmpmsg(msgType, msg);
        write(writefd,&tmpmsg,sizeof(tmpmsg));
    }

    void connect(){
        char* initfifo = "/home/john/tmp/ttt";

        fifos = Fifos();
        fifos.create();

        int fd = open(initfifo, O_WRONLY);
            write(fd, &fifos, sizeof(fifos));
        close(fd);

        writefd = open(fifos.clientFIFO.c_str(), O_WRONLY);
        readfd = open(fifos.serverFIFO.c_str(), O_RDONLY);
    }
};

int main(){
    TTTGame myGame;
    myGame.startGame();
}
