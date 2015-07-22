//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#include <iostream>
#include <thread>
#include "talk/woogeen/sdk/conference/socketsignalingchannel.h"
#include "webrtc/base/logging.h"

namespace woogeen {
SocketSignalingChannel::SocketSignalingChannel() : socket_client_(new sio::client()) {
}

SocketSignalingChannel::~SocketSignalingChannel(){
  delete socket_client_;
}

void SocketSignalingChannel::AddObserver(std::shared_ptr<ConferenceSignalingChannelObserver> observer){
  observers_.push_back(observer);
}

void SocketSignalingChannel::RemoveObserver(std::shared_ptr<ConferenceSignalingChannelObserver> observer){
  // TODO:
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
  rtc::GetStringFromJsonObject(jsonToken, "host", &host);
  rtc::GetStringFromJsonObject(jsonToken, "tokenId", &token_id);
  rtc::GetStringFromJsonObject(jsonToken, "signature", &signature);
  std::cout<<signature<<std::endl;
  socket_client_->socket();
  socket_client_->set_open_listener([=](void){
    sio::message::ptr token_message = sio::object_message::create();
    token_message->get_map()["host"]=sio::string_message::create(host);
    token_message->get_map()["tokenId"]=sio::string_message::create(token_id);
    token_message->get_map()["signature"]=sio::string_message::create(signature);
    socket_client_->socket()->emit("token", token_message, [=](sio::message::list const& msg){
      if(msg.size()<2){
        std::cout<<"Received unkown message while sending token."<<std::endl;
        if(on_failure!=nullptr){
          std::unique_ptr<ConferenceException> e(new ConferenceException(ConferenceException::kUnkown, "Received unkown message from server."));
          std::thread t(on_failure, std::move(e));
          t.detach();
        }
        return;
      }
      if(on_success==nullptr)
        return;
      sio::message::ptr ack=msg.at(0);  // The first element indicates the state.
      std::string state=ack->get_string();
      if(state=="error"||state=="timeout"){
        std::cout<<"Server returns "<<state<<" while joining a conference."<<std::endl;
        if(on_failure!=nullptr){
          std::unique_ptr<ConferenceException> e(new ConferenceException(ConferenceException::kUnkown, "Received error message from server."));
          std::thread t(on_failure, std::move(e));
          t.detach();
        }
        return;
      }
      sio::message::ptr message=msg.at(1);  // The second element is room info, please refer to MCU erizoController's implementation for detailed message format.
      Json::Value room_info;
      if(message->get_map()["p2p"]==nullptr)
        room_info["p2p"]=false;
      else
        room_info["p2p"]=true;
      // Streams in the room
      std::vector<Json::Value> streams;
      auto stream_messages=message->get_map()["streams"]->get_vector();
      for(auto it=stream_messages.begin();it!=stream_messages.end();++it){
        Json::Value stream = ParseStream(*it);
        streams.push_back(stream);
      }
      room_info["streams"]=rtc::ValueVectorToJsonArray(streams);
      std::cout<<"Room info: "<<room_info;
      on_success(room_info);
    });
  });
  socket_client_->socket()->on("onAddStream", sio::socket::event_listener_aux([&](std::string const& name, sio::message::ptr const& data, bool is_ack, sio::message::ptr &ack_resp){
    std::cout<<"Received on add stream."<<std::endl;
    Json::Value stream=ParseStream(data);
    for(auto it=observers_.begin();it!=observers_.end();++it){
      (*it)->OnStreamAdded(stream);
    }
  }));
  socket_client_->connect(scheme.append(host));
}

void SocketSignalingChannel::Disconnect(std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)>on_failure){
  socket_client_->close();
  std::thread t(on_success);  // TODO: Check if socket client is connected.
  t.detach();
}

