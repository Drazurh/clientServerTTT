#include "patch.h"

class Fifos{
public:
    string clientReadFIFO;
    string clientWriteFIFO;
    string serverReadFIFO;
    string serverWriteFIFO;
    int clientfifo[2];
    int serverfifo[2];

    Fifos(){
        clientReadFIFO = "/home/john/tmp/tttClientRead_";
        clientWriteFIFO = "/home/john/tmp/tttClientWrite_";
        serverReadFIFO = "/home/john/tmp/tttServerRead_";
        serverWriteFIFO = "/home/john/tmp/tttServerWrite_";
        pid_t pid = getpid();
        string pidstring = patch::to_string(pid);

        clientReadFIFO += pidstring;
        clientWriteFIFO += pidstring;
        serverReadFIFO += pidstring;
        serverWriteFIFO += pidstring;
    }

    void create(){

        mkfifo(clientReadFIFO.c_str(), 0666);
        mkfifo(clientWriteFIFO.c_str(), 0666);
        mkfifo(serverReadFIFO.c_str(), 0666);
        mkfifo(serverWriteFIFO.c_str(), 0666);

        //clientfifo[WRITE] = open(clientWriteFIFO.c_str(), O_RDONLY);
        //serverfifo[READ] = open(clientWriteFIFO.c_str(), O_RDONLY);

    }
    void receive(){
        //clientfifo[READ] = open(clientWriteFIFO.c_str(), O_RDONLY);
        //serverfifo[WRITE] = open(clientWriteFIFO.c_str(), O_RDONLY);
    }
};
