#include "userquery.h"

using namespace std;

UserQuery::UserQuery() {
    pre_trigger_config_min = 0;
    cur_trigger_config_min = 0;
}

bool UserQuery::Run(string behaver_message) {
    log_str = "";

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
    SendMessage();

    LOG(INFO) << log_str;
    return true;
}

Json::Value UserQuery::get_url_json(char* buf) {
    Json::Value result;
    char * ret_begin = buf + sizeof (uint32_t);
    char * ret_end = buf + sizeof (uint32_t) + *((uint32_t *) buf);
    *ret_end = '\0';

    bool rt = reader.parse(ret_begin, ret_end, result);
    if (!rt) {
        LOG(WARNING) << "murl_get_url parse failed url";
        return false;
    }

    return result;
}

bool UserQuery::SendMessage() {
    //LOG(INFO) << log_str;
    int ret;
    char buf[1024];
    string url = "http://192.168.3.127:9000/riskmgt/antispam?param=freq&bid=10038&kv1=activity,"+activity;
    if ((ret = murl_get_url(url.c_str(), buf, 10240, 0, NULL, NULL, NULL)) != MURLE_OK) {
        LOG(WARNING) << "riskmgt interface error";
        return false;
    }

    Json::Value result;
    result = get_url_json(buf);
    if (result["code"].asString() == "500") {
        log_str += "=>:hit_freq: activity full";
        return false;
    }

    memset(buf, 0, 1024);
    url = "http://192.168.3.127:9000/riskmgt/antispam?param=freq&bid=10038&kv1=user_id,"+ uid;
    if ((ret = murl_get_url(url.c_str(), buf, 10240, 0, NULL, NULL, NULL)) != MURLE_OK) {
        LOG(WARNING) << "riskmgt interface error";
        return false;
    }

    result = get_url_json(buf);
    if (result["code"].asString() == "500") {
        log_str += "=>:hit_freq: userid full";
        return false;
    }

    log_str += "=>send message";

    return true;
}

bool UserQuery::is_include(const BaseConfig& config, string user_msg) {
    string not_contain;
    vector<string> user_msg_vec;
    user_msg_vec.push_back(user_msg);
    if (config.filter_id == "userprofile.city") {
        not_contain = ".NOT_IN.";
    } else if (config.filter_id == "userprofile.competitor") {
        not_contain = "BITMAP_NONE";
        user_msg_vec.clear();
        if (user_msg.find("摩拜单车") != string::npos) 
            user_msg_vec.push_back("1");
        if (user_msg.find("小蓝单车") != string::npos) 
            user_msg_vec.push_back("2");
        if (user_msg.find("一步单车") != string::npos) 
            user_msg_vec.push_back("4");
        if (user_msg.find("优拜单车") != string::npos) 
            user_msg_vec.push_back("32");
        if (user_msg.find("hello单车") != string::npos) 
            user_msg_vec.push_back("64");
        if (user_msg.find("小鸣单车") != string::npos) 
            user_msg_vec.push_back("2048");
        if (user_msg.find("永安行") != string::npos) 
            user_msg_vec.push_back("16384");
    } else {
        not_contain = ".NOT_IN.";
    }

    if (config.option_id.find(not_contain) != string::npos) {
        for (unsigned int i = 0; i < config.values.size(); ++i) {
            for (unsigned int j =0; j < user_msg_vec.size(); ++j) {
                if(config.values[i] == user_msg_vec[j])
                    return false;
            }
        }
        return true;
    } else {
        for (unsigned int i = 0; i < user_msg_vec.size(); ++i){
            for (unsigned int j = 0; j < config.values.size(); ++j) {
                if (user_msg_vec[i] != config.values[j])
                    return false;
            }
        }
        return true;
    }

    return true;
}