void SocketSignalingChannel::SendSdp(Json::Value &options, std::string &sdp, bool is_publish, std::function<void(Json::Value &ack, std::string& stream_id)> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  LOG(LS_INFO) << "Send publish event.";
  std::string state;
  std::string audio;
  std::string video;
  std::string stream_id;
  rtc::GetStringFromJsonObject(options, "state", &state);
  rtc::GetStringFromJsonObject(options, "audio", &audio);
  rtc::GetStringFromJsonObject(options, "video", &video);  // TODO(jianjun): include more information.
  rtc::GetStringFromJsonObject(options, "streamId", &stream_id);
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
  socket_client_->socket()->emit(event_name, message_list, [=](sio::message::list const& msg){
    LOG(LS_INFO) << "Received ack from server.";
    if(on_success==nullptr){
      LOG(LS_WARNING) << "Does not implement success callback. Make sure it is what you want.";
      return;
    }
    sio::message::ptr message = msg.at(0);
    if(message->get_flag()!=sio::message::flag_string){
      LOG(LS_WARNING) << "The first element of publish ack is not a string.";
      if(on_failure){
        std::unique_ptr<ConferenceException> e(new ConferenceException(ConferenceException::kUnkown, "Received unkown message from server."));
        on_failure(std::move(e));
      }
      return;
    }
    Json::Reader reader;
    Json::Value ack_json;
    if(!reader.parse(message->get_string(), ack_json)){
      LOG(LS_WARNING) << "Cannot parse answer.";
      if(on_failure){
        std::unique_ptr<ConferenceException> e(new ConferenceException(ConferenceException::kUnkown, "Received unkown message from server."));
        on_failure(std::move(e));
      }
      return;
    }
    std::string sid=stream_id;
    if(event_name=="publish"){
      sio::message::ptr id = msg.at(1);
      if(id->get_flag()!=sio::message::flag_string){
        LOG(LS_WARNING) << "The second element of publish ack is not a string. (Expected to be stream ID)";
        return;
      }
      sid = id->get_string();
    }
    on_success(ack_json, sid);
  });
}

void SocketSignalingChannel::SendStreamEvent(const std::string& event, const std::string& stream_id, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)>on_failure){
  sio::message::ptr message = sio::string_message::create(stream_id);
  socket_client_->socket()->emit(event, message, [=](sio::message::list const& msg){
    sio::message::ptr ack = msg.at(0);
    if(ack->get_flag()!=sio::message::flag_string){
      LOG(LS_WARNING) << "The first element of stream event ack is not a string.";
      if(on_failure){
        std::unique_ptr<ConferenceException> e(new ConferenceException(ConferenceException::kUnkown, "Received unkown message from server."));
        std::thread t(on_failure, std::move(e));
        t.detach();
      }
      return;
    }
    if(ack->get_string()=="success"){
      if(on_success!=nullptr){
        std::thread t(on_success);
        t.detach();
      }
    }
    else {
      LOG(LS_WARNING) << "Send stream event received negative ack.";
      if(on_failure!=nullptr){
        std::unique_ptr<ConferenceException> e(new ConferenceException(ConferenceException::kUnkown, "Negative acknowledgement for stream event."));
        std::thread t(on_failure, std::move(e));
        t.detach();
      }
    }
  });
}

Json::Value SocketSignalingChannel::ParseStream(const sio::message::ptr stream_message){
  Json::Value stream;
  stream["id"]=stream_message->get_map()["id"]->get_string();
  stream["from"]=stream_message->get_map()["from"]->get_string();
  // Very ugly code for video type because MCU sends unconsistent messages :(
  if(stream_message->get_map()["video"]!=nullptr){
    Json::Value video_json;
    if(stream_message->get_map()["video"]->get_flag()==sio::message::flag_object){
      auto video = stream_message->get_map()["video"]->get_map();
      if(video.find("device")!=video.end())
        video_json["device"]=video["device"]->get_string();
      if(video.find("resolution")!=video.end())
        video_json["resolution"]=video["resolution"]->get_string();
    }
    stream["video"]=video_json;
  }
  return stream;
}
}
