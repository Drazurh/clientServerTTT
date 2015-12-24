#include "message.h"

using namespace std;


message::message(messageType _type, string msg){
    id = _msgid;
    _msgid++;
    type = _type;
    msg.copy(str, len);
}
message::message(){
    id = _msgid;
    _msgid++;
    type = NO_MESSAGE;
}
void message::setempty(){
    type = NO_MESSAGE;
}

string message::get_str(){
    return string(str);
}
