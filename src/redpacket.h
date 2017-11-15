#ifndef REDPACKET_H_
#define REDPACKET_H_

#include <unordered_map>
#include <grpc++/grpc++.h>

#include "redpacket.grpc.pb.h"
#include "redpacket.pb.h"

using namespace std;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using ofo::redpacket::v1::redpacketService;
using ofo::redpacket::v1::addRedpacketRequest;
using ofo::redpacket::v1::addRedpacketResponse;

class Redpacket {
    public:
        Redpacket(std::shared_ptr<Channel> channel)
            : stub_(redpacketService::NewStub(channel)) {}

        bool send_redpacket (std::unordered_map<string, string> request_args, addRedpacketResponse* response) {
            addRedpacketRequest request;
            request.set_user_id(atol(request_args["user_id"].c_str()));
            request.set_source(atoi(request_args["source"].c_str()));
            request.set_action(atoi(request_args["action"].c_str()));
            request.set_money(atoi(request_args["money"].c_str()));
            request.set_desc(request_args["desc"]);
            request.set_request_id(request_args["request_id"]);
            ClientContext context;
            Status status;
            status = stub_->addRedpacket(&context, request, response);

            cout << response->code() << ":" << response->msg() << ":" << response->packet_id() << ":" << response->is_repeat() << endl;
            return true;
        }

    private:
        std::unique_ptr<redpacketService::Stub> stub_;

};


#endif
