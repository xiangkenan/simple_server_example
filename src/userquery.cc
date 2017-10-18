#include "userquery.h"

using namespace std;

UserQuery::UserQuery() {
    pre_trigger_config_min = 0;
    cur_trigger_config_min = 0;
}

bool UserQuery::Run(string behaver_message) {
    log_str = "";
    cout << behaver_message << endl;

    if(!FreshTriggerConfig()) {
        return false;
    }

    if(!Parse(behaver_message)) {
        return false;
    }

    if (!HandleProcess()) {
        return false;
    }

    //发短信
    //if (SendMessage()) {
    //}


    return true;
}

bool UserQuery::is_include(BaseConfig* config, string user_msg) {
    if (config->option_id.find(".NOT_IN.") != string::npos) {
        for (unsigned int i = 0; i < config->values.size(); ++i) {
            if(config->values[i] == user_msg)
                return false;
        }
    } else {
        for (unsigned int i = 0; i < config->values.size(); ++i) {
            if(config->values[i] != user_msg)
                return false;
        }
    }

    return true;
}

bool UserQuery::is_confirm(BaseConfig* config, string user_msg) {
    if (config->option_id.find(".NOT_EQUAL_TO.") != string::npos) {
        if (user_msg == "1")
            return false;
    } else {
        if(user_msg == "0")
            return false;
    }

    return true;
}

bool UserQuery::is_time_range(BaseConfig* config, string user_msg) {
    struct tm tmp_time;
    strptime(user_msg.c_str(),"%Y%m%d %H:%M:%S",&tmp_time);
    time_t user_time, cur_time;
    user_time = mktime(&tmp_time);
    time(&cur_time);
    double cost = difftime(cur_time, user_time);
    if (config->option_id.find("-7D.") != string::npos) {
        if (cost > 604800)
            return false;
    } else if (config->option_id.find("-14D.") != string::npos) {
        if (cost > 1209600)
            return false;
    } else if (config->option_id.find("-30D.") != string::npos) {
        if (cost > 2592000)
            return false;
    } else if (config->option_id.find("OPTION_TIME_RANGE") != string::npos) {
        if (cost > 604800)
            return false;
    }

    return true;
}

bool UserQuery::is_time_range_value(BaseConfig* config, string user_msg) {
    if (config->option_id.find("GREATER") != string::npos) {
        if (atoi(user_msg.c_str()) <= atoi(config->values[0].c_str()))
            return false;
    } else if (config->value_id.find("LESS") != string::npos) {
        if (atoi(user_msg.c_str()) >= atoi(config->values[0].c_str()))
            return false;
    } else if (config->value_id.find("BETWEEN") != string::npos) {
        if (atoi(user_msg.c_str()) <= atoi(config->values[0].c_str()))
            return false;
    }

    return true;
}

bool UserQuery::is_range_value(BaseConfig* config, string user_msg) {
    if (config->option_id.find("GREATER") != string::npos) {
        if (atoi(user_msg.c_str()) <= atoi(config->values[0].c_str()))
            return false;
    } else if (config->value_id.find("LESS") != string::npos) {
        if (atoi(user_msg.c_str()) >= atoi(config->values[0].c_str()))
            return false;
    } else if (config->value_id.find("BETWEEN") != string::npos) {
        if (atoi(user_msg.c_str()) <= atoi(config->values[0].c_str()))
            return false;
    }
    return true;
}

bool UserQuery::write_log(string msg, bool flag) {
    if (flag == true) {
        log_str = log_str +  "&yes:" + msg;
    }
    else {
        log_str = log_str + "&no:" + msg;
    }
    return flag;
}

/* flag参数解释
 * 1：包含， 不包含
 * 2:是， 否
 * 3:当前时间在 某个范围内
 * 4:规定范围内，订单是否满足条件（目前只判断是否满足条件）
 * 5:满足大于，小于，范围
*/
bool UserQuery::data_core_operate(BaseConfig* config, string user_msg, int flag) {
    bool ret;
    switch (flag) {
        case 1:
            ret = is_include(config, user_msg);
            return write_log(config->filter_id, ret);
        case 2:
            ret = is_confirm(config, user_msg);
            return write_log(config->filter_id, ret);
        case 3:
            ret = is_time_range(config, user_msg);
            return write_log(config->filter_id, ret);
        case 4:
            ret = is_time_range_value(config, user_msg);
            return write_log(config->filter_id, ret);
        case 5:
            ret = is_range_value(config, user_msg);
            return write_log(config->filter_id, ret);
        default:
            log_str += " 未知字段未满足";
            return false;
    }

    return true;
}

