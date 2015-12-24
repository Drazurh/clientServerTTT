
#ifndef _INCL_GUARD
#define _INCL_GUARD
#include "includes.h"
#endif

#include "message.h"
#include "fifos.h"

using namespace std;

static int client_id = 0;
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
    TTTGameServer(Fifos _fifos){

        fifos = _fifos;
        readfd = open(fifos.clientFIFO().c_str(), O_RDONLY);
        writefd = open(fifos.serverFIFO().c_str(), O_WRONLY);
        id = client_id;
        client_id ++;
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
        cout << "initiating handshake";
        message msg(HANDSHAKE, patch::to_string(fifos.get_pid()));
        writefd = open(fifos.clientFIFO().c_str(), O_WRONLY);
        write(writefd, &msg, sizeof(msg));

        readfd = open(fifos.clientFIFO().c_str(), O_RDONLY);
        getNewMessage(msg);
        if(msg.get_type() != HANDSHAKE){
            log("Handshake failed, exiting");
            return false;
        }

        return true;

    }

    void start(){
        if(!handshake())
            exit_game();
        reset();
        stringstream stream;
        messageType msgType;
        message msg = getClientMsg();
        while(true){
            if(!msg.get_str().empty()){
                stream.flush();
                stream << msg.get_str();
                msgType = msg.get_type();

                if(msgType == MOVE){
                    makeMove(stream);
                }
                else if(msgType == NEWGAME){
                    log("resetting");
                    reset();
                }
                else if(msgType == EXIT){
                    exit_game();

                }
                else{
                    log("Server received empty or invalid message");
                }
            }
            msg = getClientMsg();
        }


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

    void sendClientMsg(string msg){
        log("sending message");
        int timestamp = static_cast<int>(time(NULL));
        msg += " ";
        msg += patch::to_string(timestamp);
        write(writefd, msg.c_str(), MAX_BUF);
        log("done");
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

void newClient(Fifos fifos){
    cout << "Connecting to new client\n";
//        << "Server writing to "
//        << fifos.serverFIFO
//        << "\nServer reading from "
//        << fifos.clientFIFO
//        << endl;
    pid_t pid = fork();
    cout << "pid: " << pid << endl;;
    if(pid < 0){
        cout << "FORKED FAILED\n";
    }
    else if(pid > 0){
        cout <<"server forked for new client\n";
        //close(fd);
        TTTGameServer gameServer(fifos);
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


    string myfifo = "/home/john/tmp/ttt";
    char buf[MAX_BUF];
    Fifos fifos = Fifos();
    pid_t prev_pid = -2;

    cout << "Server started, waiting for clients\n" ;
    fd = open(myfifo.c_str(), O_RDONLY);
    while(true){
        cout << "waiting for new message...";
        read(fd, buf, MAX_BUF);
        cout << "received\n";
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
