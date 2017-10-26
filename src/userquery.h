#ifndef USERQUERY_H_
#define USERQUERY_H_

#include <iostream>
#include <string>
#include <glog/logging.h>
#include <time.h>
#include <jsoncpp/jsoncpp.h>
#include <stdlib.h>
#include <murl.h>
#include <thread>

#include "redis.h"
#include "string_tools.h"
#include "noah_config.h"
#include "config_struct.h"

class UserQuery {
    public:
        UserQuery();
        ~UserQuery() {};
        bool Init();
        bool Run(std::string behaver_message);
        bool run_;
    private:
        bool InitRedis(Redis* redis_userid, Redis* redis_user_trigger_config);
        bool HandleProcess(Redis* redis_userid, Redis* redis_user_trigger_config, KafkaData* kafka_data);
        bool Parse_kafka_data(Redis* redis_userid, Redis* redis_user_trigger_config, std::string behaver_message, KafkaData* kafka_data);
        void parse_noah_config();
        bool FreshTriggerConfig(Redis* redis_user_trigger_config);
        bool SendMessage(KafkaData* kafka_data);
        void Detect();

        bool pretreatment(Json::Value all_config);

        bool data_core_operate(const BaseConfig& config, int flag, KafkaData* kafka_data);
        bool write_log(std::string msg, bool flag, KafkaData* kafka_data);

        bool is_include(const BaseConfig& config, std::string user_msg); //包含,不包含类型
        bool is_confirm(const BaseConfig& config, std::string user_msg); //是,否类型
        bool is_time_range(const BaseConfig& config, std::string user_msg); //是否在时间范围内
        bool is_time_range_value(const BaseConfig& config, std::string user_msg); //是否在时间范围内,并且满足条件
        bool is_range_value(const BaseConfig& config, std::string user_msg); // 是否大于，小于，范围
        bool is_satisfied_value(const BaseConfig& config, std::string user_msg); //是否满足条件  app行为

        std::map<std::string, std::string> all_json; //离线noah 配置

 //     std::map<std::string, std::string> realtime_data; //实时redis数据

        std::map<std::string, std::vector<BaseConfig>> lasso_config_map;
};

#endif
