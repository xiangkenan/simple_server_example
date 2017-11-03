#include "kafka_consume.h"
#include "ofo_crm.h"

#define THREAD_COUNT 20

static void sigterm (int sig) {
    kafka_consumer_client::run_ = false;
}

void *run_kafka(void *run_kafka_fun) {
    RunKafka* run_kafka = (RunKafka*)run_kafka_fun;
    (run_kafka->ofo_crm_)->Run(run_kafka->num_);

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

    OfoCrm ofo_crm;
    if(!ofo_crm.user_query.Init()) {
        return false;
    }

    for (int i = 0; i < THREAD_COUNT; ++i) {
        RunKafka run_kafka_fun(&ofo_crm, i);
        pthread_create(&id[i], NULL, run_kafka, (void *)&run_kafka_fun);
    }

    void *thread_result;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(id[i], &thread_result);
    }

    return 0;
}
