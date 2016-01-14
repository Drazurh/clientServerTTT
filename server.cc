
#ifndef _INCL_GUARD
#define _INCL_GUARD
#include "includes.h"
#endif

#include "message.h"
#include "fifos.h"

using namespace std;

int fd;

class TTTGameServer{
private:
    int id;
    int readfd;
    int writefd;
    Fifos fifos;
    char readbuf[MAX_BUF];
    int lastmsgid;
    cellState board[3][3];
public:
    TTTGameServer(Fifos _fifos, int clientID){

        fifos = _fifos;
        id = clientID;
        lastmsgid = -1;

    }

    void reset(){
        for(int i=0; i<3; i++)
            for(int j=0; j<3; j++)
                board[i][j]=EMPTY;
    }

    void getNewMessage(message& msg){
        read(readfd, readbuf, MAX_BUF);
        memcpy(&msg, readbuf, sizeof(msg));
    }

    bool handshake(){
        log("initiating handshake");
        message msg(HANDSHAKE, patch::to_string(fifos.get_pid()).c_str());
        writefd = open(fifos.serverFIFO().c_str(), O_WRONLY);
        write(writefd, &msg, sizeof(msg));

        readfd = open(fifos.clientFIFO().c_str(), O_RDONLY);
        getNewMessage(msg);
        if(msg.get_type() != HANDSHAKE){
            log("Handshake failed, exiting");
            return false;
        }
        log("success");
        return true;

    }

    void start(){
        if(!handshake())
            return;
        reset();
        stringstream stream;
        messageType msgType;
        message msg;
        bool done = false;
        while(!done){
            getNewMessage(msg);
            log("server received message:");
            log(msg.get_str());
            stream.flush();
            stream.str(msg.get_str());
            msgType = msg.get_type();
            switch(msgType){
                case MOVE:
                    makeMove(stream);
                break;
                case NEWGAME:
                    log("resetting");
                    reset();
                break;
                case EXIT:
                    log("exiting game");
                    close(writefd);
                    close(readfd);
                    done = true;
                break;
                default:
                    log("Server received empty or invalid message");
                break;


            }
        }

    }

    void log(string msg){
        cout << "client " << id << ": " << msg <<endl;
    }

    messageType winnerCellState(){
        int server, player, moveY, moveX, emptyX, emptyY;
        int totalFree = 9;
        //Horizontal
        for(int y=0; y<3; y++){
            server = 0;
            player = 0;
            for(int x=0; x<3; x++){
                if(board[y][x] == PLAYER){
                        player ++;
                        totalFree--;
                }else if(board[y][x] == SERVER){
                        server ++;
                        totalFree--;
                }
            }
            if(server == 3)
                return SERVER_WIN;
            if(player == 3)
                return PLAYER_WIN;

        }
        //Vertical
        for(int x=0; x<3; x++){
            server = 0;
            player = 0;
            for(int y=0; y<3; y++){
                if(board[y][x] == PLAYER){
                        player ++;
                }else if(board[y][x] == SERVER){
                        server ++;
                }
            }
            if(server == 3)
                return SERVER_WIN;
            if(player == 3)
                return PLAYER_WIN;
        }
        //Diagonal 1
        server = 0;
        player = 0;
        for(int y=0; y<3; y++){
            if(board[y][y] == PLAYER){
                    player ++;
            }else if(board[y][y] == SERVER){
                    server ++;
            }
        }
        if(server == 3)
            return SERVER_WIN;
        if(player == 3)
            return PLAYER_WIN;

        //Diagonal 2
        server = 0;
        player = 0;
        for(int y=0; y<3; y++){
            if(board[2-y][y] == PLAYER){
                    player ++;
            }else if(board[2-y][y] == SERVER){
                    server ++;
            }
        }
        if(server == 3)
            return SERVER_WIN;
        if(player == 3)
            return PLAYER_WIN;

        if(totalFree == 0)
            return TIE;
        return NO_MESSAGE;
    }


    string messageTypeString(messageType msgType){
        switch(msgType){
            case MOVE:
                return "MOVE";
            break;
            case EXIT:
                return "EXIT";
            break;
            case NEWGAME:
                return "NEWGAME";
            break;
            default:
                return "NO_MSG_TYPE_ERROR";
            break;
        }
    }

    void makeMove(stringstream& stream){
        int y, x;
        stream >> y;
        stream >> x;
        makeMove(y, x);
    }

