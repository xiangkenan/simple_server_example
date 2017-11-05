#include "kafka_consume.h"
#include "ofo_crm.h"
#include "queue.h"

#define THREAD_COUNT 1
#define RATIO 1
#define THREAD_QUEUE THREAD_COUNT*RATIO

static void sigterm (int sig) {
    kafka_consumer_client::run_ = false;
}

void *run_kafka(void *run_kafka_fun) {
    RunKafka* run_kafka = (RunKafka*)run_kafka_fun;
    (run_kafka->ofo_crm_)->Run(run_kafka->num_, run_kafka->queue_kafka_, run_kafka->offset_);

    return NULL;
}

//消费queue
void *run_queue(void *run_queue_fun) {
    RunQueue* run_queue = (RunQueue*)run_queue_fun;
    while(kafka_consumer_client::run_) {
        while(!(((run_queue->ofo_crm_)->user_query).run_) || run_queue->queue_kafka_->empty()) {
            if (kafka_consumer_client::run_ == false) {
                return NULL;
            }
            usleep(1000);
        }
        string log_str;
        struct timeval start_time;
        gettimeofday(&start_time, NULL);
        string get_msg;
        run_queue->queue_kafka_->get_queue(get_msg);
        //处理kafka用户信息
        ((run_queue->ofo_crm_)->user_query).Run(get_msg, log_str);
        if (!log_str.empty()) {
            LOG(INFO) << log_str << write_ms_log(start_time, "cost:");
        }
    }

    return NULL;
}

int main(int argc, char **argv) {
    FLAGS_logbufsecs = 0;
    FLAGS_max_log_size = 2000;
    FLAGS_log_dir = "./log";
    google::InitGoogleLogging("user_behaviour");

    signal(SIGINT, sigterm);
    signal(SIGTERM, sigterm);
    signal(SIGKILL, sigterm);
    signal(SIGFPE, sigterm);

    pthread_t id[THREAD_COUNT];
    pthread_t id_queue[THREAD_QUEUE];

    OfoCrm ofo_crm;
    if(!ofo_crm.user_query.Init()) {
        return false;
    }

    //获取上次offset
    vector<long> offset;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        ifstream fin("conf/offset/offset_"+to_string(i)+".txt");
        if (!fin) {
            ofstream ofile;
            ofile.open("conf/offset/offset_"+to_string(i)+".txt");
            ofile << 0;
            ofile.close();
            offset.push_back(0);
            continue;
        }
        string line;
        while (getline(fin, line)) {
            if ((line = Trim(line)).empty()) {
                continue;
            }
            offset.push_back(atol(line.c_str()));
            break;
        }
        fin.close();
    }
    if (offset.size() != THREAD_COUNT) {
        cout << "load offset num is error!";
        return 0;
    }

    //初始化内部队列
    vector<QueueKafka*> queue_kafka_vec;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        QueueKafka* queue_kafka = new QueueKafka();
        queue_kafka_vec.push_back(queue_kafka);
    }

    //kafka获取
    for (int i = 0; i < THREAD_COUNT; ++i) {
        usleep(100);
        RunKafka run_kafka_fun(&ofo_crm, i, queue_kafka_vec[i], offset[i]);
        pthread_create(&id[i], NULL, run_kafka, (void *)&run_kafka_fun);
    }

    //内部队列消费
    for (int i = 0; i < THREAD_QUEUE; ++i) {
        usleep(100);
        RunQueue run_queue_fun(&ofo_crm, queue_kafka_vec[i%THREAD_COUNT]);
        pthread_create(&id_queue[i], NULL, run_queue, (void *)&run_queue_fun);
    }

    void *thread_result;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(id[i], &thread_result);
    }

    for (int i = 0; i < THREAD_QUEUE; ++i) {
        pthread_join(id_queue[i], &thread_result);
    }

    return 0;
}
