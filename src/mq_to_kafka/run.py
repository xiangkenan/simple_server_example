from rabbitmq_consumer import RabbitMQConsumer
import logging
import sys

#choose = "sorder"
#choose = "eorder"
choose = sys.argv[1]
conf_dict = {}
if choose == "sorder":
    conf_dict['url'] = "amqp://p_ofo_order:dl3lAl0Ro0e@192.168.1.133:5672/order?heartbeat=15"
    conf_dict['exchange'] = "ex-sorder-direct.prod"
    conf_dict['queue'] = "q.sorder_kafka.prod"
    conf_dict['key'] = "rk.sorder.prod"
    #conf_dict['exchange_type'] = "fanout"
    conf_dict['exchange_type'] = "direct"
    conf_dict['queue_durable'] = ""
    
    conf_dict['ka_ip'] = "192.168.30.236"
    conf_dict['ka_port'] = 9092
    conf_dict['topic'] = "crm_order_action"
    conf_dict['order_action'] = "sorder"

elif choose == "eorder":
    conf_dict['url'] = "amqp://p_ofo_order:dl3lAl0Ro0e@192.168.1.133:5672/order?heartbeat=15"
    conf_dict['exchange'] = "ex-eorder-direct.prod"
    conf_dict['queue'] = "q.eorder_kafka.prod"
    conf_dict['key'] = "rk.eorder.prod"
    #conf_dict['exchange_type'] = "fanout"
    conf_dict['exchange_type'] = "direct"
    conf_dict['queue_durable'] = ""
    
    conf_dict['ka_ip'] = "192.168.30.236"
    conf_dict['ka_port'] = 9092
    conf_dict['topic'] = "crm_order_action"
    conf_dict['order_action'] = "eorder"
else:
    sys.exit(0)

logging.basicConfig(level=logging.INFO,
        format='%(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S',
        filename='crm_mq_kafka.log',
        filemode='w')

rabbit_mq_consumer = RabbitMQConsumer(conf_dict, logging)
body = rabbit_mq_consumer.run()
