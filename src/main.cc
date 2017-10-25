#include "kafka_consume.h"

#define THREAD_COUNT 19

static void sigterm (int sig) {
    kafka_consumer_client::run_ = false;
}

void *run_kafka(void *i) {
    try {
    //std::shared_ptr<kafka_consumer_client> kafka_consumer_client_ = std::make_shared<kafka_consumer_client>(brokers, topics, group, 0, partition);
    std::shared_ptr<kafka_consumer_client> kafka_consumer_client_ = std::make_shared<kafka_consumer_client>("192.168.30.236:9092", "userevents", "1", 0, *(int *)i);
    if (!kafka_consumer_client_->initClient()){
        fprintf(stderr, "kafka server initialize error\n");
    }else{
        printf("start kafka consumer\n");
        kafka_consumer_client_->consume(1000);
    }
    fprintf(stderr, "kafka consume exit! \n");
    } catch (std::runtime_error &e) {
        printf("catch runtime error: %s\n", e.what());
        //LOG(ERROR) << "catch runtime error:" <<  e.what();
    } catch (...) {
        printf("catch unknown error");
        //LOG(ERROR) << "catch unknown error";
    }
    
    return NULL;
}

int main(int argc, char **argv){
    FLAGS_logbufsecs = 2000;
    FLAGS_max_log_size = 2000;
    FLAGS_log_dir = "./log";
    google::InitGoogleLogging("user_behaviour");
    int opt;
    //int32_t partition;
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
            //case 'p':
            //    partition = atoi(optarg);
            //    break;
            default:
                break;
        }
    }

    signal(SIGINT, sigterm);
    signal(SIGTERM, sigterm);
    signal(SIGKILL, sigterm);
    signal(SIGFPE, sigterm);

    pthread_t id[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_create(&id[i], NULL, run_kafka, (void *)&i);
        cout << id[i] << endl;
    }

    void *thread_result;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(id[i], &thread_result);
        cout << "线程失败:" << id[i] << ";" <<  endl;
    }

    return 0;
}
