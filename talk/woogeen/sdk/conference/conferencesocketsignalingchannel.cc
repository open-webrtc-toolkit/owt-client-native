//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#include <iostream>
#include <thread>
#include <algorithm>
#include "talk/woogeen/sdk/conference/conferencesocketsignalingchannel.h"
#include "webrtc/base/common.h"
#include "webrtc/base/checks.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/json.h"

namespace woogeen {
namespace conference {

const std::string kEventNameCustomMessage = "customMessage";
const std::string kEventNameSignalingMessage = "signaling_message";
const std::string kEventNameOnSignalingMessage = "signaling_message_erizo";
const std::string kEventNameOnCustomMessage = "custom_message";
const std::string kEventNameStreamControl = "control";
const std::string kEventNameGetRegion = "getRegion";
const std::string kEventNameSetRegion = "setRegion";
const std::string kEventNameOnAddStream = "add_stream";
const std::string kEventNameOnRemoveStream = "remove_stream";
const std::string kEventNameOnUpdateStream = "update_stream";
const std::string kEventNameOnUserJoin = "user_join";
const std::string kEventNameOnUserLeave = "user_leave";

ConferenceSocketSignalingChannel::ConferenceSocketSignalingChannel()
    : socket_client_(new sio::client()) {}

ConferenceSocketSignalingChannel::~ConferenceSocketSignalingChannel() {
  delete socket_client_;
}

void ConferenceSocketSignalingChannel::AddObserver(
    ConferenceSocketSignalingChannelObserver& observer) {
  observers_.push_back(&observer);
}

void ConferenceSocketSignalingChannel::RemoveObserver(
    ConferenceSocketSignalingChannelObserver& observer) {
  observers_.erase(std::remove(observers_.begin(), observers_.end(), &observer),
                   observers_.end());
}

void ConferenceSocketSignalingChannel::Connect(
    const std::string& token,
    std::function<void(sio::message::ptr room_info)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  Json::Value jsonToken;
  Json::Reader reader;
  if (!reader.parse(token, jsonToken)) {
    std::cout << "Error parse token." << std::endl;
    if (on_failure != nullptr) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnkown, "Invalid token."));
      on_failure(std::move(e));
    }
    return;
  }
  std::string scheme("http://");
  std::string host;
  std::string signature;
  std::string token_id;
  rtc::GetStringFromJsonObject(jsonToken, "host", &host);
  rtc::GetStringFromJsonObject(jsonToken, "tokenId", &token_id);
  rtc::GetStringFromJsonObject(jsonToken, "signature", &signature);
  socket_client_->socket();
  socket_client_->set_socket_close_listener([this](std::string const& nsp) {
    LOG(LS_INFO) << "Socket.IO disconnected.";
    if (disconnect_complete_) {
      disconnect_complete_();
    }
    disconnect_complete_ = nullptr;
    for (auto it = observers_.begin(); it != observers_.end(); ++it) {
      (*it)->OnServerDisconnected();
    }
  });
  socket_client_->set_open_listener([=](void) {
    sio::message::ptr token_message = sio::object_message::create();
    token_message->get_map()["host"] = sio::string_message::create(host);
    token_message->get_map()["tokenId"] = sio::string_message::create(token_id);
    token_message->get_map()["signature"] =
        sio::string_message::create(signature);
    socket_client_->socket()->emit(
        "token", token_message, [=](sio::message::list const& msg) {
          if (msg.size() < 2) {
            std::cout << "Received unkown message while sending token."
                      << std::endl;
            if (on_failure != nullptr) {
              std::unique_ptr<ConferenceException> e(new ConferenceException(
                  ConferenceException::kUnkown,
                  "Received unkown message from server."));
              on_failure(std::move(e));
            }
            return;
          }
          if (on_success == nullptr)
            return;
          sio::message::ptr ack =
              msg.at(0);  // The first element indicates the state.
          std::string state = ack->get_string();
          if (state == "error" || state == "timeout") {
            std::cout << "Server returns " << state
                      << " while joining a conference." << std::endl;
            if (on_failure != nullptr) {
              std::unique_ptr<ConferenceException> e(new ConferenceException(
                  ConferenceException::kUnkown,
                  "Received error message from server."));
              on_failure(std::move(e));
            }
            return;
          }
          // The second element is room info, please refer to MCU
          // erizoController's implementation for detailed message format.
          sio::message::ptr message = msg.at(1);
          std::cout << "Message length: " << msg.size() << std::endl;
          on_success(message);
        });
  });
  socket_client_->socket()->on(
      kEventNameOnAddStream,
      sio::socket::event_listener_aux(
          [&](std::string const& name, sio::message::ptr const& data,
              bool is_ack, sio::message::list& ack_resp) {
            std::cout << "Received on add stream." << std::endl;
            for (auto it = observers_.begin(); it != observers_.end(); ++it) {
              (*it)->OnStreamAdded(data);
            }
          }));
  socket_client_->socket()->on(
      kEventNameOnCustomMessage,
      sio::socket::event_listener_aux(
          [&](std::string const& name, sio::message::ptr const& data,
              bool is_ack, sio::message::list& ack_resp) {
            std::cout << "Received custom message." << std::endl;
            std::string from = data->get_map()["from"]->get_string();
            std::string message = data->get_map()["data"]->get_string();
            for (auto it = observers_.begin(); it != observers_.end(); ++it) {
              (*it)->OnCustomMessage(from, message);
            }
          }));
  socket_client_->socket()->on(
      kEventNameOnUserJoin,
      sio::socket::event_listener_aux(
          [&](std::string const& name, sio::message::ptr const& data,
              bool is_ack, sio::message::list& ack_resp) {
            std::cout << "Received user join message." << std::endl;
            // auto user = std::make_shared<const conference::User>(
            //     ParseUser(data->get_map()["user"]));
            // for (auto it = observers_.begin(); it != observers_.end(); ++it)
            // {
            //   (*it)->OnUserJoined(user);
            // }
          }));
  socket_client_->socket()->on(
      kEventNameOnUserLeave,
      sio::socket::event_listener_aux(
          [&](std::string const& name, sio::message::ptr const& data,
              bool is_ack, sio::message::list& ack_resp) {
            std::cout << "Received user leave message." << std::endl;
            // auto user = std::make_shared<const conference::User>(
            //     ParseUser(data->get_map()["user"]));
            // for (auto it = observers_.begin(); it != observers_.end(); ++it)
            // {
            //   (*it)->OnUserLeft(user);
            // }
          }));
  socket_client_->socket()->on(
      kEventNameOnRemoveStream,
      sio::socket::event_listener_aux(
          [&](std::string const& name, sio::message::ptr const& data,
              bool is_ack, sio::message::list& ack_resp) {
            std::cout << "Received on remove stream." << std::endl;
            for (auto it = observers_.begin(); it != observers_.end(); ++it) {
              (*it)->OnStreamRemoved(data);
            }
          }));
  socket_client_->socket()->on(
      kEventNameOnUpdateStream,
      sio::socket::event_listener_aux(
          [&](std::string const& name, sio::message::ptr const& data,
              bool is_ack, sio::message::list& ack_resp) {
            std::cout << "Received on update stream." << std::endl;
            for (auto it = observers_.begin(); it != observers_.end(); ++it) {
              (*it)->OnStreamUpdated(data);
            }
          }));
  socket_client_->socket()->on(
      kEventNameOnSignalingMessage,
      sio::socket::event_listener_aux(
          [&](std::string const& name, sio::message::ptr const& data,
              bool is_ack, sio::message::list& ack_resp) {
            std::cout << "Received signaling message from erizo." << std::endl;
            for (auto it = observers_.begin(); it != observers_.end(); ++it) {
              (*it)->OnSignalingMessage(data);
            }
          }));
  socket_client_->connect(scheme.append(host));
}