bool UserQuery::is_confirm(const BaseConfig& config, string user_msg) {
    string not_contain;
    if (config.filter_id == "userprofile.oauth")
        not_contain = "NOT_EQUAL_TO";
    else if (config.filter_id == "userprofile.bond")
        not_contain = "EQUAL_TO";
    else if (config.filter_id == "userprofile.recharge")
        not_contain = "EQUAL_TO";
    else if (config.filter_id == "userprofile.device")
        not_contain = "FILTER..device..EQUAL_TO.INT32.OPTION_PARA.2";
    else
        not_contain = "NOT_EQUAL_TO";
    if (config.option_id.find(not_contain) != string::npos) {
        if (user_msg == "1")
            return false;
    } else {
        if(user_msg == "0")
            return false;
    }

    return true;
}

bool UserQuery::is_time_range(const BaseConfig& config, string user_msg) {
    struct tm tmp_time;
    strptime(user_msg.c_str(),"%Y%m%d %H:%M:%S",&tmp_time);
    time_t user_time, cur_time;
    user_time = mktime(&tmp_time);
    time(&cur_time);
    double cost = difftime(cur_time, user_time);
    if (config.option_id.find("-7D.") != string::npos) {
        if (cost > 604800)
            return false;
    } else if (config.option_id.find("-14D.") != string::npos) {
        if (cost > 1209600)
            return false;
    } else if (config.option_id.find("-30D.") != string::npos) {
        if (cost > 2592000)
            return false;
    } else if (config.option_id.find("OPTION_TIME_RANGE") != string::npos) {
        if (cost > 604800)
            return false;
    }

    return true;
}

bool UserQuery::is_time_range_value(const BaseConfig& config, string user_msg) {
    if (config.option_id.find("GREATER") != string::npos) {
        if (atoi(user_msg.c_str()) <= atoi(config.values[0].c_str()))
            return false;
    } else if (config.value_id.find("LESS") != string::npos) {
        if (atoi(user_msg.c_str()) >= atoi(config.values[0].c_str()))
            return false;
    } else if (config.value_id.find("BETWEEN") != string::npos) {
        if (atoi(user_msg.c_str()) < atoi(config.values[0].c_str()) || atoi(user_msg.c_str()) > atoi(config.values[1].c_str()) )
            return false;
    }

    return true;
}

bool UserQuery::is_range_value(const BaseConfig& config, string user_msg) {
    if (config.option_id.find("GREATER") != string::npos) {
        if (atoi(user_msg.c_str()) <= atoi(config.values[0].c_str()))
            return false;
    } else if (config.value_id.find("LESS") != string::npos) {
        if (atoi(user_msg.c_str()) >= atoi(config.values[0].c_str()))
            return false;
    } else if (config.value_id.find("BETWEEN") != string::npos) {
        if (atoi(user_msg.c_str()) <= atoi(config.values[0].c_str()))
            return false;
    }
    return true;
}

