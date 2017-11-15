#! /bin/bash -x
DIR=`pwd`
SRC_DIR="$DIR/proto"
PROTO_DIR="$DIR/proto"

cd $PROTO_DIR
protoc --cpp_out=${SRC_DIR}/ *.proto
protoc --grpc_out=${SRC_DIR}/ --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin redpacket.proto
cd -