void ConferenceSocketSignalingChannel::Disconnect(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!socket_client_->opened()) {
    if (on_failure) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnkown, "Socket.IO is not connected."));
      on_failure(std::move(e));
    }
    return;
  }
  disconnect_complete_=on_success;
  socket_client_->close();
}

void ConferenceSocketSignalingChannel::SendInitializationMessage(
    sio::message::ptr options,
    std::string publish_stream_label,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  sio::message::list message_list;
  message_list.push(options);
  // The second place used to be SDP.
  message_list.push(sio::object_message::create());
  std::string event_name;
  if (publish_stream_label != "")
    event_name = "publish";
  else
    event_name = "subscribe";
  socket_client_->socket()->emit(
      event_name, message_list, [=](sio::message::list const& msg) {
        LOG(LS_INFO) << "Received ack from server.";
        if (on_success == nullptr) {
          LOG(LS_WARNING) << "Does not implement success callback. Make sure "
                             "it is what you want.";
          return;
        }
        sio::message::ptr message = msg.at(0);
        if (message->get_flag() != sio::message::flag_string) {
          LOG(LS_WARNING)
              << "The first element of publish ack is not a string.";
          if (on_failure) {
            std::unique_ptr<ConferenceException> e(new ConferenceException(
                ConferenceException::kUnkown,
                "Received unkown message from server."));
            on_failure(std::move(e));
          }
          return;
        }
        if (message->get_string() == "initializing") {
          if (msg.size() == 1) {  // Subscribe
            on_success();
            return;
          }
          if (msg.at(1)->get_flag() != sio::message::flag_string) {
            ASSERT(false);
            return;
          }
          std::string stream_id = msg.at(1)->get_string();
          for (auto it = observers_.begin(); it != observers_.end(); ++it) {
            (*it)->OnStreamId(stream_id, publish_stream_label);
          }
          on_success();
          return;
        } else if (message->get_string() == "error" && msg.at(1) != nullptr &&
                   msg.at(1)->get_flag() == sio::message::flag_string) {
          if (on_failure) {
            std::unique_ptr<ConferenceException> e(new ConferenceException(
                ConferenceException::kUnkown, msg.at(1)->get_string()));
            on_failure(std::move(e));
          }
        } else {
          if (on_failure) {
            std::unique_ptr<ConferenceException> e(new ConferenceException(
                ConferenceException::kUnkown,
                "Ack for initializing message is not expected."));
            on_failure(std::move(e));
          }
          return;
        }
      });
}

