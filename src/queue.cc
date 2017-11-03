#include "queue.h"

using namespace std;

string QueueKafka::get_queue() {
    if (mq_.empty()) {
        return "";
    }
    pthread_mutex_lock(&m_mutex);
    string msg_str = mq_.front();
    mq_.pop();
    pthread_mutex_unlock(&m_mutex);
    return msg_str;
}

void QueueKafka::put_queue(const string& str_msg) {
    pthread_mutex_lock(&m_mutex);
    mq_.push(str_msg);
    pthread_mutex_unlock(&m_mutex);
}

