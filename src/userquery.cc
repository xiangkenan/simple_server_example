#include "userquery.h"

using namespace std;

extern void * parallel_load_config(void*);

UserQuery::UserQuery() {
    run_ = false;
    last_update_increment_date = "0";
    dump_file_every_date = "0";

    time_range_file.insert(make_pair("order.repair_order", "repair_order"));
    time_range_file.insert(make_pair("order.order", "history_order"));
    time_range_file.insert(make_pair("order.free_order", "free_order"));
    time_range_file.insert(make_pair("order.weekday_order", "weekday_order"));
    time_range_file.insert(make_pair("order.peak_order", "peak_order"));
    time_range_file.insert(make_pair("offline.bikeFailed", "bike_failed"));
}

bool UserQuery::GetQconfRedis(Redis* redis_handle, const string& config_qconf) {
    Json::Reader reader;
    Json::Value result;
    char value[QCONF_CONF_BUF_MAX_LEN];
    int ret = qconf_get_conf(config_qconf.c_str(), value, sizeof(value), NULL);
    if (ret != QCONF_OK) {
        LOG(ERROR) << "Qconf connect failed!";
        return false;
    }
    reader.parse(value, result);
    if (!redis_handle->Connect(result["host"].asString().c_str(), result["port"].asInt(), result["password"].asString().c_str())) {
        LOG(WARNING) << "connect user_trigger_config redis failed" ;
        return false;
    }

    return true;
}

bool UserQuery::InitRedis(Redis* redis_user_trigger_config) { Json::Reader reader;
    if (!GetQconfRedis(redis_user_trigger_config, "/Dba/redis/prc/online/ofo_crm/connection")) {
        return false;
    }

    return true;
}

bool UserQuery::Run(const string& behaver_message, string& log_str) {
    KafkaData kafka_data;

    Redis redis_user_trigger_config;

    if(!InitRedis(&redis_user_trigger_config)) {
        return false;
    }

    if(!Parse_kafka_data(&redis_user_trigger_config, behaver_message, &kafka_data)) {
        log_str = kafka_data.log_str;
        return false;
    }


    if (!HandleProcess(&redis_user_trigger_config, &kafka_data)) {
        log_str = kafka_data.log_str;
        return false;
    }

    //发短信
    SendMessage(&kafka_data, &redis_user_trigger_config);

    log_str = kafka_data.log_str;
    return true;
}

