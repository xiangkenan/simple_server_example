CC=g++
DIR=$(shell pwd)
SRC=${DIR}/src
LIB=${DIR}/lib

INCLUDE=  -I/usr/local/include/librdkafka \
		  -I./lib/glog/include \
		  -I./lib/jsoncpp/include \
		  -I./lib/curl/include \
		  -I./lib/murl/include

LDFLAGS=  -L/usr/local/lib -lrdkafka \
		  -L./lib/glog/lib -lglog -Wl,-rpath,./lib/glog/lib \
		  -L./lib/jsoncpp/lib -ljsoncpp \
		  -L./lib/curl/lib -lcurl \
		  -L./lib/curl/lib -lcares \
		  -L./lib/murl/lib -lmurl


CFLAG= ${INCLUDE} ${LDFLAGS} -Wall -std=c++11  -lrdkafka++ -lz -lpthread -lrt -lhiredis -lpthread

all: run clean

run: kafka_consume.o main.o userquery.o string_tools.o ofo_crm.o noah_config.o queue.o
	${CC} $^ -o $@ ${CFLAG}

main.o:
	${CC} -c -o $@ ${SRC}/main.cc ${CFLAG}

userquery.o:
	${CC} -c -o $@ ${SRC}/userquery.cc ${CFLAG}

kafka_consume.o:
	${CC} -c -o $@ ${SRC}/kafka_consume.cc ${CFLAG}

string_tools.o:
	${CC} -c -o $@ ${SRC}/string_tools.cc ${CFLAG}

ofo_crm.o:
	${CC} -c -o $@ ${SRC}/ofo_crm.cc ${CFLAG}

noah_config.o:
	${CC} -c -o $@ ${SRC}/noah_config.cc ${CFLAG}

queue.o:
	${CC} -c -o $@ ${SRC}/queue.cc ${CFLAG}

clean:
	rm -fr *.o
