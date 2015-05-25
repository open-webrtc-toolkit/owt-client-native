//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#include <iostream>
#include "talk/woogeen/sdk/conference/socketsignalingchannel.h"

namespace woogeen {
  SocketSignalingChannel::SocketSignalingChannel() : socket_client_(new sio::client()) {
  }

  void SocketSignalingChannel::Connect(const std::string &token, std::function<void (Json::Value &room_info)> on_success, std::function<void (std::unique_ptr<ConferenceException>)> on_failure){
    Json::Value jsonToken;
    Json::Reader reader;
    if(!reader.parse(token, jsonToken)){
      std::cout<<"Error parse token."<<std::endl;
    }
    std::string scheme("http://");
    std::string host;
    std::string signature;
    std::string token_id;
    GetStringFromJsonObject(jsonToken, "host", &host);
    GetStringFromJsonObject(jsonToken, "tokenId", &token_id);
    GetStringFromJsonObject(jsonToken, "signature", &signature);
    std::cout<<signature<<std::endl;
    socket_client_->socket();
    socket_client_->set_open_listener([=](void){
      sio::message::ptr token_message = sio::object_message::create();
      token_message->get_map()["host"]=sio::string_message::create(host);
      token_message->get_map()["tokenId"]=sio::string_message::create(token_id);
      token_message->get_map()["signature"]=sio::string_message::create(signature);
      socket_client_->socket()->emit("token", token_message, [&](sio::message::ptr const& message){
        if(message->get_flag()!=sio::message::flag_object){
          std::cout<<"Received unkown message while sending token."<<std::endl;
          if(on_failure!=nullptr){
            // TODO: invoke on_failure
          }
        }
        if(on_success==nullptr)
          return;
        Json::Value room_info;
        if(message->get_map()["p2p"]==nullptr)
          room_info["p2p"]=false;
        else
          room_info["p2p"]=true;
        // Streams in the room
        std::vector<Json::Value> streams;
        auto stream_messages=message->get_map()["streams"]->get_vector();
        for(auto it=stream_messages.begin();it!=stream_messages.end();++it){
          Json::Value stream;
          stream["id"]=(*it)->get_map()["id"]->get_string();
          // Very ugly code for video type because MCU sends unconsistent messages :(
          if((*it)->get_map()["video"]!=nullptr){
            Json::Value video;
            if((*it)->get_map()["video"]->get_flag()==sio::message::flag_object){;
              video["category"]=(*it)->get_map()["video"]->get_map()["category"]->get_string();
            }
            stream["video"]=video;
          }
          streams.push_back(stream);
        }
        room_info["streams"]=ValueVectorToJsonArray(streams);
        std::cout<<"Room info: "<<room_info;
        on_success(room_info);
      });
    });
    socket_client_->connect(scheme.append(host));
  }
}