bool UserQuery::SendMessage(KafkaData* kafka_data, Redis* redis_user_trigger_config) {
    int ret;
    char buf[1024];
    kafka_data->log_str += "\tMSG PUSH:\t";

    for (size_t i = 0; i < kafka_data->action_id.size(); ++i) {
        int limit;
        if (lasso_config_map[kafka_data->action_id[i]].limit == "") {
            limit = 0;
        } else {
            limit = atoi(lasso_config_map[kafka_data->action_id[i]].limit.c_str());
        }

        string url = "http://192.168.3.127:9000/riskmgt/antispam?param=freq&op=query&bid=10038&kv1=activity," + kafka_data->action_id[i];
        if ((ret = murl_get_url(url.c_str(), buf, 10240, 100, NULL, NULL, NULL)) != MURLE_OK) {
            LOG(WARNING) << "riskmgt interface error";
            continue;
        }

        Json::Value result;
        result = get_url_json(buf);
        int limit_num_cur = atoi(result["data"][0]["frequence"][0]["value"].asString().c_str());
        if (limit < limit_num_cur && limit != 0) {
            kafka_data->log_str += "(=>:hit freq activity:" + kafka_data->action_id[i] + " full <"+to_string(limit_num_cur)+">)";
            continue;
        } else {
            string url = "http://192.168.3.127:9000/riskmgt/antispam?param=freq&bid=10038&kv1=activity," + kafka_data->action_id[i];
            if ((ret = murl_get_url(url.c_str(), buf, 10240, 100, NULL, NULL, NULL)) != MURLE_OK) {
                LOG(WARNING) << "riskmgt interface error";
                continue;
            }
        }

        //每个活动 每人只发送一次短信或push
        url = "http://192.168.3.127:9000/riskmgt/antispam?param=freq&bid=10038&kv1=user_id," + kafka_data->action_id[i] + "_" + kafka_data->uid;
        if ((ret = murl_get_url(url.c_str(), buf, 10240, 100, NULL, NULL, NULL)) != MURLE_OK) {
            LOG(WARNING) << "riskmgt interface error";
            continue;
        }
        result = get_url_json(buf);
        string code = result["code"].asString();
        if (code != "200") {
            kafka_data->log_str += "(=>:hit freq userid full)";
            return false;
        }

        //活动计数
        redis_user_trigger_config->HIncrby("crm_activity_num", kafka_data->action_id[i], 1);

        //开发短信和push
        vector<TelPushMsg> tel_push_msg = lasso_config_map[kafka_data->action_id[i]].tel_push_msg;
        for (size_t j = 0; j < tel_push_msg.size(); ++j) {
            memset(buf, 0, sizeof(buf));
            //个性化信息转换
            replace_all_distinct(tel_push_msg[j].content, "{register.city}", kafka_data->userprofile_city);
            replace_all_distinct(tel_push_msg[j].content,"{register.days}", kafka_data->register_day);
            replace_all_distinct(tel_push_msg[j].content,"{accumulate.orders}", kafka_data->order_num);

            if (tel_push_msg[j].type == "message") {
                string url = "192.168.2.123/now";
                string args = "to=" + kafka_data->tel + "&templateId=crm_notify&context="+tel_push_msg[j].content;
                string token = "x-ofo-token:eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxODYzNjY0Nzk2MiIsIm5hbWUiOiLmt7HlnLMifQ.EjXJEjEWGKcsI896Mx6BUCbtnlq_gcnQ2NjpQaZSLkE";
                ////*************
                //if ((ret = murl_get_url(url.c_str(), buf, 10240, 0, NULL, token.c_str(), args.c_str())) != MURLE_OK) {
                //    kafka_data->log_str += "{send message error!!}";
                //}
                ////*************
                kafka_data->log_str += "●●{send message to " + kafka_data->tel + ":" + 
                    tel_push_msg[j].content + "}";
            }
            if (tel_push_msg[j].type == "push") {

                //连接push redis
                //Redis redis_push;
                //if (!GetQconfRedis(&redis_push, "/Dba/redis/prc/online/ofo_push/connection")) {
                //    return false;
                //}

                //int id = (atoi(kafka_data->uid.c_str()))%1+1;
                //Json::Value push_json;
                //Json::FastWriter writer;
                //push_json["cid"] = kafka_data->uid+ kafka_data->tel;
                //push_json["content"] = tel_push_msg[j].content;
                //push_json["jump_url"] = tel_push_msg[j].jump_url;
                //string push_json_str = writer.write(push_json);
                //redis_push->Lpush("push:"+to_string(id), push_json_str);
                
                kafka_data->log_str += "●●{send push to "+kafka_data->uid+":" + 
                    tel_push_msg[j].content + "jump_url:" + tel_push_msg[j].jump_url+ "}";
            }
        }
    }

    return true;
}

//1:满足配置 2:不满足配置 -1:出错
bool UserQuery::HandleProcess(Redis* redis_user_trigger_config, KafkaData *kafka_data) {
    kafka_data->log_str += "date:" + kafka_data->user_behaviour_date + " action:"+kafka_data->action+" userid:" + kafka_data->uid + " ";
    //初始化配置操作类
    NoahConfigRead noah_config_read(&time_range_origin);
    for (unordered_map<std::string, NoahConfig>::iterator iter = lasso_config_map.begin();
            iter != lasso_config_map.end();
            iter++) {
        int flag_hit = 0;
        kafka_data->log_str += "(●|●)activity:" + iter->first + "=>";
        for(unsigned int i = 0; i < iter->second.base_config.size(); ++i) {
            //测试
            char char_tail_uid = (kafka_data->uid)[kafka_data->uid.length()-1];
            stringstream stream;
            stream << char_tail_uid;
            string tail_uid = stream.str();

            vector<string>::iterator ret;
            ret = find(iter->second.tail_number.begin(), iter->second.tail_number.end(), tail_uid);
            if (ret == iter->second.tail_number.end()) {
                kafka_data->log_str += "&no tailuid";
                flag_hit = -1;
                break;
            }
            BaseConfig cc = iter->second.base_config[i];
            //判断app触发条件是否满足
            if (cc.filter_id == "realtime.app.action") {
                if ((cc.option_id == "app.action.appon" && kafka_data->action == "appscan") || 
                        (cc.option_id == "app.action.scan" && kafka_data->action == "appstart")) {
                    kafka_data->log_str += "&touch failed";
                    flag_hit = -1;
                    break;
                } else {
                    kafka_data->log_str += "&touch success";
                    continue;
                }
            }

            if (!noah_config_read.Run(iter->second.base_config[i], kafka_data)) {
                flag_hit = -1;
                break;
            }
        }

        if (flag_hit == -1)
            continue;

        kafka_data->log_str += "=>:regular_pass";
        kafka_data->action_id.push_back(iter->first); //赋值action_id
        continue;
    }

    if (lasso_config_map.empty()) {
        kafka_data->log_str += ">>>>>:config empty";
    }

    if (kafka_data->action_id.size() > 0) {
        return true;
    } else {
        return false;
    }
}

