#include "ofo_crm.h"
#include "kafka_consume.h"

using namespace std;

bool OfoCrm::Run() {
    try {
        //std::shared_ptr<kafka_consumer_client> kafka_consumer_client_ = std::make_shared<kafka_consumer_client>("192.168.30.236:9092", "userevents", "1", 0, *(int *)i);
        std::shared_ptr<kafka_consumer_client> kafka_consumer_client_ = std::make_shared<kafka_consumer_client>("192.168.30.236:9092", "userevents", "1", 0, 1);
        if (!kafka_consumer_client_->initClient()){
            fprintf(stderr, "kafka server initialize error\n");
        }else{
            LOG(WARNING) << "start kafka consumer";
            kafka_consumer_client_->consume(1000, &user_query);
        }
        LOG(WARNING) << "kafka consume exit";
    } catch (std::runtime_error &e) {
        printf("catch runtime error: %s\n", e.what());
        //LOG(ERROR) << "catch runtime error:" <<  e.what();
    } catch (...) {
        printf("catch unknown error");
        //LOG(ERROR) << "catch unknown error";
    }

    return NULL;
}
