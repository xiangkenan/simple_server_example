#include "kafka_consume.h"
#include "ofo_crm.h"
#include "queue.h"

#define THREAD_COUNT 20

static void sigterm (int sig) {
    kafka_consumer_client::run_ = false;
}

void *run_kafka(void *run_kafka_fun) {
    RunKafka* run_kafka = (RunKafka*)run_kafka_fun;
    (run_kafka->ofo_crm_)->Run(run_kafka->num_, run_kafka->queue_kafka_);

    return NULL;
}

//消费queue
void *run_queue(void *run_queue_fun) {
    RunQueue* run_queue = (RunQueue*)run_queue_fun;
    while(1) {
        while(!run_queue->ofo_crm_->user_query.run_) {
            sleep(1);
        }

        cout << run_queue->queue_kafka_->get_queue() << endl;
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
    pthread_t id_queue[THREAD_COUNT];

    OfoCrm ofo_crm;
    if(!ofo_crm.user_query.Init()) {
        return false;
    }

    vector<QueueKafka*> queue_kafka_vec;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        QueueKafka* queue_kafka = new QueueKafka();
        queue_kafka_vec.push_back(queue_kafka);
    }

    for (int i = 0; i < THREAD_COUNT; ++i) {
        usleep(1);
        RunKafka run_kafka_fun(&ofo_crm, i, queue_kafka_vec[i]);
        pthread_create(&id[i], NULL, run_kafka, (void *)&run_kafka_fun);
    }

    for (int i = 0; i < THREAD_COUNT; ++i) {
        RunQueue run_queue_fun(&ofo_crm, queue_kafka_vec[i%20]);
        pthread_create(&id_queue[i], NULL, run_queue, (void *)&run_queue_fun);
    }

    void *thread_result;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(id[i], &thread_result);
    }

    return 0;
}
