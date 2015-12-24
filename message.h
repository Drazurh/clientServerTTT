#include <string.h>
#include <string>
#define STR_LENGTH
using namespace std;

enum messageType{MOVE, EXIT, NEWGAME, HANDSHAKE, NO_MESSAGE};
const int len = 512;
static int _msgid = 0;

class message{
private:
    int id;
    char str[STR_LENGTH];
public:
    messageType type;
    message(messageType _type, string msg);
    message();
    int getid(){return id;};
    string get_str();
    void setempty();
    messageType get_type(){return type;};
};
