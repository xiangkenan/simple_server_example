#ifndef OFO_CRM_H_
#define OFO_CRM_H_

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <getopt.h>
#include <csignal>
#include <iostream>

#include "rdkafkacpp.h"
#include "userquery.h"
#include "queue.h"

class OfoCrm {
    public:
        OfoCrm() {};
        ~OfoCrm() {};

        bool Run(int num, QueueKafka* queue_kafka, long offset);
        bool run_operate(QueueKafka* queue_kafka);
        UserQuery user_query;
};

class RunKafka {
    public:
        RunKafka(OfoCrm* ofo_crm, int num, QueueKafka* queue_kafka, long offset) {
            ofo_crm_ = ofo_crm;
            num_ = num;
            queue_kafka_ = queue_kafka;
            offset_ = offset;
        }
        OfoCrm *ofo_crm_;
        int num_;
        QueueKafka* queue_kafka_;
        long offset_;
};

class RunQueue {
    public:
        RunQueue(OfoCrm* ofo_crm, QueueKafka* queue_kafka) {
            ofo_crm_ = ofo_crm;
            queue_kafka_ = queue_kafka;
        }
        OfoCrm *ofo_crm_;
        QueueKafka* queue_kafka_;
};

#endif