void ConferenceSocketSignalingChannel::SendSdp(
    sio::message::ptr message,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  socket_client_->socket()->emit(kEventNameSignalingMessage, message);
  if (on_success) {  // MCU doesn't ack for this event.
    on_success();
  }
}

void ConferenceSocketSignalingChannel::SendStreamEvent(
    const std::string& event,
    const std::string& stream_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  sio::message::ptr message = sio::string_message::create(stream_id);
  socket_client_->socket()->emit(event, message,
                                 [=](sio::message::list const& msg) {
                                   OnEmitAck(msg, on_success, on_failure);
                                 });
}

void ConferenceSocketSignalingChannel::SendCustomMessage(
    const std::string& message,
    const std::string& receiver,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  sio::message::ptr send_message = sio::object_message::create();
  // If receiver is empty string, it means send this message to all participants
  // of the conference.
  if (receiver == "") {
    send_message->get_map()["receiver"] = sio::string_message::create("all");
  } else {
    send_message->get_map()["receiver"] = sio::string_message::create(receiver);
  }
  send_message->get_map()["type"] = sio::string_message::create("data");
  send_message->get_map()["data"] = sio::string_message::create(message);
  socket_client_->socket()->emit(kEventNameCustomMessage, send_message,
                                 [=](sio::message::list const& msg) {
                                   OnEmitAck(msg, on_success, on_failure);
                                 });
}
void ConferenceSocketSignalingChannel::SendStreamControlMessage(
    const std::string& stream_id,
    const std::string& action,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  sio::message::ptr send_message = sio::object_message::create();
  sio::message::ptr payload = sio::object_message::create();
  payload->get_map()["action"] = sio::string_message::create(action);
  payload->get_map()["streamId"] = sio::string_message::create(stream_id);
  send_message->get_map()["type"] = sio::string_message::create("control");
  send_message->get_map()["payload"] = payload;
  socket_client_->socket()->emit(kEventNameCustomMessage, send_message,
                                 [=](sio::message::list const& msg) {
                                   OnEmitAck(msg, on_success, on_failure);
                                 });
}

void ConferenceSocketSignalingChannel::GetRegion(
      const std::string& stream_id,
      std::function<void(std::string)> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure){
  sio::message::ptr send_message = sio::object_message::create();
  send_message->get_map()["id"] = sio::string_message::create(stream_id);
  socket_client_->socket()->emit(
      kEventNameGetRegion, send_message, [=](sio::message::list const& msg) {
        OnEmitAck(msg, [on_success, msg] {
          if (on_success == nullptr)
            return;
          sio::message::ptr region_ptr = msg.at(1);
          RTC_CHECK(region_ptr->get_flag() == sio::message::flag_object);
          RTC_CHECK(region_ptr->get_map()["region"] &&
                    region_ptr->get_map()["region"]->get_flag() ==
                        sio::message::flag_string);
          std::string region_id = region_ptr->get_map()["region"]->get_string();
          on_success(region_id);
        }, on_failure);
      });
}

void ConferenceSocketSignalingChannel::SetRegion(
      const std::string& stream_id,
      const std::string& region_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure){
  sio::message::ptr send_message = sio::object_message::create();
  send_message->get_map()["id"] = sio::string_message::create(stream_id);
  send_message->get_map()["region"] = sio::string_message::create(region_id);
  socket_client_->socket()->emit(kEventNameSetRegion, send_message,
                                 [=](sio::message::list const& msg) {
                                   OnEmitAck(msg, on_success, on_failure);
                                 });
}

void ConferenceSocketSignalingChannel::OnEmitAck(
    sio::message::list const& msg,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  sio::message::ptr ack = msg.at(0);
  if (ack->get_flag() != sio::message::flag_string) {
    LOG(LS_WARNING) << "The first element of emit ack is not a string.";
    if (on_failure) {
      std::unique_ptr<ConferenceException> e(
          new ConferenceException(ConferenceException::kUnkown,
                                  "Received unkown message from server."));
      on_failure(std::move(e));
    }
    return;
  }
  if (ack->get_string() == "success") {
    if (on_success != nullptr) {
      std::thread t(on_success);
      t.detach();
    }
  } else {
    LOG(LS_WARNING) << "Send message to MCU received negative ack.";
    if (msg.size() > 1) {
      sio::message::ptr error_ptr = msg.at(1);
      if (error_ptr->get_flag() == sio::message::flag_string) {
        LOG(LS_WARNING) << "Detail negative ack message: "
                        << error_ptr->get_string();
      }
    }
    if (on_failure != nullptr) {
      std::unique_ptr<ConferenceException> e(
          new ConferenceException(ConferenceException::kUnkown,
                                  "Negative acknowledgement from server."));
      on_failure(std::move(e));
    }
  }
}
}
}
