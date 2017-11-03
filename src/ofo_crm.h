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

class OfoCrm {
    public:
        OfoCrm() {};
        ~OfoCrm() {};

        bool Run(int num);
        UserQuery user_query;
};

class RunKafka {
    public:
        RunKafka(OfoCrm* ofo_crm, int num) {
            ofo_crm_ = ofo_crm;
            num_ = num;
        }
        OfoCrm *ofo_crm_;
        int num_;
};

#endif
