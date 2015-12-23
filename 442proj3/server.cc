
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
    message lastmsg;
    cellState board[3][3];
public:
    TTTGameServer(Fifos _fifos){

        fifos = _fifos;
        readfd = open(fifos.clientFIFO.c_str(), O_RDONLY);
        writefd = open(fifos.serverFIFO.c_str(), O_WRONLY);
        id = client_id;
        client_id ++;


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

void newClient(Fifos fifos){
    cout << "Connecting to new client \nServer writing to "
        << fifos.serverFIFO
        << "\nServer reading from "
        << fifos.clientFIFO
        << endl;
    pid_t pid = fork();

    if(pid < 0){
        cout << "FORKED FAILED\n";
    }
    else if(pid > 0){
        cout << "forking server\n";
        sleep(1000);
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


    char * myfifo = "/home/john/tmp/ttt";
    char buf[MAX_BUF];
    char msg[MAX_BUF];
    Fifos fifos = Fifos();


    cout << "Server started, waiting for clients\n";
    while(true){
        fd = open(myfifo, O_RDONLY);

        read(fd, buf, sizeof(fifos));
        if(strcmp(buf,msg) != 0){
            memcpy(msg,buf,sizeof(fifos));
            memcpy(&fifos,msg,sizeof(fifos));
            newClient(fifos);
        }
        close(fd);
    }

    return true;
}
