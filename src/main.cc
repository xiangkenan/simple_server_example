#include "kafka_consume.h"

static void sigterm (int sig) {
    kafka_consumer_client::run_ = false;
}

int main(int argc, char **argv){
    //FLAGS_logbufsecs = 0;
    FLAGS_log_dir = "./log";
    google::InitGoogleLogging("user_behaviour");
    int opt;
    int32_t partition;
    std::string topics;
    std::string brokers = "localhost:9092";
    std::string group = "1";
    std::string log_path = "log_path";

    while ((opt = getopt(argc, argv, "g:b:p:t:qd:eX:As:DO")) != -1){
        switch (opt) {
            case 'b':
                brokers = optarg;
                break;
            case 'g':
                group = optarg;
                break;
            case 't':
                topics = optarg;
                break;
            case 'p':
                partition = atoi(optarg);
                break;
            default:
                break;
        }
    }

    signal(SIGINT, sigterm);
    signal(SIGTERM, sigterm);
    signal(SIGKILL, sigterm);
    signal(SIGFPE, sigterm);

    try {
    std::shared_ptr<kafka_consumer_client> kafka_consumer_client_ = std::make_shared<kafka_consumer_client>(brokers, topics, group, 0, partition);
    if (!kafka_consumer_client_->initClient()){
        fprintf(stderr, "kafka server initialize error\n");
    }else{
        printf("start kafka consumer\n");
        kafka_consumer_client_->consume(1000);
    }

    fprintf(stderr, "kafka consume exit! \n");
    } catch (std::runtime_error &e) {
        LOG(ERROR) << "catch runtime error:" <<  e.what();
    } catch (...) {
        LOG(ERROR) << "catch unknown error";
    }

    return 0;
}
