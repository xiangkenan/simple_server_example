#ifndef CONFIG_STRUCT_H_
#define CONFIG_STRUCT_H_

class BaseConfig {
    public:
        BaseConfig(): start(""), end(""), filter_id(""), option_id(""), value_id(""), map_field("") {
            values.clear();
        }

        std::string start;
        std::string end;
        std::string filter_id;
        std::string option_id;
        std::string value_id;
        std::string map_field;
        std::vector<std::string> values;
};

//kafka 数据
class KafkaData {
    public:
        KafkaData():uid(""), action(""), tel(""), log_str(""), action_id("") {
            offline_data_json.clear();
        }
        std::string uid;
        std::string action;
        std::string tel;
        std::string log_str;
        std::string action_id;
        Json::Value offline_data_json;
};

class TimeRange {
    public:
        TimeRange():date(""), num(0) {}
        std::string date;
        int num;
};

#endif
