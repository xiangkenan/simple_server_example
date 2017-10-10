#ifndef USERQUERY_H_
#define USERQUERY_H_

#include <iostream>
#include <string>
#include <glog/logging.h>

#include "redis.h"

class UserQuery{
    public:
        UserQuery() {};
        ~UserQuery() {};
        bool Init(std::string behaver_message);
    private:
        bool InitRedis();
        bool Parse(std::string behaver_message);
        void Split(const std::string& s,
                const std::string& delim,
                std::vector<std::string>* ret) {
            size_t last = 0;
            size_t index = s.find_first_of(delim, last);
            while (index != std::string::npos) {
                ret->push_back(s.substr(last, index - last));
                last = index + 1;
                index = s.find_first_of(delim, last);
            }
            if (index - last > 0) {
                ret->push_back(s.substr(last, index - last));
            }
        }

        Redis *redis_userid;

};

#endif
