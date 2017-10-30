#include "userquery.h"

using namespace std;

UserQuery::UserQuery() {
    run_ = false;

    time_range_file.push_back("order.order");
    time_range_file.push_back("order.repair_order");
    time_range_file.push_back("order.free_order");
    time_range_file.push_back("order.weekday_order");
    time_range_file.push_back("order.peak_order");
    time_range_file.push_back("offline.bikeFailed");
}

bool UserQuery::InitRedis(Redis* redis_userid, Redis* redis_user_trigger_config) {
    if (!redis_userid->Connect("192.168.9.242", 3000, "MKL7cOEehQf8aoIBtHxs")) {
        LOG(WARNING) << "connect userid redis failed" ;
        return false;
    }

    if (!redis_user_trigger_config->Connect("192.168.2.27", 6379, "spam_dev@ofo")) {
        LOG(WARNING) << "connect user_trigger_config redis failed" ;
        return false;
    }

    return true;

}

bool UserQuery::Run(string behaver_message) {
    KafkaData kafka_data;

    Redis redis_userid, redis_user_trigger_config;

    if(!InitRedis(&redis_userid, &redis_user_trigger_config)) {
        return false;
    }

    if(!Parse_kafka_data(&redis_userid, &redis_user_trigger_config, behaver_message, &kafka_data)) {
        return false;
    }

    if (!HandleProcess(&redis_userid, &redis_user_trigger_config, &kafka_data)) {
        return false;
    }

    //发短信
    SendMessage(&kafka_data);

    LOG(INFO) << kafka_data.log_str;
    return true;
}

bool UserQuery::SendMessage(KafkaData* kafka_data) {
    int ret;
    char buf[1024];
    string url = "http://192.168.3.127:9000/riskmgt/antispam?param=freq&bid=10038&kv1=activity," + kafka_data->action_id;
    if ((ret = murl_get_url(url.c_str(), buf, 10240, 0, NULL, NULL, NULL)) != MURLE_OK) {
        LOG(WARNING) << "riskmgt interface error";
        return false;
    }

    Json::Value result;
    result = get_url_json(buf);
    if (result["code"].asString() == "500") {
        kafka_data->log_str += "=>:hit_freq: activity full";
        return false;
    }

    memset(buf, 0, 1024);
    url = "http://192.168.3.127:9000/riskmgt/antispam?param=freq&bid=10038&kv1=user_id,"+ kafka_data->uid;
    if ((ret = murl_get_url(url.c_str(), buf, 10240, 0, NULL, NULL, NULL)) != MURLE_OK) {
        LOG(WARNING) << "riskmgt interface error";
        return false;
    }

    result = get_url_json(buf);
    if (result["code"].asString() == "500") {
        kafka_data->log_str += "=>:hit_freq: userid full";
        return false;
    }

    kafka_data->log_str += "=>send message";

    return true;
}

//1:满足配置 2:不满足配置 -1:出错
bool UserQuery::HandleProcess(Redis* redis_userid, Redis* redis_user_trigger_config, KafkaData *kafka_data) {
    kafka_data->log_str = kafka_data->uid;
    //初始化配置操作类
    NoahConfigRead noah_config_read(time_range_origin);
    for (unordered_map<std::string, vector<BaseConfig>>::iterator iter = lasso_config_map.begin();
            iter != lasso_config_map.end();
            iter++) {
        int flag_hit = 0;
        kafka_data->log_str += "|activity:" + iter->first + "=>";
        for(unsigned int i = 0; i < iter->second.size(); ++i) {
            BaseConfig cc = iter->second[i];
            //判断app触发条件是否满足
            if (cc.filter_id == "realtime.app.action") {
                if ((cc.option_id == "app.action.appon" && kafka_data->action == "appscan") || 
                        (cc.option_id == "app.action.scan" && kafka_data->action == "appstart")) {
                    kafka_data->log_str += "&no action";
                    flag_hit = -1;
                    break;
                } else {
                    kafka_data->log_str += "& action";
                    continue;
                }
            }

            if (!noah_config_read.Run(iter->second[i], kafka_data)) {
                flag_hit = -1;
                break;
            }
        }

        if (flag_hit == -1)
            continue;

        kafka_data->log_str += "=>:hit_result: " + iter->first;
        kafka_data->action_id = iter->first; //赋值action_id
        return true;
    }

    LOG(INFO) << kafka_data->log_str;
    return false;
}

