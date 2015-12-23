#include "message.h"

using namespace std;

static int _id = 0;
message::message(messageType _type, string msg){
		id = _id;
		_id++;
    type = _type;
    str = msg;
}
message::message(){
    id = _id;
    _id++;
    type = NO_MESSAGE;
    str = string();
}