//1:满足配置 2:不满足配置 -1:出错
bool UserQuery::HandleProcess() {
    //get 用户数据
    string user_offline_data;
    Json::Value offline_data_json;

    redis_user_trigger_config->HGet("user_offline_data", "554345677", &user_offline_data);
    reader.parse(user_offline_data.c_str(), offline_data_json);

    
    for (unsigned int i = 0; i < lasso_config_set.size(); ++i) {
        if (lasso_config_set[i]->filter_id == "userprofile.city") {
            if(!data_core_operate(lasso_config_set[i], offline_data_json["rv"]["0"].asString(), 1))
                goto failed;
        } else if (lasso_config_set[i]->filter_id == "userprofile.competitor") {
            if(!data_core_operate(lasso_config_set[i], offline_data_json["rv"]["1"].asString(), 1))
                goto failed;
        } else if (lasso_config_set[i]->filter_id == "userprofile.oauth") {
            if(!data_core_operate(lasso_config_set[i], offline_data_json["rv"]["2"].asString(), 2))
                goto failed;
        } else if (lasso_config_set[i]->filter_id == "userprofile.bond") {
            if(!data_core_operate(lasso_config_set[i], offline_data_json["rv"]["3"].asString(), 2))
                goto failed;
        } else if (lasso_config_set[i]->filter_id == "userprofile.recharge") {
            if(!data_core_operate(lasso_config_set[i], offline_data_json["rv"]["4"].asString(), 2))
                goto failed;
        } else if (lasso_config_set[i]->filter_id == "userprofile.device") {
            if(!data_core_operate(lasso_config_set[i], offline_data_json["rv"]["5"].asString(), 2))
                goto failed;
        } else if (lasso_config_set[i]->filter_id == "userprofile.reg_time") {
            if(!data_core_operate(lasso_config_set[i], offline_data_json["rv"]["6"].asString(), 3))
                goto failed;
        } else if (lasso_config_set[i]->filter_id == "userprofile.auth_time") {
            if(!data_core_operate(lasso_config_set[i], offline_data_json["rv"]["7"].asString(), 3))
                goto failed;
        } else if (lasso_config_set[i]->filter_id == "userprofile.first_order_time") {
            if(!data_core_operate(lasso_config_set[i], offline_data_json["rv"]["8"].asString(), 3))
                goto failed;
        } else if (lasso_config_set[i]->filter_id == "order.order") {
            if(!data_core_operate(lasso_config_set[i], offline_data_json["rv"]["8"].asString(), 4))
                goto failed;
        } else if (lasso_config_set[i]->filter_id == "order.repair_order") {
            if(!data_core_operate(lasso_config_set[i], offline_data_json["rv"]["8"].asString(), 4))
                goto failed;
        } else if (lasso_config_set[i]->filter_id == "order.free_order") {
            if(!data_core_operate(lasso_config_set[i], offline_data_json["rv"]["8"].asString(), 4))
                goto failed;
        } else if (lasso_config_set[i]->filter_id == "order.weekday_order") {
            if(!data_core_operate(lasso_config_set[i], offline_data_json["rv"]["8"].asString(), 4))
                goto failed;
        } else if (lasso_config_set[i]->filter_id == "order.peak_order") {
            if(!data_core_operate(lasso_config_set[i], offline_data_json["rv"]["8"].asString(), 4))
                goto failed;
        } else if (lasso_config_set[i]->filter_id == "order.month_card") {
            if(!data_core_operate(lasso_config_set[i], offline_data_json["rv"]["8"].asString(), 5))
                goto failed;
        }
    }

    LOG(INFO) << log_str;
    return true;

failed:
    LOG(INFO) << log_str << endl;
    return false;
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

    redis_user_trigger_config->HGet("crm_noah_config", "hongbao", &all_json);


    parse_noah_config();
    return true;
}

void UserQuery::parse_noah_config() {
    Json::Value all_config, lasso_config, offline_config, real_config;
    reader.parse(all_json.c_str(), all_config);

    lasso_config = all_config["filter_list"];
    offline_config = all_config["jobArray"][0]["filter_list"];
    real_config = all_config["jobArray"][1]["filter_list"];

    //释放圈选数据
    for (unsigned int i = 0; i < lasso_config_set.size(); ++i) {
        delete lasso_config_set[i];
    }
    lasso_config_set.clear();

    //初始化圈选数据
    for (unsigned int i = 0; i < lasso_config.size(); ++i) {
        BaseConfig *base_config = new BaseConfig();
        base_config->filter_id = lasso_config[i]["filter_id"].asString();
        base_config->option_id = lasso_config[i]["options"]["option_id"].asString();
        base_config->value_id = lasso_config[i]["values"]["value_id"].asString();
        if (base_config->value_id.find("LIST_MULTIPLE") != string::npos) {
            for (unsigned int j = 0; j < lasso_config[i]["values"]["list"].size(); ++j) {
                base_config->values.push_back(lasso_config[i]["values"]["list"][j].asString());
            }
        } else if (base_config->value_id.find("VALUE_INPUT") != string::npos) {
            for (unsigned int j = 0; j < lasso_config[i]["values"]["input"].size(); ++j) {
                base_config->values.push_back(lasso_config[i]["values"]["input"][j].asString());
            }
        }

        lasso_config_set.push_back(base_config);
    }

    return;
}

bool UserQuery::Init() {
    redis_userid  = new Redis();
    if (!redis_userid->Connect("192.168.9.242", 3000, "MKL7cOEehQf8aoIBtHxs")) {
        LOG(WARNING) << "connect userid redis failed" ;
        return false;
    }

    redis_user_trigger_config = new Redis();
    if (!redis_user_trigger_config->Connect("192.168.2.27", 6379, "spam_dev@ofo")) {
        LOG(WARNING) << "connect user_trigger_config redis failed" ;
        return false;
    }

    return true;
}

//获取用户uid,action
bool UserQuery::Parse(string behaver_message) {

    if(behaver_message.find("userid\":\"") == string::npos && behaver_message.find("action\":\"") == string::npos) {
        return false;
    }

    Json::Value user_json;

    string json_behaver_message = behaver_message.substr(behaver_message.find("body\":")+6, string::npos);
    json_behaver_message[json_behaver_message.size()-1] = '\0';
    reader.parse(json_behaver_message.c_str(), user_json);

    //string user_id_md5 = behaver_message.substr(behaver_message.find("userid\":\"")+9, 32);
    string user_id_md5 = user_json["content"][0]["userid"].asString();
    if (user_id_md5.size() != 32) {
        return false;
    }

    string value;
    redis_userid->HGet("user_info_"+user_id_md5, "userid", &value);
    if(value.empty()) {
        return false;
    }

    //赋值用户id,action
    uid = value;
    action = user_json["content"][0]["action"].asString();
    LOG(INFO) << uid;

    return true;
}