bool UserQuery::FreshTriggerConfig(Redis* redis_user_trigger_config) {
    unordered_map<string, string> all_json;
    //获取noah配置
    redis_user_trigger_config->HGetAll("crm_noah_config_bak", &all_json);

    parse_noah_config(all_json);

    return true;
}

bool UserQuery::pretreatment(Json::Value all_config) {
    //string activity;
    //activity = all_config["activityId"].asString();
    if (all_config["status"].asString() != "true") {
        return false;
    }
    return true;
}

void UserQuery::parse_noah_config(const unordered_map<string, string>& all_json) {

    Json::Reader reader;
    lasso_config_map.clear();

    for (unordered_map<string, string>::const_iterator iter = all_json.begin(); iter != all_json.end(); ++iter) {
        vector<BaseConfig> lasso_config_set;
        Json::Value all_config, lasso_config, offline_config;
        reader.parse((iter->second).c_str(), all_config);

        if (!pretreatment(all_config)) {
            continue;
        }

        lasso_config = all_config["filters_list"];
        offline_config = all_config["jobArray"][0]["filters_list"];

        //cout << offline_config << endl;
        //cout << all_config << endl;

        //初始化圈选数据
        for (unsigned int i = 0; i < lasso_config.size(); ++i) {
            BaseConfig base_config;
            base_config.filter_id = lasso_config[i]["filter_id"].asString();
            base_config.option_id = lasso_config[i]["options"]["option_id"].asString();
            base_config.start = lasso_config[i]["options"]["start"].asString();
            base_config.end = lasso_config[i]["options"]["end"].asString();
            base_config.value_id = lasso_config[i]["values"]["value_id"].asString();
            if (base_config.value_id.find("LIST_MULTIPLE") != string::npos) {
                for (unsigned int j = 0; j < lasso_config[i]["values"]["list"].size(); ++j) {
                    base_config.values.push_back(lasso_config[i]["values"]["list"][j].asString());
                }
            } else if (base_config.value_id.find("VALUE_INPUT") != string::npos || base_config.value_id.find("value.month_card.INPUT") != string::npos) {
                for (unsigned int j = 0; j < lasso_config[i]["values"]["input"].size(); ++j) {
                    base_config.values.push_back(lasso_config[i]["values"]["input"][j].asString());
                }
            }

            lasso_config_set.push_back(base_config);
        }

        //初始化圈选数据
        for (unsigned int i = 0; i < offline_config.size(); ++i) {
            BaseConfig base_config;
            base_config.filter_id = offline_config[i]["filter_id"].asString();
            base_config.option_id = offline_config[i]["options"]["option_id"].asString();
            base_config.start = offline_config[i]["options"]["start"].asString();
            base_config.end = offline_config[i]["options"]["end"].asString();
            if (base_config.option_id == "")
                base_config.option_id = "noah_config";
            base_config.value_id = offline_config[i]["values"]["value_id"].asString();
            if (base_config.value_id.find("LIST_MULTIPLE") != string::npos) {
                for (unsigned int j = 0; j < offline_config[i]["values"]["list"].size(); ++j) {
                    base_config.values.push_back(offline_config[i]["values"]["list"][j].asString());
                }
            } else if (base_config.value_id.find("VALUE_INPUT") != string::npos) {
                for (unsigned int j = 0; j < offline_config[i]["values"]["input"].size(); ++j) {
                    base_config.values.push_back(offline_config[i]["values"]["input"][j].asString());
                }
            }

            lasso_config_set.push_back(base_config);
        }

        lasso_config_map.insert(pair<string, vector<BaseConfig>>(iter->first, lasso_config_set));
    }

    return;
}

void UserQuery::Detect() {
    while(true) {
        run_ = false;
        sleep(2);
        Redis redis_userid, redis_user_trigger_config;
        if(!InitRedis(&redis_userid, &redis_user_trigger_config)) {
            LOG(ERROR) << "redis init error";
            continue;
        }

        if(!FreshTriggerConfig(&redis_user_trigger_config)) {
            LOG(ERROR) << "init conf error";
            continue;
        }
        run_ = true;
        sleep(60);
    }
}

bool UserQuery::Init() {
    if(!LoadInitialRangeData()) {
        cout << "init file date error" << endl;
        return false;
    }
    std::thread observer(&UserQuery::Detect, this);
    observer.detach();

    return true;
}

