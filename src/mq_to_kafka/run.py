from rabbitmq_consumer import RabbitMQConsumer
import logging

conf_dict = {}
conf_dict['url'] = "amqp://p_ofo_order:dl3lAl0Ro0e@192.168.1.133:5672/order?heartbeat=15"
conf_dict['exchange'] = "ex-eorder-direct.prod"
conf_dict['queue'] = "q.eorder_kafka.prod"
conf_dict['key'] = "rk.eorder.prod"
#conf_dict['exchange_type'] = "fanout"
conf_dict['exchange_type'] = "direct"
conf_dict['queue_durable'] = ""

logging.basicConfig(level=logging.INFO,
        format='%(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S',
        filename='crm_mq_kafka.log',
        filemode='w')

rabbit_mq_consumer = RabbitMQConsumer(conf_dict, logging)
body = rabbit_mq_consumer.run()
