#include "kafka_consume.h"

bool kafka_consumer_client::run_ = true;

kafka_consumer_client::kafka_consumer_client(const std::string& brokers, const std::string& topics, std::string groupid, int64_t offset, int32_t partition)
    :brokers_(brokers),
    topics_(topics),
    groupid_(groupid),
    offset_(offset),
    partition_(partition){
    }

kafka_consumer_client::kafka_consumer_client(){}

kafka_consumer_client::~kafka_consumer_client(){}

bool kafka_consumer_client::initClient(QueueKafka* queue_kafka){

    queue_kafka_ = queue_kafka;

    RdKafka::Conf *conf = nullptr;
    conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    if(!conf){
        fprintf(stderr, "RdKafka create global conf failed\n");
        return false;
    }

    std::string errstr;
    /*设置broker list*/
    if (conf->set("bootstrap.servers", brokers_, errstr) != RdKafka::Conf::CONF_OK){
        fprintf(stderr, "RdKafka conf set brokerlist failed : %s\n", errstr.c_str());
    }

    /*设置consumer group*/
    if (conf->set("group.id", groupid_, errstr) != RdKafka::Conf::CONF_OK){
        fprintf(stderr, "RdKafka conf set group.id failed : %s\n", errstr.c_str());
    }

    std::string strfetch_num = "10240000";
    /*每次从单个分区中拉取消息的最大尺寸*/
    if(conf->set("max.partition.fetch.bytes", strfetch_num, errstr) != RdKafka::Conf::CONF_OK){
        fprintf(stderr, "RdKafka conf set max.partition failed : %s\n", errstr.c_str());
    }

    /*创建kafka consumer实例*/
    kafka_consumer_ = RdKafka::Consumer::create(conf, errstr);
    if(!kafka_consumer_){
        fprintf(stderr, "failed to ceate consumer\n");
    }
    delete conf;

    RdKafka::Conf *tconf = nullptr;
    /*创建kafka topic的配置*/
    tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
    if(!tconf){
        fprintf(stderr, "RdKafka create topic conf failed\n");
        return false;
    }

    /*kafka + zookeeper,当消息被消费时,会想zk提交当前groupId的consumer消费的offset信息,
      当consumer再次启动将会从此offset开始继续消费.在consumter端配置文件中(或者是
      ConsumerConfig类参数)有个"autooffset.reset"(在kafka 0.8版本中为auto.offset.reset),
      有2个合法的值"largest"/"smallest",默认为"largest",此配置参数表示当此groupId下的消费者,
      在ZK中没有offset值时(比如新的groupId,或者是zk数据被清空),consumer应该从哪个offset开始
      消费.largest表示接受接收最大的offset(即最新消息),smallest表示最小offset,即从topic的
      开始位置消费所有消息.*/
    //if(tconf->set("auto.offset.reset", "smallest", errstr) != RdKafka::Conf::CONF_OK){
    if(tconf->set("auto.offset.reset", "largest", errstr) != RdKafka::Conf::CONF_OK){
        fprintf(stderr, "RdKafka conf set auto.offset.reset failed : %s\n", errstr.c_str());
    }

    topic_ = RdKafka::Topic::create(kafka_consumer_, topics_, tconf, errstr);
    if(!topic_){
        fprintf(stderr, "RdKafka create topic failed : %s\n", errstr.c_str());
    }
    delete tconf;

    RdKafka::ErrorCode resp = kafka_consumer_->start(topic_, partition_, offset_);
    if (resp != RdKafka::ERR_NO_ERROR){
        fprintf(stderr, "failed to start consumer : %s\n", RdKafka::err2str(resp).c_str());
    }

    return true;
}

void kafka_consumer_client::consumer(RdKafka::Message *message, void *opt, UserQuery *user_query){
    string behaver_message;
    string log_str;

    switch(message->err()){
        case RdKafka::ERR__TIMED_OUT:
            //LOG(WARNING) << "ERR__TIMED_OUT";
            break;
        case RdKafka::ERR_NO_ERROR:
            //printf("%.*s\n\n", 
            //        static_cast<int>(message->len()),
            //        static_cast<const char*>(message->payload()));
            //len = static_cast<int>(message->len());
            //kafka_consumer_->stop(topic_, partition_);  
            //behaver_message = static_cast<const char*>(message->payload());
            behaver_message = static_cast<const char*>(message->payload());
            behaver_message = behaver_message.substr(0, message->len());
            //cout << behaver_message << endl;
            //struct timeval start_time;
            //gettimeofday(&start_time, NULL);
            //处理kafka用户信息
            //user_query->Run(behaver_message, log_str);
            //当锁定是后，直接丢弃数据
            if (user_query->run_) {
                queue_kafka_->put_queue(behaver_message);
            }
            //cout << write_ms_log(start_time, "cost:") << endl;
            //kafka_consumer_->start(topic_, partition_, offset_);
            //if (!log_str.empty()) {
            //    LOG(INFO) << log_str << write_ms_log(start_time, "cost:");
            //}
            last_offset_ = message->offset();
            break;
        case RdKafka::ERR__PARTITION_EOF:
            //LOG(WARNING) << "ERR__PARTITION_EOF=>" << last_offset_ << "Reached the end of the queue, offset: ";
            break;
        case RdKafka::ERR__UNKNOWN_TOPIC:
            LOG(WARNING) << "ERR__UNKNOWN_TOPIC";
        case RdKafka::ERR__UNKNOWN_PARTITION:
            LOG(WARNING) << "ERR__UNKNOWN_PARTITION=>" << "Consume failed: " << message->errstr() << endl;
            run_ = false;
            break;
        default:
            LOG(WARNING) << "default=>" << "Consume failed: " << message->errstr()  << endl;
            run_ = false;
            break;
    }
}

bool kafka_consumer_client::consume(int timeout_ms, UserQuery *user_query, int i) {
    RdKafka::Message *msg = nullptr;

    while(run_){
        //while (!user_query->run_){
        //    ofstream ofile;
        //    ofile.open("./conf/offset/offset_" + to_string(i) + ".txt");
        //    if (last_offset_ == 0) {
        //        ofile << "-1";
        //    } else {
        //        ofile << last_offset_;
        //    }
        //    ofile.close();

        //    sleep(2);
        //}
        msg = kafka_consumer_->consume(topic_, partition_, timeout_ms);

        consumer(msg, nullptr, user_query);
        kafka_consumer_->poll(0);
        delete msg;
    }

    kafka_consumer_->stop(topic_, partition_);
    if(topic_){
        delete topic_;
        topic_ = nullptr;
    }
    if(kafka_consumer_){
        delete kafka_consumer_;
        kafka_consumer_ = nullptr;
    }

    /*销毁kafka实例*/
    RdKafka::wait_destroyed(5000);
    LOG(WARNING) << "已销毁实例";
    return true;
}
