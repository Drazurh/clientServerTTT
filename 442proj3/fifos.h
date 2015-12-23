#include "patch.h"

class Fifos{
public:
    string clientFIFO;
    string serverFIFO;

    Fifos(){
        clientFIFO = "/tmp/ttt_";
        serverFIFO = "/tmp/tttServer_";
        pid_t pid = getpid();
        string pidstring = patch::to_string(pid);
        clientFIFO += pidstring;
        serverFIFO += pidstring;
    }

    void create(){
        int code1, code2;
        code1 = mkfifo(clientFIFO.c_str(), 0666);
        code2 = mkfifo(serverFIFO.c_str(), 0666);
        if(code1 == -1 || code2 == -1){
           perror("mkfifo returned an error - file may already exist");
        }
    }
};
