#include "queue.h"

using namespace std;

void QueueKafka::get_queue(string& get_msg) {
    pthread_mutex_lock(&mutex);
    if(mq_.empty()) {
        get_msg = "empty";
        pthread_mutex_unlock(&mutex);
        return;
    }
    get_msg = mq_.front();
    mq_.pop();
    pthread_mutex_unlock(&mutex);
}

void QueueKafka::put_queue(const string& str_msg) {
    pthread_mutex_lock(&mutex);
    mq_.push(str_msg);
    pthread_mutex_unlock(&mutex);  
}