bool UserQuery::FreshTriggerConfig(Redis* redis_user_trigger_config) {
    unordered_map<string, string> all_json;
    //获取noah配置
    redis_user_trigger_config->HGetAll("crm_noah_config", &all_json);

    parse_noah_config(all_json);

    return true;
}

bool UserQuery::pretreatment(Json::Value all_config, NoahConfig* noah_config) {
    if (all_config["status"].asString() != "true") {
        return false;
    }

    string hour_min_sec = get_now_hour_min_sec();
    //判断活动时间是否满足
    string date_cur = get_now_date();
    date_cur = date_cur.substr(0, 4) + "-" + date_cur.substr(4, 2) + "-" + date_cur.substr(6, 2) + " " + hour_min_sec;
    if (date_cur < all_config["startTime"].asString() || date_cur > all_config["endTime"].asString())
        return false;

    Json::Value msg_push_config = all_config["jobArray"][0]["touchUser"];

    for (unsigned int i = 0; i < msg_push_config.size(); ++i) {
        if(hour_min_sec > msg_push_config[i]["end"].asString() || hour_min_sec < msg_push_config[i]["start"].asString()) {
            continue;
        }
        TelPushMsg tel_push_msg;
        tel_push_msg.content = msg_push_config[i]["content"].asString();
        tel_push_msg.type = msg_push_config[i]["type"].asString();
        if (tel_push_msg.type == "push") {
            tel_push_msg.jump_url = msg_push_config[i]["jumpUrl"].asString();
        }
        noah_config->tel_push_msg.push_back(tel_push_msg);
    }

    //不存在发送短信，push， 不在时间段内则配置无效
    if (noah_config->tel_push_msg.size() <= 0) {
        return false;
    }

    Json::Value tail_number = all_config["jobArray"][0]["jobUid"];
    for (unsigned int i = 0; i < tail_number.size(); ++i) {
        noah_config->tail_number.push_back(tail_number[i].asString());
    }

    noah_config->activity = all_config["activityId"].asString();
    noah_config->limit = all_config["limit"].asString();

    return true;
}

void UserQuery::parse_noah_config(const unordered_map<string, string>& all_json) {
    Json::Reader reader;
    lasso_config_map.clear();

    for (unordered_map<string, string>::const_iterator iter = all_json.begin(); iter != all_json.end(); ++iter) {
        NoahConfig noah_config;

        Json::Value all_config, lasso_config, offline_config;
        reader.parse((iter->second).c_str(), all_config);

        if (!pretreatment(all_config, &noah_config)) {
            continue;
        }

        cout << all_config << endl;
        lasso_config = all_config["filters_list"];
        offline_config = all_config["jobArray"][0]["filters_list"];


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

            noah_config.base_config.push_back(base_config);
        }

        //初始化圈选数据
        for (unsigned int i = 0; i < offline_config.size(); ++i) {
            BaseConfig base_config;
            base_config.filter_id = offline_config[i]["filter_id"].asString();
            base_config.option_id = offline_config[i]["options"]["option_id"].asString();
            if (base_config.option_id.find("NEAR_TIMES") != string::npos) {
                base_config.count = atoi(offline_config[i]["options"]["count"].asString().c_str());
                base_config.type = offline_config[i]["options"]["type"].asString();
            }
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

            noah_config.base_config.push_back(base_config);
        }

        lasso_config_map.insert(pair<string, NoahConfig>(iter->first, noah_config));
    }

    return;
}

void UserQuery::Detect() {
    while(true) {
        run_ = false;
        sleep(2);
        InitConf("./conf/conf.txt");
        Redis redis_user_trigger_config;
        LOG(WARNING) << "start update config every min";
        //初始化redis
        if(!InitRedis(&redis_user_trigger_config)) {
            LOG(ERROR) << "redis init error";
            continue;
        }

        //更新noah配置
        if(!FreshTriggerConfig(&redis_user_trigger_config)) {
            LOG(ERROR) << "init conf error";
            continue;
        }

        //每天增量更新
        if (!UpdateDayIncrement()) {
            LOG(ERROR) << "every day update increment failed!";
            continue;
        }

        run_ = true;

        //dump file
        if (!DumpDayFile()) {
            LOG(ERROR) << "dump file failed!";
        }
        
        sleep(60);
    }
}

