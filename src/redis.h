#ifndef REDIS_H_
#define REDIS_H_

#include <iostream>
#include <string>

#include <hiredis/hiredis.h>
#include <glog/logging.h>

class Redis {
  public:
    Redis() {
        connect_ = NULL;
        reply = NULL;
    }
    ~Redis() {
        if(connect_ != NULL)
           redisFree(connect_);

        connect_ = NULL;
        reply = NULL;
    }

    bool Connect(const std::string& host, const int port, const std::string& passwd = "") {
        struct timeval tv = {0, 100000};
        connect_ = redisConnectWithTimeout(host.c_str(), port, tv);
        if (connect_ != NULL && connect_->err) {
            LOG(ERROR) << "redis " << host << ":" << port << " connect error: " << connect_->errstr;
            return false;
        }

        if (!passwd.empty()) {
            reply = (redisReply*) redisCommand(connect_, "AUTH %s", passwd.c_str());
            bool error = (reply->type == REDIS_REPLY_ERROR);
            freeReplyObject(reply);
            if (error) {
                LOG(ERROR) << "redis " << host << ":" << port << " auth failed";
                return false;
            }
        }

        return true;
    }

    bool Expire(const std::string &key, const int time) {
        reply = (redisReply*) redisCommand(connect_, "EXPIRE %s %d", key.c_str(), time);

        bool error = (reply->type == REDIS_REPLY_ERROR);
        freeReplyObject(reply);

        if (error) {
            LOG(ERROR) << "Failed to set expire time: " << key;
            return false;
        }

        return true;
    }

    bool Get(const std::string& key, std::string* value) {
        value->clear();
        reply = (redisReply*) redisCommand(connect_, "GET %s", key.c_str());
        if (reply->len > 0) {
            value->assign(reply->str);
        }

        bool error = (reply->type == REDIS_REPLY_ERROR);
        freeReplyObject(reply);

        if (error) {
            LOG(ERROR) << "Failed to get value from key: " << key;
            return false;
        }

        return true;
    }

    bool HGetAll(const std::string &id, std::map<std::string, std::string> *value) {
        value->clear();
        reply = (redisReply*) redisCommand(connect_, "HGETALL %s", id.c_str());

        bool error = (reply->type == REDIS_REPLY_ERROR);
        if (error) {
            LOG(ERROR) << "Failed to hgetall value from key: " << id;
            freeReplyObject(reply);
            return false;
        }

        if (reply->element != NULL) {
            for (int i = 0; (reply->element)[i] != NULL; i += 2) {
                value->insert(std::pair<std::string, std::string>((reply->element)[i]->str, (reply->element)[i+1]->str));
                //value[(reply->element)[i]->str] = (reply->element)[i+1]->str;
            }
        }
        freeReplyObject(reply);

        return true;
    }

    bool HGet(const std::string &id, const std::string &key ,std::string *value) {
        value->clear();
        reply = (redisReply*) redisCommand(connect_, "HGET %s %s", id.c_str(), key.c_str());
        if (reply->len > 0) {
            value->assign(reply->str);
        }

        bool error = (reply->type == REDIS_REPLY_ERROR);
        freeReplyObject(reply);

        if (error) {
            LOG(ERROR) << "Failed to hget value from key: " << id << ":" << key;
            return false;
        }

        return true;
    }

    bool Set(const std::string& key, const std::string& value, const int expire_time = 0) {
        if (expire_time > 0) {
            reply = (redisReply*) redisCommand(connect_, "SETEX %s %d %s", key.c_str(), expire_time, value.c_str());
        } else {
            reply = (redisReply*) redisCommand(connect_, "SET %s %s", key.c_str(), value.c_str());
        }

        bool error = (reply->type == REDIS_REPLY_ERROR);
        freeReplyObject(reply);
        if (error) {
            LOG(ERROR) << "Failed to set " << key << " " << value << " expire time: " << expire_time;
            return false;
        }

        return true;
    }

    bool Incr(const std::string& key) {
        reply = (redisReply*) redisCommand(connect_, "INCR %s", key.c_str());
        bool error = (reply->type == REDIS_REPLY_ERROR);
        freeReplyObject(reply);
        if (error) {
            LOG(ERROR) << "Failed to incr " << key;
            return false;
        }

        return true;
    }

    bool GetTTL(const std::string& key, int* ttl) {
        reply = (redisReply*) redisCommand(connect_, "TTL %s", key.c_str());
        bool error = (reply->type == REDIS_REPLY_ERROR);
        *ttl = reply->integer;
        freeReplyObject(reply);
        if (error) {
            LOG(ERROR) << "Failed to get ttl " << key;
            return false;
        }

        return true;
    }

  private:
    redisContext* connect_;
    redisReply* reply;
};

#endif  //_REDIS_H_
