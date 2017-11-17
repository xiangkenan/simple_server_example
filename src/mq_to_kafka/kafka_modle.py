# -*- coding: utf-8 -*-
from kafka import KafkaProducer
from kafka import KafkaConsumer
from kafka.errors import KafkaError
import json


class Kafka_producer():
    '''
    使用kafka的生产模块
    '''

    def __init__(self, kafkahost,kafkaport, kafkatopic):
        self.kafkaHost = kafkahost
        self.kafkaPort = kafkaport
        self.kafkatopic = kafkatopic
        self.producer = KafkaProducer(bootstrap_servers = '{kafka_host}:{kafka_port}'.format(
            kafka_host=self.kafkaHost,
            kafka_port=self.kafkaPort
            ))

    def sendjsondata(self, params):
        try:
            parmas_message = json.dumps(params)
            producer = self.producer
            producer.send(self.kafkatopic, parmas_message.encode('utf-8'))
            producer.flush()
        except KafkaError as e:
            print e


class Kafka_consumer():
    '''
    使用Kafka—python的消费模块
    '''

    def __init__(self, kafkahost, kafkaport, kafkatopic, groupid):
        self.kafkaHost = kafkahost
        self.kafkaPort = kafkaport
        self.kafkatopic = kafkatopic
        self.groupid = groupid
        self.consumer = KafkaConsumer(self.kafkatopic, group_id = self.groupid,
                                      bootstrap_servers = '{kafka_host}:{kafka_port}'.format(
            kafka_host=self.kafkaHost,
            kafka_port=self.kafkaPort ))

    def consume_data(self):
        print (self.groupid)
        print (self.kafkaHost)
        print (self.kafkaPort)
        print (self.kafkatopic)
        try:
            for message in self.consumer:
                message.value.decode('utf-8')
        except KeyboardInterrupt, e:
            print e


#def main():
#    '''
#    测试consumer和producer
#    :return:
#    '''
#    producer = Kafka_producer('127.0.0.1', 9092, "crm_test_kenan")
#    producer.sendjsondata("nihaoaaaa")
#
#
#    #consumer = Kafka_consumer('192.168.30.236', 9092, "userevents", 'crm_noah_kenan_test')
#    #message = consumer.consume_data()
#    #for i in message:
#    #    print i.value
#
#
#if __name__ == '__main__':
#    main()
