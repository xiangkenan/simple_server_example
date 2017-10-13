#include "userquery.h"

using namespace std;

UserQuery::UserQuery() {
    pre_trigger_config_min = 0;
    cur_trigger_config_min = 0;
}

bool UserQuery::Run(string behaver_message) {

    FreshTriggerConfig();

    if(!Parse(behaver_message)) {
        return false;
    }

    return true;
}

bool UserQuery::FreshTriggerConfig() {
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);

    cur_trigger_config_min = p->tm_min;

    if (cur_trigger_config_min == pre_trigger_config_min) {
        return true;
    }

    pre_trigger_config_min = cur_trigger_config_min;

    Redis redis_user_trigger_config;
    if (!redis_user_trigger_config.Connect("192.168.2.27", 6379, "spam_dev@ofo")) {
        LOG(WARNING) << "connect user_trigger_config redis failed" ;
        return false;
    }

    redis_user_trigger_config.Get("base_config", &base_value);

    parse_config();
    return true;
}

void UserQuery::parse_config() {
    Json::Value base_all_config;
    Json::Reader reader;
    reader.parse(base_value.c_str(), base_all_config);
    cout << base_all_config["dimensions"][0]["filters_list"].size() << endl;
    cout << base_all_config["dimensions"][1]["filters_list"].size() << endl;
    return;
}

bool UserQuery::Init() {
    redis_userid  = new Redis();
    if (!redis_userid->Connect("192.168.9.242", 3000, "MKL7cOEehQf8aoIBtHxs")) {
        LOG(WARNING) << "connect userid redis failed" ;
        return false;
    }

    return true;
}

//获取用户uid
bool UserQuery::Parse(string behaver_message) {
    if(behaver_message.find("userid\":\"") == string::npos) {
        return false;
    }
    string user_id_md5 = behaver_message.substr(behaver_message.find("userid\":\"")+9, 32);
    if (user_id_md5.size() != 32) {
        return false;
    }

    string value;
    redis_userid->HGet("user_info_"+user_id_md5, "userid", &value);
    if(value.empty()) {
        return false;
    }
    //赋值用户id
    uid = value;

    return true;
}
