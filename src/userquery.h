#ifndef USERQUERY_H_
#define USERQUERY_H_

#include <iostream>
#include <string>
#include <glog/logging.h>
#include <time.h>
#include <jsoncpp/jsoncpp.h>
#include <stdlib.h>
#include <murl.h>

#include "redis.h"

class BaseConfig {
    public:
        BaseConfig():filter_id(""), option_id(""), value_id(""), map_field("") {
            values.clear();
        }

        std::string filter_id;
        std::string option_id;
        std::string value_id;
        std::string map_field;
        std::vector<std::string> values;
};

class UserQuery {
    public:
        UserQuery();
        ~UserQuery() {};
        bool Init();
        bool Run(std::string behaver_message);
    private:
        bool HandleProcess();
        bool Parse(std::string behaver_message);
        void parse_noah_config();
        bool FreshTriggerConfig();
        bool data_core_operate(const BaseConfig& config, Json::Value user_msg_json, int flag);

        bool get_url(const char *url, char *buf, int size, long timeout_ms, const char *cookie, const char *token);
        bool write_log(std::string msg, bool flag);
        bool is_include(const BaseConfig& config, std::string user_msg); //包含,不包含类型
        bool is_confirm(const BaseConfig& config, std::string user_msg); //是,否类型
        bool is_time_range(const BaseConfig& config, std::string user_msg); //是否在时间范围内
        bool is_time_range_value(const BaseConfig& config, std::string user_msg); //是否在时间范围内,并且满足条件
        bool is_range_value(const BaseConfig& config, std::string user_msg); // 是否大于，小于，范围

        bool SendMessage();
        bool pretreatment(Json::Value all_config);
        Json::Value get_url_json(char* buf);

        void Split(const std::string& s,
                const std::string& delim,
                std::vector<std::string>* ret) {
            size_t last = 0;
            size_t index = s.find_first_of(delim, last);
            while (index != std::string::npos) {
                ret->push_back(s.substr(last, index - last));
                last = index + 1;
                index = s.find_first_of(delim, last);
            }
            if (index - last > 0) {
                ret->push_back(s.substr(last, index - last));
            }
        }

        Redis *redis_userid;
        Redis *redis_user_trigger_config;
        Redis *redis_user_offline_data;
        Redis *redis_user_realtime_data;

        std::string uid; //用户uid
        std::string action;//用户开锁行为
        std::string activity; //活动
        std::string tel; //电话

        int pre_trigger_config_min;
        int cur_trigger_config_min;

        Json::Reader reader;
        Json::FastWriter writer;
        std::string base_value; //基数 资源
        std::string base_choose_value; //基数 选择

        std::map<std::string, std::string> all_json;

        std::map<std::string, std::vector<BaseConfig>> lasso_config_map;
        std::map<std::string, std::vector<BaseConfig>> offline_config_map;
        std::map<std::string, std::string> redis_field_map;

        std::string log_str;
};

#endif
