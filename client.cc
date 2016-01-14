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
        cursorX=1;
        cursorY=1;
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
    }

    void exitGame(){
        sendMessage(EXIT, "client exited game");
        close(writefd);
        close(readfd);
        endwin();
        exit(1);
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
        gameLoop();
    }

    void displayWin(cellState winner){
        if(winner == SERVER)
            mvprintw(16,1,"SERVER WINS");
        if(winner == PLAYER)
            mvprintw(16,1,"PLAYER WINS");
        if(winner == EMPTY)
            mvprintw(16,1,"TIE");

        mvprintw(17,1,"press any key to continue...");
        getch();
        reset();
        mvprintw(16,1,"                                                                      ");
        mvprintw(17,1,"                                                                      ");
    }

    void clearScreen(){
        for(int y=5; y<20; y++){
            mvprintw(y,0,"                                                                   ");
        }
        refresh();
    }

    void moveCursor(int _y, int _x){
        if(_x<0 || _x >2 || _y<0 || _y>2)
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
            case 0: screenY = 9; break;
            case 1: screenY = 11; break;
            case 2: screenY = 13; break;
            default: return;
        }
        switch(_x){
            case 0: screenX = 14; break;
            case 1: screenX = 19; break;
            case 2: screenX = 25; break;
            default: return;
        }
        screenX += xOffset;
        mvaddch(screenY,screenX, ch);
    }

    void makeMove(int _y, int _x){
        string txt;
        txt += patch::to_string(_y);
        txt += " ";
        txt += patch::to_string(_x);

        sendMessage(MOVE,txt);
        message msg;
        getNewMessage(msg);
        if(msg.type == MOVE_APPROVED || msg.type == SERVER_WIN){
            board[_y][_x] = PLAYER;
            displayToBoard(_y,_x, 0, 'X');
            stringstream stream;
            stream.str(msg.get_str());
            stream >> _y;
            stream >> _x;
            board[_y][_x] = SERVER;
            displayToBoard(_y,_x, 0, 'O');
        }
        switch(msg.type){
            case MOVE_APPROVED:

            break;
            case MOVE_DENIED:
                displayError("invalid move, please try again");
            break;
            case PLAYER_WIN:
                board[_y][_x] = PLAYER;
                displayToBoard(_y,_x, 0, 'X');
                displayWin(PLAYER);
            break;
            case SERVER_WIN:
                displayWin(SERVER);
            break;
            case TIE:
                board[_y][_x] = PLAYER;
                displayToBoard(_y,_x, 0, 'X');
                displayWin(EMPTY);
            break;
        }

    }

    void displayError(string str){
        mvprintw(16,1,str.c_str());
        mvprintw(17,1,"press any key to continue...");
        getch();
        mvprintw(16,1,"                                                                      ");
        mvprintw(17,1,"                                                                      ");

    }

    string getNewMessage(){
        read(readfd, readbuf, MAX_BUF);
        while(strcmp(readbuf, lastmsg) == 0){
            read(readfd, readbuf, MAX_BUF);
        }
        memcpy(lastmsg,readbuf,MAX_BUF);
        return string(lastmsg);
    }

    void getNewMessage(message& msg){
        read(readfd, readbuf, MAX_BUF);
        memcpy(&msg, readbuf, sizeof(msg));
    }

    void sendMessage(messageType msgType, string _msg){
        string msg;
        msg += _msg;
        msg += " ";
        msg += patch::to_string(static_cast<int>(time(NULL)));

        message tmpmsg(msgType, msg.c_str());
        writefd = open(fifos.clientFIFO().c_str(), O_WRONLY);
        write(writefd,&tmpmsg,sizeof(tmpmsg));
        close(writefd);
    }

    void connect(){
        char* initfifo = "/home/john/tmp/ttt";

        fifos = Fifos();
        fifos.create();

        int fd = open(initfifo, O_WRONLY);
        write(fd, &fifos, sizeof(fifos));
        close(fd);

        if(!handshake())
            exitGame();

    }

    bool handshake(){
        readfd = open(fifos.serverFIFO().c_str(), O_RDONLY);
        message msg;
        getNewMessage(msg);
        if(msg.get_type() != HANDSHAKE){
            displayError("Handshake failed, exiting");
            return false;
        }
        writefd = open(fifos.clientFIFO().c_str(), O_WRONLY);
        write(writefd, &msg, sizeof(msg));
        return true;
    }


};

TTTGame* myGame;
void exitRoutine(int s){
    myGame->exitGame();
    exit(1);
}

int main(){
    myGame = new TTTGame;
    signal(SIGINT, exitRoutine);
    myGame->startGame();
    myGame->exitGame();
    return true;
}
