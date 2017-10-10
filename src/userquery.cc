#include "userquery.h"

using namespace std;

bool UserQuery::Init(string behaver_message) {

    //if (InitRedis() == false) {
    //    return false;
    //}

    Parse(behaver_message);
    return true;
}

bool UserQuery::InitRedis() {
    redis_userid  = new Redis();
    if (!redis_userid->Connect("192.168.9.242", 3000, "MKL7cOEehQf8aoIBtHxs")) {
        LOG(WARNING) << "connect userid redis failed" ;
        return false;
    }

    return true;
}

bool UserQuery::Parse(string behaver_message) {
    //if(behaver_message.find("userid\":\"") == string::npos) {
    //    return false;
    //}
    //string user_id_md5 = behaver_message.substr(behaver_message.find("userid\":\"")+9, 32);
    //if (user_id_md5.size() != 32) {
    //    return false;
    //}

    //string value;
    //redis_userid->HGet("user_info_"+user_id_md5, "userid", &value);
    cout << behaver_message << endl;

    return true;
}
