#include <string.h>
#include <string>
#define STR_LENGTH 512
using namespace std;

enum messageType{MOVE, EXIT, NEWGAME, HANDSHAKE, NO_MESSAGE, MOVE_DENIED, MOVE_APPROVED, PLAYER_WIN, SERVER_WIN, CHECK_WIN, TIE};
static int _msgid = 0;

class message{
private:
    int id;
    char str[STR_LENGTH];
public:
    messageType type;
    message(messageType _type, const char* msg);
    message();
    int getid(){return id;};
    string get_str();
    void setempty();
    messageType get_type(){return type;};
};
