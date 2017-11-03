#ifndef NOAH_CONFIG_H_
#define NOAH_CONFIG_H_

#include <iostream>
#include <string>
#include <glog/logging.h>
#include <time.h>
#include <jsoncpp/jsoncpp.h>
#include <stdlib.h>

#include "userquery.h"
#include "config_struct.h"

class NoahConfigRead {
    public:
        NoahConfigRead(std::unordered_map<std::string, std::unordered_map<long, std::vector<TimeRange>>>* time_range_origin);
        ~NoahConfigRead() {};
        bool Run(const BaseConfig& config, KafkaData* kafka_data);
    private:
        bool data_core_operate(const BaseConfig& config, int flag, KafkaData* kafka_data);
        bool write_log(const BaseConfig& config, bool flag, KafkaData* kafka_data);

        bool is_include(const BaseConfig& config, std::string user_msg); //包含,不包含类型
        bool is_confirm(const BaseConfig& config, std::string user_msg); //是,否类型
        bool is_time_range(const BaseConfig& config, std::string user_msg); //是否在时间范围内
        int is_time_range_value(const BaseConfig& config, KafkaData* kafka_data); //是否在时间范围内,并且满足条件
        bool is_big_small(const BaseConfig& config, int user_msg); //判断大小

        std::unordered_map<std::string, std::string> redis_field_map;
        std::unordered_map<std::string, int> type_map_operate;

        std::unordered_map<std::string, std::unordered_map<long, std::vector<TimeRange>>>* time_range_origin_;
};

#endif
