#ifndef QUEUE_H_
#define QUEUE_H_

#include <iostream>
#include <queue>

class QueueKafka {
    public:
        QueueKafka() {
            pthread_mutex_init(&m_mutex,NULL);
        }
        ~QueueKafka() {}
        std::string get_queue();
        void put_queue(const std::string& str_msg);
    private:
        std::queue<std::string> mq_;
        pthread_mutex_t m_mutex;
};

#endif
