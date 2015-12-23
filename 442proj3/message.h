#include <string.h>
#include <string>

using namespace std;

enum messageType{MOVE, EXIT, NEWGAME, NO_MESSAGE};



class message{
public:
		int id;
    messageType type;
    string str;
    message(messageType _type, string msg);
    message();

};
