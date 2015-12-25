#include "message.h"

using namespace std;


message::message(messageType _type, const char* msg){
    id = _msgid;
    _msgid++;
    type = _type;
    memcpy(str, msg, STR_LENGTH);
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
