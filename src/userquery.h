#ifndef USERQUERY_H_
#define USERQUERY_H_

#include <iostream>
#include <string>
#include <glog/logging.h>
#include <time.h>
#include<jsoncpp/jsoncpp.h>

#include "redis.h"

class base_config {
    base_config():filter_id("") {}

    std::string filter_id;
    std::vector<std::string> optiion_id;
    std::vector<std::string> values;


};

class UserQuery {
    public:
        UserQuery();
        ~UserQuery() {};
        bool Init();
        bool Run(std::string behaver_message);
    private:
        bool Parse(std::string behaver_message);
        void parse_noah_config();
        bool FreshTriggerConfig();
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
        std::string uid;

        int pre_trigger_config_min;
        int cur_trigger_config_min;

        std::string base_value; //基数 资源
        std::string base_choose_value; //基数 选择
};

#endif