bool UserQuery::DumpDayFile() {
    string date = get_now_date();
    if (switch_dump != "yes") {
        if (get_now_hour() != dump_hour_time)
            return true;


        if (date == dump_file_every_date) {
            return true;
        }

        dump_file_every_date = date;
    }
    LOG(WARNING) << "start dump file :" << date;

    string dump_path = "mv  ./data/dump/" + date + " ./data/dump/" + date + "." + get_now_hour_min_sec();
    system(dump_path.c_str());
    dump_path = "mkdir -p ./data/dump/" + date;
    system(dump_path.c_str());
    for (unordered_map<std::string, unordered_map<long, vector<TimeRange>>>::iterator iter = time_range_origin.begin();
            iter != time_range_origin.end(); iter++) {
        if(!DumpFile("./data/dump/" + date + "/" + time_range_file[iter->first] + ".txt", iter->second)) {
            LOG(WARNING) << "dump file :" << iter->first << " failed!";
            continue;
        }
    }
    LOG(WARNING) << "dump all file success" << date;

    return true;
}

bool UserQuery::UpdateDayIncrement() {
    string date = get_now_date();
    if (switch_update != "yes") {
        if (get_now_hour() != update_increment_hour_time)
            return true;

        if (date == last_update_increment_date) {
            return true;
        }

        last_update_increment_date = date;
    }

    LOG(WARNING) << "start update increment user data" << date;

    for (unordered_map<string, string>::iterator iter = time_range_file.begin();
            iter != time_range_file.end(); iter++) {

        if (!LoadRangeOriginConfig("./data/"+iter->second + "_" + date +".txt", &time_range_origin[iter->first])) {
            continue;
        }
    }
    LOG(WARNING) << "update all increment user data" << date;

    return true;
}

bool UserQuery::InitConf(string conf_file) {
    ifstream fin(conf_file);
    if (!fin) {
        LOG(ERROR) << "load "<< conf_file << " failed!!";
        return false;
    }

    LOG(INFO) << "start loading file:" << conf_file;
    string line;
    while (getline(fin, line)) {
        if ((line = Trim(line)).empty()) {
            continue;
        }
        vector<string> line_vec;
        Split(line, "=", &line_vec);
        if (line_vec.size() != 2)
            return false;
        if (line_vec[0] == "update_increment_hour_time") {
            update_increment_hour_time = line_vec[1];
        } else if (line_vec[0] == "dump_time_hour") {
            dump_hour_time = line_vec[1];
        } else if (line_vec[0] == "switch_update") {
            switch_update = line_vec[1];
        } else if (line_vec[0] == "switch_dump") {
            switch_dump = line_vec[1];
        } else {
            LOG(ERROR) << "unknown field in conf file!!";
            return false;
        }
    }

    return true;
}

bool UserQuery::Init() {
    int ret = qconf_init();
    if (ret != QCONF_OK) {
        LOG(ERROR) << "Failed to init qconf!";
        return false;
    }

    if(!LoadInitialRangeData()) {
        LOG(ERROR) << "init file date error" << endl;
        return false;
    }
    std::thread observer(&UserQuery::Detect, this);
    observer.detach();

    return true;
}

bool UserQuery::LoadInitialRangeData() {
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    pthread_t id[time_range_file.size()];
    ofstream ofile;
    ofile.open("./conf/parallel_load_config.txt");
    ofile.close();

    int i = -1;
    for (unordered_map<string, string>::iterator iter = time_range_file.begin();
            iter != time_range_file.end(); iter++) {
        ++i;
        ParallelLoadConfig* parallel_conf = new ParallelLoadConfig();
        parallel_conf->field_name = iter->first;
        parallel_conf->file_name = iter->second;
        parallel_conf->time_range_origin = &time_range_origin;
        parallel_conf->mutex = mutex;
        cout << "内部地址:" << parallel_conf->time_range_origin << endl;
        int ret = pthread_create(&id[i], NULL, parallel_load_config, (void*)parallel_conf);
        if (ret != 0) {
            LOG(ERROR) << "parallel load conf failed, pthread create: " << strerror(errno);
            return false;
        }
    }

    void *thread_result;
    for (size_t i = 0; i < time_range_file.size(); ++i) {
        pthread_join(id[i], &thread_result);
    }

    if (time_range_file.size() <= 0) {
        LOG(ERROR) << "parallel load conf failed!!";
        return false;
    }

    //查看文件加载是否失败
    ifstream fin("./conf/parallel_load_config.txt");
    if (!fin) {
        LOG(ERROR) << "no load file!";
        return false;
    }

    string line;
    while (getline(fin, line)) {
        if ((line = Trim(line)).empty()) {
            continue;
        }
        if (line.find("false") != string::npos) {
            LOG(ERROR) << "load order file failed!!";
            return false;
        }
    }

    return true;
}

