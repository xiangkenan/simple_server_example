#include "queue.h"

using namespace std;

string QueueKafka::get_queue() {
    string msg_str = "";
    if (mq_.empty()) {
        usleep(1000);
        return msg_str;
    }
    pthread_mutex_lock(&mutex);
    msg_str = mq_.front();
    mq_.pop();
    pthread_mutex_unlock(&mutex);  
    return msg_str;
}

void QueueKafka::put_queue(const string& str_msg) {
    pthread_mutex_lock(&mutex);
    mq_.push(str_msg);
    pthread_mutex_unlock(&mutex);  
}