bool UserQuery::write_log(string msg, bool flag) {
    if (flag == true) {
        log_str += "&yes:" + msg;
    }
    else {
        log_str += "&no:" + msg;
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
bool UserQuery::data_core_operate(const BaseConfig& config, string user_msg, int flag) {
    bool ret;
    switch (flag) {
        case 1:
            ret = is_include(config, user_msg);
            return write_log(config.filter_id, ret);
        case 2:
            ret = is_confirm(config, user_msg);
            return write_log(config.filter_id, ret);
        case 3:
            ret = is_time_range(config, user_msg);
            return write_log(config.filter_id, ret);
        case 4:
            ret = is_time_range_value(config, user_msg);
            return write_log(config.filter_id, ret);
        case 5:
            ret = is_range_value(config, user_msg);
            return write_log(config.filter_id, ret);
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

    for (map<std::string, vector<BaseConfig>>::iterator iter = lasso_config_map.begin();
            iter != lasso_config_map.end();
            iter++) {
        int flag_hit = 0;
        log_str += "|activity:" + iter->first + "=>";
        for(unsigned int i = 0; i < iter->second.size(); ++i) {
            if (iter->second[i].filter_id == "userprofile.city") {
                if(!data_core_operate(iter->second[i], offline_data_json["rv"]["1"].asString(), 1)) {
                    flag_hit = -1;
                    break;
                }
            } else if (iter->second[i].filter_id == "userprofile.competitor") {
                if(!data_core_operate(iter->second[i], offline_data_json["rv"]["2"].asString(), 1)) {
                    flag_hit = -1;
                    break;
                }
            } else if (iter->second[i].filter_id == "userprofile.oauth") {
                if(!data_core_operate(iter->second[i], offline_data_json["rv"]["3"].asString(), 2)) {
                    flag_hit = -1;
                    break;
                }
            } else if (iter->second[i].filter_id == "userprofile.bond") {
                if(!data_core_operate(iter->second[i], offline_data_json["rv"]["4"].asString(), 2)) {
                    flag_hit = -1;
                    break;
                }
            } else if (iter->second[i].filter_id == "userprofile.recharge") {
                if(!data_core_operate(iter->second[i], offline_data_json["rv"]["5"].asString(), 2)) {
                    flag_hit = -1;
                    break;
                }
            } else if (iter->second[i].filter_id == "userprofile.device") {
                if(!data_core_operate(iter->second[i], offline_data_json["rv"]["6"].asString(), 2)) {
                    flag_hit = -1;
                    break;
                }
            } else if (iter->second[i].filter_id == "userprofile.reg_time") {
                if(!data_core_operate(iter->second[i], offline_data_json["rv"]["7"].asString(), 3)) {
                    flag_hit = -1;
                    break;
                }
            } else if (iter->second[i].filter_id == "userprofile.auth_time") {
                if(!data_core_operate(iter->second[i], offline_data_json["rv"]["8"].asString(), 3)) {
                    flag_hit = -1;
                    break;
                }
            } else if (iter->second[i].filter_id == "userprofile.first_order_time") {
                if(!data_core_operate(iter->second[i], offline_data_json["rv"]["1008"].asString(), 3)) {
                    flag_hit = -1;
                    break;
                }
            } else if (iter->second[i].filter_id == "order.order") {
                if(!data_core_operate(iter->second[i], offline_data_json["rv"]["1001"].asString(), 4)) {
                    flag_hit = -1;
                    break;
                }
            } else if (iter->second[i].filter_id == "order.repair_order") {
                if(!data_core_operate(iter->second[i], offline_data_json["rv"]["1002"].asString(), 4)) {
                    flag_hit = -1;
                    break;
                }
            } else if (iter->second[i].filter_id == "order.free_order") {
                if(!data_core_operate(iter->second[i], offline_data_json["rv"]["1003"].asString(), 4)) {
                    flag_hit = -1;
                    break;
                }
            } else if (iter->second[i].filter_id == "order.weekday_order") {
                if(!data_core_operate(iter->second[i], offline_data_json["rv"]["1004"].asString(), 4)) {
                    flag_hit = -1;
                    break;
                }
            } else if (iter->second[i].filter_id == "order.peak_order") {
                if(!data_core_operate(iter->second[i], offline_data_json["rv"]["1005"].asString(), 4)) {
                    flag_hit = -1;
                    break;
                }
            } else if (iter->second[i].filter_id == "order.month_card") {
                if(!data_core_operate(iter->second[i], offline_data_json["rv"]["8"].asString(), 5)) {
                    flag_hit = -1;
                    break;
                }
            }
        }

        for(unsigned int i = 0; i < offline_config_map[iter->first].size(); ++i) {
            if(offline_config_map[iter->first][i].filter_id == "offline.silence") {
                int silence_day = atoi(offline_data_json["rv"]["8"].asString().c_str());
                for (unsigned int j = 0; j < offline_config_map[iter->first][i].values.size(); ++j) {
                    if (silence_day != atoi(offline_config_map[iter->first][i].values[j].c_str())){
                        flag_hit = -1;
                    } else {
                        flag_hit = 0;
                        break;
                    }
                }
                if (flag_hit == -1) {
                    break;
                }
            }
        }

        if (flag_hit == 0) {
            log_str += "=>:hit_result: " + iter->first;
            return true;
        }
    }

    LOG(INFO) << log_str;
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

    redis_user_trigger_config->HGetAll("crm_noah_config1", &all_json);

    parse_noah_config();
    return true;
}

bool UserQuery::pretreatment(Json::Value all_config) {
    activity = all_config["activityId"].asString();
    if (all_config["status"].asString() != "true") {
        return false;
    }

    return true;
}

void UserQuery::parse_noah_config() {

    lasso_config_map.clear();

    for (map<string, string>::iterator iter = all_json.begin(); iter != all_json.end(); ++iter) {
        vector<BaseConfig> lasso_config_set, offline_config_set, real_config_set;
        Json::Value all_config, lasso_config, offline_config, real_config;
        reader.parse((iter->second).c_str(), all_config);

        if (!pretreatment(all_config)) {
            continue;
        }

        lasso_config = all_config["filter_list"];
        offline_config = all_config["jobArray"][0]["filters_list"];
        //real_config = all_config["jobArray"][1]["filter_list"];
        //cout << offline_config << endl;
        //cout << all_config << endl;


        //初始化圈选数据
        for (unsigned int i = 0; i < lasso_config.size(); ++i) {
            BaseConfig base_config;
            base_config.filter_id = lasso_config[i]["filter_id"].asString();
            base_config.option_id = lasso_config[i]["options"]["option_id"].asString();
            base_config.value_id = lasso_config[i]["values"]["value_id"].asString();
            if (base_config.value_id.find("LIST_MULTIPLE") != string::npos) {
                for (unsigned int j = 0; j < lasso_config[i]["values"]["list"].size(); ++j) {
                    base_config.values.push_back(lasso_config[i]["values"]["list"][j].asString());
                }
            } else if (base_config.value_id.find("VALUE_INPUT") != string::npos) {
                for (unsigned int j = 0; j < lasso_config[i]["values"]["input"].size(); ++j) {
                    base_config.values.push_back(lasso_config[i]["values"]["input"][j].asString());
                }
            }

            lasso_config_set.push_back(base_config);
        }
        lasso_config_map.insert(pair<string, vector<BaseConfig>>(iter->first, lasso_config_set));

        //初始化圈选数据
        for (unsigned int i = 0; i < offline_config.size(); ++i) {
            BaseConfig base_config;
            base_config.filter_id = offline_config[i]["filter_id"].asString();
            base_config.option_id = offline_config[i]["options"]["option_id"].asString();
            base_config.value_id = offline_config[i]["value"]["value_id"].asString();
            if (base_config.value_id.find("LIST_MULTIPLE") != string::npos) {
                for (unsigned int j = 0; j < offline_config[i]["values"]["list"].size(); ++j) {
                    base_config.values.push_back(offline_config[i]["values"]["list"][j].asString());
                }
            } else if (base_config.value_id.find("VALUE_INPUT") != string::npos) {
                for (unsigned int j = 0; j < offline_config[i]["values"]["input"].size(); ++j) {
                    base_config.values.push_back(offline_config[i]["values"]["input"][j].asString());
                }
            }

            offline_config_set.push_back(base_config);
        }
        offline_config_map.insert(pair<string, vector<BaseConfig>>(iter->first, offline_config_set));

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
    for (unsigned int i =0; i < user_json["content"].size(); ++i) {
        if (user_json["content"][i]["action"].asString() == "AppLaunch_Manner_00192") {
            action = "appstart";
        } else if (user_json["content"][i]["action"].asString() == "Visitor_click__00215" && user_json["content"][i]["params"]["more"]["click"].asString() == "start") {
            action = "appscan";
        } else {
            continue;
        }

        //string user_id_md5 = behaver_message.substr(behaver_message.find("userid\":\"")+9, 32);
        string user_id_md5 = user_json["content"][i]["userid"].asString();
        if (user_id_md5.size() != 32) {
            continue;
        }

        string value;
        redis_userid->HGet("user_info_"+user_id_md5, "userid", &value);
        if(value.empty()) {
            continue;
        }

        string value_tel;
        redis_userid->HGet("user_info_"+user_id_md5, "telephone", &value_tel);
        if(value_tel.empty()) {
            continue;
        }

        //赋值用户id,action
        uid = value;
        tel = value_tel;

        return true;
    }

    return false;

}
