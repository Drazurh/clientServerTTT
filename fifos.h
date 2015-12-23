#include "patch.h"


using namespace std;

const string client("/tmp/tttClient_");
const string server("/tmp/tttServer_");

class Fifos{
private:
    pid_t pid;
public:
    Fifos(){
        pid = getpid();
    }
    Fifos operator = (Fifos other){
        pid = other.get_pid();
    }

    string pidstring(){
        return patch::to_string(static_cast<int>(pid));
    }

    string clientFIFO(){
        return client + pidstring();
    }
    string serverFIFO(){
        return server + pidstring();
    }
    int get_pid(){
        return pid;
    }
    void create(){
        int code1, code2;
        code1 = mkfifo(clientFIFO().c_str(), 0666);
        code2 = mkfifo(serverFIFO().c_str(), 0666);
        if(code1 == -1 || code2 == -1){
           perror("mkfifo returned an error - file may already exist");
        }
    }
};
