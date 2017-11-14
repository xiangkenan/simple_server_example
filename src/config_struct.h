#ifndef CONFIG_STRUCT_H_
#define CONFIG_STRUCT_H_

//noah 短信,push配置
class TelPushMsg {
    public:
        TelPushMsg():content(""), jump_url(""), type("") {}

        std::string content;
        std::string jump_url;
        std::string type;
        std::string money;
};

//noah 圈选用户配置
class BaseConfig {
    public:
        BaseConfig(): start(""), end(""), filter_id(""), option_id(""), value_id(""), map_field(""), type(""), count(0) {
            values.clear();
        }

        std::string start;
        std::string end;
        std::string filter_id;
        std::string option_id;
        std::string value_id;
        std::string map_field;
        std::string type;
        int count;
        std::vector<std::string> values;
};

//Noah所有配置
class NoahConfig {
    public:
        NoahConfig():limit(""), activity("") {
            base_config.clear();
            tel_push_msg.clear();
            tail_number.clear();
        }

        std::vector<BaseConfig> base_config;
        std::vector<TelPushMsg> tel_push_msg;
        std::vector<std::string> tail_number;
        std::string limit;
        std::string activity;
};

//kafka 数据
class KafkaData {
    public:
        KafkaData():user_behaviour_date(""), uid(""), action(""), tel(""), log_str(""), userprofile_city(""), register_day(""), order_num("") {
            action_id.clear();
            offline_data_json.clear();
        }
        std::string user_behaviour_date;
        std::string uid;
        std::string action;
        std::string tel;
        std::string log_str;
        std::string userprofile_city;
        std::string register_day;
        std::string order_num;
        std::vector<std::string> action_id;
        Json::Value offline_data_json;
};

class TimeRange {
    public:
        TimeRange():date(0), num(0) {}
        int date;
        int num;
};

//多线程加载文件类
class ParallelLoadConfig {
    public:
        ParallelLoadConfig ():field_name(""), file_name(""), time_range_origin(NULL) {}
        std::string field_name;
        std::string file_name;
        std::unordered_map<std::string, std::unordered_map<long, std::vector<TimeRange>>>* time_range_origin; 
        pthread_mutex_t mutex;
};

#endif
