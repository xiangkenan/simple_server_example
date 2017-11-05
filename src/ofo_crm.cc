#include "ofo_crm.h"
#include "kafka_consume.h"

using namespace std;

bool OfoCrm::Run(int i, QueueKafka* queue_kafka, long offset) {
    try {
        std::shared_ptr<kafka_consumer_client> kafka_consumer_client_ = std::make_shared<kafka_consumer_client>("192.168.30.236:9092", "userevents", "crm_noah", offset, i);
        //std::shared_ptr<kafka_consumer_client> kafka_consumer_client_ = std::make_shared<kafka_consumer_client>("10.6.26.147:9092", "crm-test", "crm_noah", offset, i);
        if (!kafka_consumer_client_->initClient(queue_kafka)){
            fprintf(stderr, "kafka server initialize error\n");
        }else{
          LOG(WARNING) << "start kafka consumer";
          kafka_consumer_client_->consume(1000, &user_query, i);
        }
        LOG(WARNING) << "kafka consume exit";
    } catch (std::runtime_error &e) {
        printf("catch runtime error: %s\n", e.what());
    } catch (...) {
        printf("catch unknown error");
    }

    return NULL;
}

bool OfoCrm::run_operate(QueueKafka* queue_kafka) {
    //消费queue
    while(kafka_consumer_client::run_) {
        while(!(user_query.run_) || queue_kafka->empty()) {
            if (kafka_consumer_client::run_ == false) {
                return NULL;
            }
            usleep(1000);
        }
        string log_str;
        struct timeval start_time;
        gettimeofday(&start_time, NULL);
        string get_msg;
        queue_kafka->get_queue(get_msg);
        if (get_msg == "empty")
            continue;
        //处理kafka用户信息
        user_query.Run(get_msg, log_str);
        if (!log_str.empty()) {
            LOG(INFO) << log_str << write_ms_log(start_time, "cost:");
        }
    }
    return NULL;
}
