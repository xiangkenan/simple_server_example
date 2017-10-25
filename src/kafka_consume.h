#ifndef KAFKA_CONSUME_H_
#define KAFKA_CONSUME_H_

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <getopt.h>
#include <csignal>
#include <iostream>
#include <glog/logging.h>

#include "rdkafkacpp.h"
#include "userquery.h"

using namespace std;

class kafka_consumer_client{
    public:
        kafka_consumer_client(const std::string& brokers, const std::string& topics, const std::string groupid, int64_t offset, int32_t partition);
        kafka_consumer_client();
        virtual ~kafka_consumer_client();

        bool initClient();
        bool consume(int timeout_ms, UserQuery *user_query);
        void finalize();
        static bool run_;
    private:
        void consumer(RdKafka::Message *msg, void *opt, UserQuery *user_query);

        std::string brokers_;
        std::string topics_;
        std::string groupid_;

        int64_t last_offset_ = 0;
        RdKafka::Consumer *kafka_consumer_ = nullptr;   
        RdKafka::Topic *topic_          = nullptr;
        int64_t offset_          = RdKafka::Topic::OFFSET_BEGINNING;
        int32_t partition_ = 0;
        //UserQuery *user_query;
};

#endif