    bool makeMove(int y, int x){
        messageType msgType = NO_MESSAGE;
        log("received client move");
        if(y<0 || y>2 || x<0 || x>2){
            log("client move out of bounds");
            log(patch::to_string(y) + string(" ") + patch::to_string(x) );
            return false;
        }
        if(board[y][x] == EMPTY){
            log("approved");
            board[y][x] = PLAYER;
            msgType = winnerCellState();
            if(msgType == PLAYER_WIN)
                sendClientMsg(PLAYER_WIN, "");
            else if(msgType == TIE){
                sendClientMsg(TIE,"");
            }
            else{
                string str = serverMove();
                msgType = winnerCellState();
                if(msgType == SERVER_WIN)
                    sendClientMsg(SERVER_WIN, str);
                else
                    sendClientMsg(MOVE_APPROVED, str);
            }

            return true;
        }else{
            log("denied");
            sendClientMsg(MOVE_DENIED, "");
            return false;
        }
    }

    string serverMove(){
        int max_line = 0;
        int server, player, moveY, moveX, emptyX, emptyY, free;
        //Horizontal
        for(int y=0; y<3; y++){
            server = 0;
            player = 0;
            free = 3;
            for(int x=0; x<3; x++){
                if(board[y][x] == PLAYER){
                        player ++;
                        free--;
                }else if(board[y][x] == SERVER){
                        server ++;
                        free--;
                }else if(board[y][x] == EMPTY){
                    emptyY = y;
                    emptyX = x;
                }
            }
            if(free > 0 && (player > max_line || server == 2)){
                max_line = player;
                moveY = emptyY;
                moveX = emptyX;
            }
        }
        //Vertical
        for(int x=0; x<3; x++){
            server = 0;
            player = 0;
            free = 3;
            for(int y=0; y<3; y++){
                if(board[y][x] == PLAYER){
                        player ++;
                        free--;
                }else if(board[y][x] == SERVER){
                        server ++;
                        free--;
                }else if(board[y][x] == EMPTY){
                    emptyY = y;
                    emptyX = x;
                }
            }
            if(free > 0 && (player > max_line || server == 2)){
                max_line = player;
                moveY = emptyY;
                moveX = emptyX;
            }
        }
        //Diagonal 1
        server = 0;
        player = 0;
        free = 3;
        for(int y=0; y<3; y++){
            if(board[y][y] == PLAYER){
                    player ++;
                    free--;
            }else if(board[y][y] == SERVER){
                    server ++;
                    free--;
            }else if(board[y][y] == EMPTY){
                emptyY = y;
                emptyX = y;
            }
        }
        if(free > 0 && (player > max_line || server == 2)){
            max_line = player;
            moveY = emptyY;
            moveX = emptyX;
        }
        //Diagonal 2
        server = 0;
        player = 0;
        free = 3;
        for(int y=0; y<3; y++){
            if(board[2-y][y] == PLAYER){
                    player ++;
                    free--;
            }else if(board[2-y][y] == SERVER){
                    server ++;
                    free--;
            }else if(board[2-y][y] == EMPTY){
                emptyY = 2-y;
                emptyX = y;
            }
        }
        if(free > 0 && (player > max_line || server == 2)){
            max_line = player;
            moveY = emptyY;
            moveX = emptyX;
        }
        board[moveY][moveX] = SERVER;
        string str;
        str += patch::to_string(moveY);
        str += " ";
        str += patch::to_string(moveX);
        return str;
    }

    message getClientMsg(){

        read(readfd, readbuf, MAX_BUF);
        message tmp;

        memcpy(&tmp,readbuf,sizeof(message));
        if(lastmsgid != tmp.getid()){
            lastmsgid = tmp.getid();
        }else{
            tmp = message();
        }

        return tmp;
    }

    void sendClientMsg(messageType _type, string txt){

        //int timestamp = static_cast<int>(time(NULL));
        //msg += " ";
        //msg += patch::to_string(timestamp);
        log("sending message");
        log(txt);
        message msg(_type, txt.c_str());
        write(writefd, &msg, sizeof(msg));

        log("done");
    }

};

int clientID = 0;
void newClient(Fifos fifos){

    cout << "Connecting to new client\n";
    clientID++;
    pid_t pid = fork();
    cout << "pid: " << pid << endl;;
    if(pid < 0){
        cout << "FORKED FAILED\n";
    }
    else if(pid > 0){
        cout <<"server forked for new client\n";
        //close(fd);
        TTTGameServer gameServer(fifos, clientID);
        gameServer.start();

    }
}

int main(){
    string myfifo = "/home/john/tmp/ttt";
    char buf[MAX_BUF];
    Fifos fifos = Fifos();
    pid_t prev_pid = -2;

    cout << "Server started, waiting for clients\n" ;
    fd = open(myfifo.c_str(), O_RDONLY);
    while(true){
        //cout << "waiting for new message...";
        read(fd, buf, MAX_BUF);
        //cout << "received\n";
        memcpy(&fifos,buf,sizeof(fifos));
        if(fifos.get_pid() != prev_pid){
            prev_pid  = fifos.get_pid();
            cout << "new message, id: " << prev_pid << endl;
            newClient(fifos);
        }
        sleep(1);
    }
    close(fd);
    return true;
}