//获取用户uid,action
bool UserQuery::Parse_kafka_data(Redis* redis_user_trigger_config, string behaver_message, KafkaData* kafka_data) {
    Json::Reader reader;
    Json::Value user_json;

    if(behaver_message.find("userid\":\"") == string::npos && behaver_message.find("action\":\"") == string::npos) {
        return false;
    }

    kafka_data->user_behaviour_date =  behaver_message.substr(behaver_message.find("INFO ")+5, 19);
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

        string user_id_md5 = user_json["content"][i]["userid"].asString();
        if (user_id_md5.size() != 32) {
            continue;
        }

        //连接解密userid redis
        Redis redis_userid;
        if (!redis_userid.Connect("192.168.9.242", 3000, "MKL7cOEehQf8aoIBtHxs")) {
            LOG(WARNING) << "connect userid redis failed" ;
            return false;
        }

        redis_userid.HGet("user_info_"+user_id_md5, "userid", &(kafka_data->uid));
        if(kafka_data->uid.empty()) {
            continue;
        }

        redis_userid.HGet("user_info_"+user_id_md5, "telephone", &(kafka_data->tel));
        if(kafka_data->tel.empty()) {
            continue;
        }

        //白名单过滤
        //string white_user;
        //redis_user_trigger_config->HGet("crm_write_list", kafka_data->tel, &white_user);
        //if (white_user != "crm_write") {
        //    return false;
        //}

        //测试
        //kafka_data->uid = "554345677";

        //获取用户离线数据
        string user_offline_data;
        redis_user_trigger_config->Get("ofo:user_lib:"+kafka_data->uid, &user_offline_data);

        reader.parse(user_offline_data.c_str(), kafka_data->offline_data_json);

        string json_1006 = kafka_data->offline_data_json["rv"]["1006"].asString();

        if (kafka_data->offline_data_json["rv"]["1006"] != "") {
            kafka_data->offline_data_json["rv"]["1006"]  =  distance_time_now(kafka_data->offline_data_json["rv"]["1006"].asString())/86400;
        }

        if (kafka_data->offline_data_json["rv"]["7"] != "") {
            //注册时间先不加一天
            kafka_data->offline_data_json["rv"]["7"] =  distance_time_now(kafka_data->offline_data_json["rv"]["7"].asString());
            kafka_data->register_day = to_string(atoi(kafka_data->offline_data_json["rv"]["7"].asString().c_str())/86400);
            //注册天数 （10000）
            kafka_data->offline_data_json["rv"]["10000"] = to_string(atoi(kafka_data->offline_data_json["rv"]["7"].asString().c_str())/86400);
        }

        if (kafka_data->offline_data_json["rv"]["11"] != "") {
            kafka_data->userprofile_city = city.city_map[kafka_data->offline_data_json["rv"]["11"].asString()];
        }

        if (kafka_data->offline_data_json["rv"]["8"] != "") {
            //认证时间先不加一天
            kafka_data->offline_data_json["rv"]["8"] =  distance_time_now(kafka_data->offline_data_json["rv"]["8"].asString());
        }

        if (kafka_data->offline_data_json["rv"]["1008"] != "") {
            //首单时间先不加一天
            kafka_data->offline_data_json["rv"]["1008"] =  distance_time_now(kafka_data->offline_data_json["rv"]["1008"].asString());
        }

        if (kafka_data->offline_data_json["rv"]["1001"] != "") {
            kafka_data->order_num =  kafka_data->offline_data_json["rv"]["1001"].asString();
        }

        if (kafka_data->offline_data_json["rv"]["12"] != "") {
            kafka_data->offline_data_json["rv"]["12"] =  ((-1)*distance_time_now(kafka_data->offline_data_json["rv"]["12"].asString() + " 00:00:00"))/86400 + 1;
        }

        return true;
    }

    return false;
}
