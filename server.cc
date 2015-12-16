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

#define MAX_BUF 1024
#define READ 0
#define WRITE 1

#define DISPLAY_MSG_TYPE 1


#include "message.h"

namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

using namespace std;
enum cellState{EMPTY, PLAYER, SERVER};

class newClientMessage{
public:
    int clientfd[2];
    int serverfd[2];
    newClientMessage(){

    }
    void create(){
        pipe(clientfd);
        pipe(serverfd);
    }

    void receive(){

        //close(serverfd[READ]);
        //close(clientfd[WRITE]);
    }
};

static int client_id = 0;

class TTTGameServer{
private:
    int id;
    int readfd;
    int writefd;
    char readbuf[MAX_BUF];
    message lastmsg;
    cellState board[3][3];
public:
    TTTGameServer(int _readfd, int _writefd){
        id = client_id;
        client_id ++;
        readfd = _readfd;
        writefd = _writefd;

    }

    void reset(){
        for(int i=0; i<3; i++)
            for(int j=0; j<3; j++)
                board[i][j]=EMPTY;
    }

    void start(){
        reset();
        stringstream stream;
        messageType msgType;
        message msg = getClientMsg();


            cout << "loop\n";
            if(!msg.str.empty() && false){
                stream.flush();
                stream << msg.str;
                msgType = msg.type;
                if(DISPLAY_MSG_TYPE){
                    log(messageTypeString(msgType));
                }
                if(msgType == messageType(MOVE)){
                    makeMove(stream);
                }
                else if(msgType == messageType(NEWGAME)){
                     cout <<"resetting";
                    reset();
                }
                else if(msgType == messageType(EXIT)){
                    exit_game();

                }
            }
            msg = getClientMsg();

        cout <<"exiting loop";
    }

    void exit_game(){

        log("Exiting");
        exit(1);
    }

    void log(string msg){
        cout << "client " << id << ": " << msg <<endl;
    }

    string messageTypeString(messageType msgType){
        switch(msgType){
            case messageType(MOVE):
                return "MOVE";
            break;
            case messageType(EXIT):
                return "EXIT";
            break;
            case messageType(NEWGAME):
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
        bool validMove = makeMove(y, x);
        if(validMove)
            sendClientMsg("valid");
        else
            sendClientMsg("invalid");
    }
    bool makeMove(int y, int x){
        if(board[y][x] == EMPTY){
            board[y][x] = PLAYER;
            return true;
        }
        return false;
    }

    message getClientMsg(){
        log(string("getting message from ") + patch::to_string(readfd));

        read(readfd, readbuf, MAX_BUF);
        message tmp;

        memcpy(&tmp,readbuf,sizeof(message));
        if(lastmsg.id != tmp.id){
            lastmsg = tmp;
        }else{
            tmp = message();

        }
        cout <<"returning message\n";
        return tmp;
    }

    void sendClientMsg(string msg){
        cout <<"sending message\n";
        int timestamp = static_cast<int>(time(NULL));
        msg += " ";
        msg += patch::to_string(timestamp);
        write(writefd, msg.c_str(), MAX_BUF);
        cout << "done\n";
    }

};

//http://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = static_cast<char**>(malloc(sizeof(char*) * count));

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

void newClient(newClientMessage newclientmsg){
    cout << "Connecting to new client \nServer writing to "
        << newclientmsg.serverfd[WRITE]
        << "\nServer reading from "
        << newclientmsg.clientfd[READ]
        << "\nClient writing to "
        << newclientmsg.clientfd[WRITE]
        << "\nClient reading from "
        << newclientmsg.serverfd[READ]
        << endl;
    pid_t pid = fork();

    if(pid < 0){
        cout << "FORKED FAILED\n";
    }
    else if(pid > 0){

        TTTGameServer gameServer(newclientmsg.clientfd[READ], newclientmsg.serverfd[WRITE]);
        gameServer.start();
    }
}

//void handleInput(char* msg){
//    char** tokens;
//    tokens = str_split(msg, ',');
//    string clientWrite;
//    string serverWrite;
//    string command;
//    if (tokens)
//    {
//        int i;
//        for (i = 0; *(tokens + i); i++)
//        {
//            if(i==0){
//                command = string(*(tokens + i));
//            }
//            if(i==1)
//                clientWrite = string(*(tokens + i));
//            if(i==2)
//                serverWrite = string(*(tokens + i));
//            free(*(tokens + i));
//        }
//        free(tokens);
//    }
//    cout <<"\nclient: " <<clientWrite << "\nserver: "<<serverWrite<<endl;
//    if(strcmp(command.c_str(),"newClient") == 0){
//        newClient(clientWrite, serverWrite);
//    }
//}

int main(){
    int fd;
    char * myfifo = "/home/john/tmp/ttt";
    char buf[MAX_BUF];
    char msg[MAX_BUF];
    fd = open(myfifo, O_RDONLY);
    newClientMessage newclientmessage;
    while(true){
        read(fd, buf, MAX_BUF);
        if(strcmp(buf,msg) != 0){
            memcpy(msg,buf,MAX_BUF);
            memcpy(&newclientmessage,msg,sizeof(newClientMessage));
            newClient(newclientmessage);
        }
    }
    close(fd);
    return true;
}
