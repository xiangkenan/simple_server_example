CC=g++
DIR=$(shell pwd)
SRC=${DIR}/src
LIB=${DIR}/lib

INCLUDE=  -I/usr/local/include/librdkafka \
		  -I./lib/glog/include \
		  -I./lib/jsoncpp/include \
		  -I./lib/curl/include

LDFLAGS=  -L/usr/local/lib -lrdkafka \
		  -L./lib/glog/lib -lglog -Wl,-rpath,./lib/glog/lib \
		  -L./lib/jsoncpp/lib -ljsoncpp \
		  -L./lib/curl/lib -lcurl \
		  -L./lib/curl/lib -lcares


CFLAG= ${INCLUDE} ${LDFLAGS} -Wall -std=c++11  -lrdkafka++ -lz -lpthread -lrt -lhiredis

all: run clean

run: kafka_consume.o main.o userquery.o
	${CC} $^ -o $@ ${CFLAG}

main.o:
	${CC} -c -o $@ ${SRC}/main.cc ${CFLAG}

userquery.o:
	${CC} -c -o $@ ${SRC}/userquery.cc ${CFLAG}

kafka_consume.o:
	${CC} -c -o $@ ${SRC}/kafka_consume.cc ${CFLAG}

clean:
	rm -fr *.o
