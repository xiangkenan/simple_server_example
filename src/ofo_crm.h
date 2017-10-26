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

        bool Run();
        UserQuery user_query;
};

#endif
