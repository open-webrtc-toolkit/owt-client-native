//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#include <iostream>
#include "talk/woogeen/sdk/conference/socketsignalingchannel.h"
#include "webrtc/base/logging.h"

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
    socket_client_->socket()->emit("token", token_message, [&](sio::message::ptr const& msg){
      if(msg->get_flag()!=sio::message::flag_array||msg->get_vector().size()<2){
        std::cout<<"Received unkown message while sending token."<<std::endl;
        if(on_failure!=nullptr){
          // TODO: invoke on_failure
        }
        return;
      }
      if(on_success==nullptr)
        return;
      sio::message::ptr message=msg->get_vector()[1];
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
          Json::Value video_json;
          if((*it)->get_map()["video"]->get_flag()==sio::message::flag_object){
            auto video = (*it)->get_map()["video"]->get_map();
            if(video.find("category")!=video.end())
              video_json["category"]=video["category"]->get_string();
            if(video.find("device")!=video.end())
              video_json["device"]=video["device"]->get_string();
            if(video.find("resolution")!=video.end())
              video_json["resolution"]=video["resolution"]->get_string();
          }
          stream["video"]=video_json;
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

void SocketSignalingChannel::SendSdp(Json::Value &options, std::string &sdp, bool is_publish, std::function<void(Json::Value &ack)> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  LOG(LS_INFO) << "Send publish event.";
  std::string state;
  std::string audio;
  std::string video;
  std::string stream_id;
  GetStringFromJsonObject(options, "state", &state);
  GetStringFromJsonObject(options, "audio", &audio);
  GetStringFromJsonObject(options, "video", &video);  // TODO(jianjun): include more information.
  GetStringFromJsonObject(options, "streamId", &stream_id);
  sio::message::ptr options_message = sio::object_message::create();
  options_message->get_map()["audio"]=sio::string_message::create(audio);
  options_message->get_map()["video"]=sio::string_message::create(video);
  options_message->get_map()["state"]=sio::string_message::create(state);
  options_message->get_map()["streamId"]=sio::string_message::create(stream_id);
  sio::message::ptr sdp_message = sio::string_message::create(sdp);
  sio::message::list message_list;
  message_list.push(options_message);
  message_list.push(sdp_message);
  std::string event_name;
  if(is_publish)
    event_name="publish";
  else
    event_name="subscribe";
  socket_client_->socket()->emit(event_name, message_list, [=](sio::message::ptr const& msg){
    LOG(LS_INFO) << "Received ack from server.";
    if(on_success==nullptr){
      LOG(LS_WARNING) << "Does not implement success callback. Make sure it is what you want.";
      return;
    }
    if(msg->get_flag()!= sio::message::flag_array){
      LOG(LS_WARNING) << "Received unknown ack after publish stream";
      if(on_failure){
        // TODO(jianjun): Trigger on_failure;
      }
      return;
    }
    sio::message::ptr message = msg->get_vector()[0];
    if(message->get_flag()!=sio::message::flag_string){
      LOG(LS_WARNING) << "The first element of publish ack is not a string.";
      if(on_failure){
        // TODO(jianjun): Trigger on_failure;
      }
      return;
    }
    LOG(LS_INFO) << "Answer: "<< message->get_string();
    Json::Reader reader;
    Json::Value ack_json;
    if(!reader.parse(message->get_string(), ack_json)){
      LOG(LS_WARNING) << "Cannot parse answer.";
      if(on_failure){
        // TODO(jianjun): Trigger on_failure;
      }
      return;
    }
    on_success(ack_json);
  });
}
}