bool UserQuery::LoadInitialRangeData() {
    for (size_t i = 0; i < time_range_file.size(); ++i) {
        unordered_map<string, vector<TimeRange>> base_vec;
        if (time_range_file[i] == "order.order") {
            if(!LoadRangeOriginConfig("./data/history_order.txt", &base_vec))
                return false;
        } else if (time_range_file[i] == "order.repair_order") {
            if(!LoadRangeOriginConfig("./data/repair_order.txt", &base_vec))
                return false;
        } else if (time_range_file[i] == "order.free_order") {
            if(!LoadRangeOriginConfig("./data/free_order.txt", &base_vec))
                return false;
        } else if (time_range_file[i] == "order.weekday_order") {
            if(!LoadRangeOriginConfig("./data/weekday_order.txt", &base_vec))
                return false;
        } else if (time_range_file[i] == "order.peak_order") {
            if(!LoadRangeOriginConfig("./data/peak_order.txt", &base_vec))
                return false;
        } else if (time_range_file[i] == "offline.bikeFailed") {
            if(!LoadRangeOriginConfig("./data/bike_failed.txt", &base_vec))
                return false;
        }
        
        time_range_origin.insert(make_pair(time_range_file[i], base_vec));
    }

    return true;
}

//获取用户uid,action
bool UserQuery::Parse_kafka_data(Redis* redis_userid, Redis* redis_user_trigger_config, string behaver_message, KafkaData* kafka_data) {

    Json::Reader reader;
    Json::Value user_json;

    if(behaver_message.find("userid\":\"") == string::npos && behaver_message.find("action\":\"") == string::npos) {
        return false;
    }

    string json_behaver_message = behaver_message.substr(behaver_message.find("body\":")+6, string::npos);
    json_behaver_message[json_behaver_message.size()-1] = '\0';
    reader.parse(json_behaver_message.c_str(), user_json);
    for (unsigned int i =0; i < user_json["content"].size(); ++i) {
        if (user_json["content"][i]["action"].asString() == "AppLaunch_Manner_00192") {
            kafka_data->action = "appstart";
        } else if (user_json["content"][i]["action"].asString() == "HomepageClick_ofo_00010" && user_json["content"][i]["params"]["more"]["click"].asString() == "StartButton") {
            kafka_data->action = "appscan";
        } else {
            continue;
        }

        //string user_id_md5 = behaver_message.substr(behaver_message.find("userid\":\"")+9, 32);
        string user_id_md5 = user_json["content"][i]["userid"].asString();
        if (user_id_md5.size() != 32) {
            continue;
        }

        redis_userid->HGet("user_info_"+user_id_md5, "userid", &(kafka_data->uid));
        if(kafka_data->uid.empty()) {
            continue;
        }

        redis_userid->HGet("user_info_"+user_id_md5, "telephone", &(kafka_data->tel));
        if(kafka_data->tel.empty()) {
            continue;
        }

        kafka_data->uid = "554345677";

        //获取用户离线数据
        string user_offline_data;
        redis_user_trigger_config->HGet("user_offline_data", "554345677", &user_offline_data);
        reader.parse(user_offline_data.c_str(), kafka_data->offline_data_json);

        if (kafka_data->offline_data_json["rv"]["1006"] != "") {
            kafka_data->offline_data_json["rv"]["1006"] =  distance_time_now(kafka_data->offline_data_json["rv"]["1006"].asString())/86400;
        }

        if (kafka_data->offline_data_json["rv"]["7"] != "") {
            kafka_data->offline_data_json["rv"]["7"] =  distance_time_now(kafka_data->offline_data_json["rv"]["7"].asString());
        }

        if (kafka_data->offline_data_json["rv"]["8"] != "") {
            kafka_data->offline_data_json["rv"]["8"] =  distance_time_now(kafka_data->offline_data_json["rv"]["8"].asString());
        }

        if (kafka_data->offline_data_json["rv"]["1008"] != "") {
            kafka_data->offline_data_json["rv"]["1008"] =  distance_time_now(kafka_data->offline_data_json["rv"]["1008"].asString());
        }

        if (kafka_data->offline_data_json["rv"]["12"] != "") {
            kafka_data->offline_data_json["rv"]["12"] =  ((-1)*distance_time_now(kafka_data->offline_data_json["rv"]["12"].asString() + " 00:00:00"))/86400;
        }

        return true;
    }

    return false;
}
