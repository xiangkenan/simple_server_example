#ifndef QUEUE_H_
#define QUEUE_H_

#include <iostream>
#include <queue>
#include <unistd.h>

class QueueKafka {
    public:
        QueueKafka() {
            pthread_mutex_init(&mutex, NULL);  
        }
        ~QueueKafka() {}
        void get_queue(std::string& get_msg);
        void put_queue(const std::string& str_msg);
        inline bool empty() {
            if (mq_.empty())
                return true;
            return false;
        }
    private:
        std::queue<std::string> mq_;
        pthread_mutex_t mutex;
};

#endif
