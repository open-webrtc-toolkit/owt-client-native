//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#include <iostream>
#include <thread>
#include <algorithm>
#include <ctime>
#if defined(WEBRTC_IOS)
#include <CoreFoundation/CFDate.h>
#endif
#include "talk/woogeen/sdk/base/sysinfo.h"
#include "talk/woogeen/sdk/conference/conferencesocketsignalingchannel.h"
#include "webrtc/base/base64.h"
#include "webrtc/base/checks.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/json.h"
#include "webrtc/base/timeutils.h"

namespace woogeen {
namespace conference {

#define SIGNALING_PROTOCOL_VERSION "1.0"
const std::string kEventNameCustomMessage = "customMessage";
const std::string kEventNameSignalingMessagePrelude = "signaling";
const std::string kEventNameSignalingMessage = "soac"; //only for soac message
const std::string kEventNameOnSignalingMessage = "progress";
const std::string kEventNameOnCustomMessage = "text";
const std::string kEventNameStreamControl = "stream-control";
const std::string kEventNameGetRegion = "get-region";
const std::string kEventNameSetRegion = "set-region";
const std::string kEventNameMute = "mute";
const std::string kEventNameUnmute = "unmute";
const std::string kEventNameMix = "mix";
const std::string kEventNameUnmix = "unmix";
const std::string kEventNameRefreshReconnectionTicket =
    "refreshReconnectionTicket";
const std::string kEventNameLogout = "logout";
const std::string kEventNameRelogin = "relogin";
const std::string kEventNameStreamMessage = "stream";
const std::string kEventNameOnAddStream = "add";
const std::string kEventNameOnRemoveStream = "remove";
const std::string kEventNameOnUpdateStream = "update";
const std::string kEventNameOnParticipant = "participant";
const std::string kNotificationNameUserJoin = "join";
const std::string kNotificationNameUserLeave = "leave";
const std::string kEventNameTextMessage = "text";
const std::string kEventNameOnStreamUpdate = "stream";
const std::string kEventNameOnUserJoin = "user_join";
const std::string kEventNameOnUserLeave = "user_leave";
const std::string kEventNameOnUserPresence = "participant";
const std::string kEventNameOnDrop = "drop";
const std::string kEventNameConnectionFailed = "connection_failed";

#if defined(WEBRTC_IOS)
// The epoch of Mach kernel is 2001/1/1 00:00:00, while Linux is 1970/1/1
// 00:00:00.
const uint64_t kMachLinuxTimeDelta = 978307200;
#endif

const int kReconnectionAttempts = 10;
const int kReconnectionDelay = 2000;

ConferenceSocketSignalingChannel::ConferenceSocketSignalingChannel()
    : socket_client_(new sio::client()),
      reconnection_ticket_(""),
      participant_id_(""),
      reconnection_attempted_(0),
      is_reconnection_(false),
      outgoing_message_id_(1) {}

ConferenceSocketSignalingChannel::~ConferenceSocketSignalingChannel() {
  delete socket_client_;
}

void ConferenceSocketSignalingChannel::AddObserver(
    ConferenceSocketSignalingChannelObserver& observer) {
  if (std::find(observers_.begin(), observers_.end(), &observer) !=
      observers_.end()) {
    LOG(LS_INFO) << "Adding duplicated observer.";
    return;
  }
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
  if (!rtc::Base64::IsBase64Encoded(token)) {
    if (on_failure != nullptr) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown, "Invalid token."));
      on_failure(std::move(e));
    }
    return;
  }
  std::string token_decoded("");
  if (!rtc::Base64::Decode(token, rtc::Base64::DO_STRICT, &token_decoded,
                           nullptr)) {
    if (on_failure) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown, "Invalid token."));
      // TODO: Use async instead.
      on_failure(std::move(e));
    }
    return;
  }
  reconnection_ticket_ = "";
  is_reconnection_ = false;
  Json::Value json_token;
  Json::Reader reader;
  if (!reader.parse(token_decoded, json_token)) {
    LOG(LS_ERROR) << "Parsing token failed.";
    if (on_failure != nullptr) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown, "Invalid token."));
      on_failure(std::move(e));
    }
    return;
  }
  std::string scheme("http://");
  std::string host;
  rtc::GetStringFromJsonObject(json_token, "host", &host);
  std::weak_ptr<ConferenceSocketSignalingChannel> weak_this =
      shared_from_this();
  socket_client_->socket();
  socket_client_->set_reconnect_attempts(kReconnectionAttempts);
  socket_client_->set_reconnect_delay(kReconnectionDelay);
  socket_client_->set_socket_close_listener(
      [weak_this](std::string const& nsp) {
        LOG(LS_INFO) << "Socket.IO disconnected.";
        auto that = weak_this.lock();
        if (that && (that->reconnection_attempted_ >= kReconnectionAttempts ||
                     that->disconnect_complete_)) {
          that->TriggerOnServerDisconnected();
        }
      });
  socket_client_->set_fail_listener([weak_this]() {
    LOG(LS_ERROR) << "Socket.IO connection failed.";
    auto that = weak_this.lock();
    if (that) {
      that->DropQueuedMessages();
      if (that->reconnection_attempted_ >= kReconnectionAttempts) {
        that->TriggerOnServerDisconnected();
      }
    }
  });
  socket_client_->set_reconnecting_listener([weak_this]() {
    LOG(LS_INFO) << "Socket.IO reconnecting.";
    auto that = weak_this.lock();
    if (that) {
      if (that->reconnection_ticket_ != "") {
        // It will be reset when a reconnection is success (open listener) or
        // fail (fail listener).
        that->is_reconnection_ = true;
        that->reconnection_attempted_++;
      }
    }
  });
  socket_client_->set_open_listener([=](void) {
    // At this time the connect failure callback is still in pending list. No
    // need to add a new entry in the pending list.
    if (!is_reconnection_) {
      woogeen::base::SysInfo sys_info(woogeen::base::SysInfo::GetInstance());
      sio::message::ptr login_message = sio::object_message::create();
      login_message->get_map()["token"] = sio::string_message::create(token);
      sio::message::ptr ua_message = sio::object_message::create();
      sio::message::ptr sdk_message = sio::object_message::create();
      sdk_message->get_map()["type"] =
          sio::string_message::create(sys_info.sdk.type);
      sdk_message->get_map()["version"] =
          sio::string_message::create(sys_info.sdk.version);
      ua_message->get_map()["sdk"] = sdk_message;
      sio::message::ptr os_message = sio::object_message::create();
      os_message->get_map()["name"] =
          sio::string_message::create(sys_info.os.name);
      os_message->get_map()["version"] =
          sio::string_message::create(sys_info.os.version);
      ua_message->get_map()["os"] = os_message;
      sio::message::ptr runtime_message = sio::object_message::create();
      runtime_message->get_map()["name"] =
          sio::string_message::create(sys_info.runtime.name);
      runtime_message->get_map()["version"] =
          sio::string_message::create(sys_info.runtime.version);
      ua_message->get_map()["runtime"] = runtime_message;
      login_message->get_map()["userAgent"] = ua_message;
      std::string protocol_version = SIGNALING_PROTOCOL_VERSION;
      login_message->get_map()["protocol"] = sio::string_message::create(protocol_version);
      Emit("login", login_message,
           [=](sio::message::list const& msg) {
             connect_failure_callback_ = nullptr;
             if (msg.size() < 2) {
               LOG(LS_ERROR) << "Received unknown message while sending token.";
               if (on_failure != nullptr) {
                 std::unique_ptr<ConferenceException> e(new ConferenceException(
                     ConferenceException::kUnknown,
                     "Received unknown message from server."));
                 on_failure(std::move(e));
               }
               return;
             }
             sio::message::ptr ack =
                 msg.at(0);  // The first element indicates the state.
             std::string state = ack->get_string();
             if (state == "error" || state == "timeout") {
               LOG(LS_ERROR) << "Server returns " << state
                             << " while joining a conference.";
               if (on_failure != nullptr) {
                 std::unique_ptr<ConferenceException> e(new ConferenceException(
                     ConferenceException::kUnknown,
                     "Received error message from server."));
                 on_failure(std::move(e));
               }
               return;
             }
             // in signaling protocol 1.0.0, the response contains following info:
             // {id: string(participantid),
             //  user: string(userid),
             //  role: string(participantrole),
             //  permission: object(permission),
             //  room: object(RoomInfo),
             //  reconnectonTicket: undefined or string(ReconnecionTicket).}
             // At present client SDK will only save reconnection ticket and participantid
             // and ignoring other info.
             sio::message::ptr message = msg.at(1);
             auto reconnection_ticket_ptr =
                 message->get_map()["reconnectionTicket"];
             if (reconnection_ticket_ptr) {
               OnReconnectionTicket(reconnection_ticket_ptr->get_string());
             }
             auto participant_id_ptr = message->get_map()["id"];
             if (participant_id_ptr) {
               participant_id_ = participant_id_ptr->get_string();
             }
             if (on_success != nullptr) {
               on_success(message);
             }
           },
           on_failure);
      is_reconnection_ = false;
      reconnection_attempted_ = 0;
    } else {
      sio::message::list reconnection_msg;
      sio::message::ptr relogin_request_name = sio::string_message::create(kEventNameRelogin);
      sio::message::ptr relogin_request_data = sio::object_message::create();
      relogin_request_data->get_map()["id"] = sio::string_message::create(participant_id_);
      relogin_request_data->get_map()["ticket"] = sio::string_message::create(reconnection_ticket_);
      socket_client_->socket()->emit(
          kEventNameSignalingMessagePrelude, reconnection_msg, [&](sio::message::list const& msg) {
            if (msg.size() < 2) {
              LOG(LS_WARNING)
                  << "Received unknown message while reconnection ticket.";
              reconnection_attempted_ = kReconnectionAttempts;
              socket_client_->close();
              return;
            }
            sio::message::ptr ack =
                msg.at(0);  // The first element indicates the state.
            std::string state = ack->get_string();
            if (state == "error" || state == "timeout") {
              LOG(LS_WARNING)
                  << "Server returns " << state
                  << " when relogin. Maybe an invalid reconnection ticket.";
              reconnection_attempted_ = kReconnectionAttempts;
              socket_client_->close();
              return;
            }
            // The second element is room info, please refer to MCU
            // erizoController's implementation for detailed message format.
            sio::message::ptr message = msg.at(1);
            if (message->get_flag() == sio::message::flag_string) {
              OnReconnectionTicket(message->get_string());
            }
            LOG(LS_VERBOSE) << "Reconnection success";
            DrainQueuedMessages();
          });
    }
  });
  socket_client_->socket()->on(
      kEventNameStreamMessage,
      sio::socket::event_listener_aux(
          [&](std::string const& name, sio::message::ptr const& data,
              bool is_ack, sio::message::list& ack_resp) {
            LOG(LS_VERBOSE) << "Received stream event.";
            if (data->get_map()["status"] != nullptr &&
                data->get_map()["status"]->get_flag() == sio::message::flag_string &&
                data->get_map()["id"] != nullptr &&
                data->get_map()["id"]->get_flag() == sio::message::flag_string) {
              std::string stream_status = data->get_map()["status"]->get_string();
              std::string stream_id = data->get_map()["id"]->get_string();
              if (stream_status == "add") {
                auto stream_info = data->get_map()["data"];
                if (stream_info != nullptr && stream_info->get_flag() == sio::message::flag_object) {
                  for (auto it = observers_.begin(); it != observers_.end(); ++it) {
                    (*it)->OnStreamAdded(stream_info);
                  }
                }
              } else if (stream_status == "update") {
                sio::message::ptr update_message = sio::object_message::create();
                update_message->get_map()["id"] = sio::string_message::create(stream_id);
                auto stream_update = data->get_map()["data"];
                if (stream_update != nullptr) {
                  update_message->get_map()["event"] = stream_update;
                }
                for (auto it = observers_.begin(); it != observers_.end(); ++it) {
                  (*it)->OnStreamUpdated(update_message);
                }
              } else if (stream_status == "remove") {
                sio::message::ptr remove_message = sio::object_message::create();
                remove_message->get_map()["id"] = sio::string_message::create(stream_id);
                for (auto it = observers_.begin(); it != observers_.end(); ++it) {
                  (*it)->OnStreamRemoved(remove_message);
                }
              }
            }
          }));
  socket_client_->socket()->on(
      kEventNameOnCustomMessage,
      sio::socket::event_listener_aux(
          [&](std::string const& name, sio::message::ptr const& data,
              bool is_ack, sio::message::list& ack_resp) {
            LOG(LS_VERBOSE) << "Received custom message.";
            std::string from = data->get_map()["from"]->get_string();
            std::string message = data->get_map()["message"]->get_string();
            for (auto it = observers_.begin(); it != observers_.end(); ++it) {
              (*it)->OnCustomMessage(from, message);
            }
          }));
  socket_client_->socket()->on(
      kEventNameOnUserPresence,
      sio::socket::event_listener_aux([&](
          std::string const& name, sio::message::ptr const& data, bool is_ack,
          sio::message::list& ack_resp) {
        LOG(LS_VERBOSE) << "Received user join/leave message.";
        if (data == nullptr || data->get_flag() != sio::message::flag_object ||
            data->get_map()["action"] == nullptr ||
            data->get_map()["action"]->get_flag() != sio::message::flag_string) {
          RTC_DCHECK(false);
          return;
        }
        auto participant_action = data->get_map()["action"]->get_string();
        if (participant_action == "join") {
          // Get the pariticipant ID from data;
          auto participant_info = data->get_map()["data"];
          if (participant_info != nullptr && participant_info->get_flag() == sio::message::flag_object
              && participant_info->get_map()["id"] != nullptr
              && participant_info->get_map()["id"]->get_flag() == sio::message::flag_string) {
            for (auto it = observers_.begin(); it != observers_.end(); ++it) {
              (*it)->OnUserJoined(participant_info);
            }
          }
        } else if (participant_action == "leave") {
          auto participant_info = data->get_map()["data"];
          if (participant_info != nullptr && participant_info->get_flag() == sio::message::flag_string) {
            for (auto it = observers_.begin(); it != observers_.end(); ++it) {
              (*it)->OnUserLeft(participant_info);
            }
          }
        } else {
          RTC_NOTREACHED();
        }
      }));
  socket_client_->socket()->on(
      kEventNameOnSignalingMessage,
      sio::socket::event_listener_aux(
          [&](std::string const& name, sio::message::ptr const& data,
              bool is_ack, sio::message::list& ack_resp) {
            LOG(LS_VERBOSE) << "Received signaling message from erizo.";
            for (auto it = observers_.begin(); it != observers_.end(); ++it) {
              (*it)->OnSignalingMessage(data);
            }
          }));
  socket_client_->socket()->on(
      kEventNameOnDrop,
      sio::socket::event_listener_aux(
          [&](std::string const& name, sio::message::ptr const& data,
              bool is_ack, sio::message::list& ack_resp) {
            LOG(LS_INFO) << "Received drop message.";
            socket_client_->set_reconnect_attempts(0);
            for (auto it = observers_.begin(); it != observers_.end(); ++it) {
              (*it)->OnServerDisconnected();
            }
          }));
  socket_client_->socket()->on(
      kEventNameConnectionFailed,
      sio::socket::event_listener_aux(
          [&](std::string const& name, sio::message::ptr const& data,
              bool is_ack, sio::message::list& ack_resp) {
            for (auto it = observers_.begin(); it != observers_.end(); ++it) {
              (*it)->OnStreamError(data);
            }
          }));
  // Store |on_failure| so it can be invoked if connect failed.
  connect_failure_callback_ = on_failure;
  socket_client_->connect(scheme.append(host));
}

void ConferenceSocketSignalingChannel::Disconnect(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!socket_client_->opened() && reconnection_attempted_ == 0 && on_failure) {
    // Socket.IO is not connected and not reconnecting.
    std::unique_ptr<ConferenceException> e(new ConferenceException(
        ConferenceException::kUnknown, "Socket.IO is not connected."));
    on_failure(std::move(e));
    // TODO: A corner case is execute Disconnect before first reconnection
    // attempt. So we don't return, still try to close socket.
  }
  reconnection_attempted_ = kReconnectionAttempts;
  disconnect_complete_ = on_success;
  if (socket_client_->opened()) {
    // Clear all pending failure callbacks after successful disconnect, don't check resp.
    socket_client_->socket()->emit(kEventNameLogout, nullptr,
                                   [=](sio::message::list const& msg) {
                                     DropQueuedMessages();
                                     socket_client_->close();
                                   });
  }
}

void ConferenceSocketSignalingChannel::SendInitializationMessage(
    sio::message::ptr options,
    std::string publish_stream_label,
    std::string subscribe_stream_label,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  sio::message::list message_list;
  message_list.push(options);
  std::string event_name;
  if (publish_stream_label != "")
    event_name = "publish";
  else if (subscribe_stream_label != "")
    event_name = "subscribe";
  Emit(event_name, message_list,
       [=](sio::message::list const& msg) {
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
                 ConferenceException::kUnknown,
                 "Received unkown message from server."));
             on_failure(std::move(e));
           }
           return;
         }
         if (message->get_string() == "ok") {
           if (msg.at(1)->get_flag() != sio::message::flag_object) {
             RTC_DCHECK(false);
             return;
           }
           std::string stream_id = msg.at(1)->get_map()["id"]->get_string();
           if (event_name == "subscribe") {
             // replace stream's id with session ID.
               for (auto it = observers_.begin(); it != observers_.end(); ++it) {
                 (*it)->OnRemoteStreamId(stream_id, subscribe_stream_label);
               }
             on_success();
             return;
           }

           for (auto it = observers_.begin(); it != observers_.end(); ++it) {
             // sessionId is not a 1:1 mapping to stream. push it
             (*it)->OnStreamId(stream_id, publish_stream_label);
           }
           on_success();
           return;
         } else if (message->get_string() == "error" && msg.at(1) != nullptr &&
                    msg.at(1)->get_flag() == sio::message::flag_string) {
           if (on_failure) {
             std::unique_ptr<ConferenceException> e(new ConferenceException(
                 ConferenceException::kUnknown, msg.at(1)->get_string()));
             on_failure(std::move(e));
           }
         } else {
           if (on_failure) {
             std::unique_ptr<ConferenceException> e(new ConferenceException(
                 ConferenceException::kUnknown,
                 "Ack for initializing message is not expected."));
             on_failure(std::move(e));
           }
           return;
         }
       },
       on_failure);
}

void ConferenceSocketSignalingChannel::SendSdp(
    sio::message::ptr message,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  std::weak_ptr<ConferenceSocketSignalingChannel> weak_this =
      shared_from_this();
  sio::message::list message_list(message);
  // Add a null message for |to_to_deprecated|. Don't know its meaning.
  message_list.push(sio::null_message::create());
  Emit(kEventNameSignalingMessage, message_list,
       [weak_this, on_success, on_failure](sio::message::list const& msg) {
         if (auto that = weak_this.lock()) {
           that->OnEmitAck(msg, on_success, on_failure);
         }
       },
       on_failure);
}

void ConferenceSocketSignalingChannel::SendStreamEvent(
    const std::string& event,
    const std::string& stream_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  sio::message::ptr message = sio::object_message::create();
  message->get_map()["id"] = sio::string_message::create(stream_id);
  std::weak_ptr<ConferenceSocketSignalingChannel> weak_this =
      shared_from_this();
  Emit(event, message,
       [weak_this, on_success, on_failure](sio::message::list const& msg) {
         if (auto that = weak_this.lock()) {
           that->OnEmitAck(msg, on_success, on_failure);
         }
       },
       on_failure);
}

void ConferenceSocketSignalingChannel::SendCustomMessage(
    const std::string& message,
    const std::string& receiver,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  sio::message::ptr send_message = sio::object_message::create();
  if (receiver == "") {
    send_message->get_map()["to"] = sio::string_message::create("all");
  } else {
    send_message->get_map()["to"] = sio::string_message::create(receiver);
  }
  send_message->get_map()["message"] = sio::string_message::create(message);
  std::weak_ptr<ConferenceSocketSignalingChannel> weak_this =
      shared_from_this();
  Emit(kEventNameTextMessage, send_message,
       [weak_this, on_success, on_failure](sio::message::list const& msg) {
         if (auto that = weak_this.lock()) {
           that->OnEmitAck(msg, on_success, on_failure);
         }
       },
       on_failure);
}

void ConferenceSocketSignalingChannel::SendStreamControlMessage(
    const std::string& stream_id,
    const std::string& action,
    const std::string& operation,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  sio::message::ptr payload = sio::object_message::create();
  payload->get_map()["id"] = sio::string_message::create(stream_id);
  payload->get_map()["operation"] = sio::string_message::create(operation);
  // Currently only pause/play will be processed here.
  if (operation == "pause" || operation == "play"
      || operation == "mix" || operation == "unmix") {
    //TODO(jianlin): Combine mute/unmute API with this.
    payload->get_map()["data"] = sio::string_message::create(action);
  }

  std::weak_ptr<ConferenceSocketSignalingChannel> weak_this =
      shared_from_this();
  Emit(kEventNameStreamControl, payload,
       [weak_this, on_success, on_failure](sio::message::list const& msg) {
         if (auto that = weak_this.lock()) {
           that->OnEmitAck(msg, on_success, on_failure);
         }
       },
       on_failure);
}

void ConferenceSocketSignalingChannel::GetRegion(
    const std::string& stream_id,
    const std::string& mixed_stream_id,
    std::function<void(std::string)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  sio::message::ptr send_message = sio::object_message::create();
  send_message->get_map()["id"] = sio::string_message::create(stream_id);
  send_message->get_map()["operation"] = sio::string_message::create("get-region");
  send_message->get_map()["data"] =
      sio::string_message::create(mixed_stream_id);
  std::weak_ptr<ConferenceSocketSignalingChannel> weak_this =
      shared_from_this();
  Emit(kEventNameStreamControl, send_message,
       [weak_this, on_success, on_failure](sio::message::list const& msg) {
         if (auto that = weak_this.lock()) {
           that->OnEmitAck(
               msg,
               [on_success, msg] {
                 if (on_success == nullptr)
                   return;
                 sio::message::ptr region_ptr = msg.at(1);
                 RTC_CHECK(region_ptr->get_flag() == sio::message::flag_object);
                 RTC_CHECK(region_ptr->get_map()["region"] &&
                           region_ptr->get_map()["region"]->get_flag() ==
                               sio::message::flag_string);
                 std::string region_id =
                     region_ptr->get_map()["region"]->get_string();
                 on_success(region_id);
               },
               on_failure);
         }
       },
       on_failure);
}

void ConferenceSocketSignalingChannel::SetRegion(
    const std::string& stream_id,
    const std::string& mixed_stream_id,
    const std::string& region_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  sio::message::ptr send_message = sio::object_message::create();
  send_message->get_map()["id"] = sio::string_message::create(stream_id);
  send_message->get_map()["operation"] = sio::string_message::create("set-region");
  sio::message::ptr region_setting = sio::object_message::create();
  region_setting->get_map()["view"] = sio::string_message::create(mixed_stream_id);
  region_setting->get_map()["region"] = sio::string_message::create(region_id);
  send_message->get_map()["data"] = region_setting;
  std::weak_ptr<ConferenceSocketSignalingChannel> weak_this =
      shared_from_this();
  Emit(kEventNameStreamControl, send_message,
       [weak_this, on_success, on_failure](sio::message::list const& msg) {
         if (auto that = weak_this.lock()) {
           that->OnEmitAck(msg, on_success, on_failure);
         }
       },
       on_failure);
}

void ConferenceSocketSignalingChannel::Mute(
    const std::string& stream_id,
    bool mute_audio,
    bool mute_video,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!mute_video && !mute_audio) {
    LOG(LS_INFO) << "Neither video nor audio will be muted.";
    if (on_failure) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown,
          "Mute without either video or audio flag set."));
      on_failure(std::move(e));
    }
    return;
  }
  sio::message::ptr send_message = sio::object_message::create();
  send_message->get_map()["id"] = sio::string_message::create(stream_id);
  send_message->get_map()["operation"] = sio::string_message::create("pause");
  std::string track_type =
      (mute_video && mute_audio) ? "av" : (!mute_video ? "audio" : "video");
  send_message->get_map()["data"] = sio::string_message::create(track_type);
  std::weak_ptr<ConferenceSocketSignalingChannel> weak_this =
      shared_from_this();
  Emit(kEventNameStreamControl, send_message,
       [weak_this, on_success, on_failure](sio::message::list const& msg) {
         if (auto that = weak_this.lock()) {
           that->OnEmitAck(msg, on_success, on_failure);
         }
       },
       on_failure);
}

void ConferenceSocketSignalingChannel::Unmute(
    const std::string& stream_id,
    bool unmute_audio,
    bool unmute_video,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!unmute_video && !unmute_audio) {
    LOG(LS_INFO) << "Neither video nor audio will be unmuted.";
    if (on_failure) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown,
          "Unmute without either video or audio flag set."));
      on_failure(std::move(e));
    }
    return;
  }
  sio::message::ptr send_message = sio::object_message::create();
  send_message->get_map()["id"] = sio::string_message::create(stream_id);
  send_message->get_map()["operation"] = sio::string_message::create("play");
  std::string track_type = (unmute_video && unmute_audio)
                               ? "av"
                               : (!unmute_video ? "audio" : "video");
  send_message->get_map()["data"] = sio::string_message::create(track_type);
  std::weak_ptr<ConferenceSocketSignalingChannel> weak_this =
      shared_from_this();
  Emit(kEventNameStreamControl, send_message,
       [weak_this, on_success, on_failure](sio::message::list const& msg) {
         if (auto that = weak_this.lock()) {
           that->OnEmitAck(msg, on_success, on_failure);
         }
       },
       on_failure);
}

void ConferenceSocketSignalingChannel::Mix(
    const std::string& stream_id,
    std::vector<std::string>& mixed_stream_ids,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  sio::message::ptr send_message = sio::object_message::create();
  send_message->get_map()["id"] = sio::string_message::create(stream_id);
  send_message->get_map()["operation"] = sio::string_message::create("mix");
  for (auto mixed_stream_id : mixed_stream_ids) {
    send_message->get_map()["data"] =
        sio::string_message::create(mixed_stream_id);
    break;
  }
  std::weak_ptr<ConferenceSocketSignalingChannel> weak_this =
      shared_from_this();
  Emit(kEventNameStreamControl, send_message,
       [weak_this, on_success, on_failure](sio::message::list const& msg) {
         if (auto that = weak_this.lock()) {
           that->OnEmitAck(msg, on_success, on_failure);
         }
       },
       on_failure);
}

void ConferenceSocketSignalingChannel::Unmix(
    const std::string& stream_id,
    std::vector<std::string>& mixed_stream_ids,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  sio::message::ptr send_message = sio::object_message::create();
  send_message->get_map()["id"] = sio::string_message::create(stream_id);
  send_message->get_map()["operation"] = sio::string_message::create("unmix");
  for (auto mixed_stream_id : mixed_stream_ids) {
    send_message->get_map()["data"] =
        sio::string_message::create(mixed_stream_id);
    break;
  }
  std::weak_ptr<ConferenceSocketSignalingChannel> weak_this =
      shared_from_this();
  Emit(kEventNameStreamControl, send_message,
       [weak_this, on_success, on_failure](sio::message::list const& msg) {
         if (auto that = weak_this.lock()) {
           that->OnEmitAck(msg, on_success, on_failure);
         }
       },
       on_failure);
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
          new ConferenceException(ConferenceException::kUnknown,
                                  "Received unkown message from server."));
      on_failure(std::move(e));
    }
    return;
  }
  if (ack->get_string() == "success" || ack->get_string() == "ok") {
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
          new ConferenceException(ConferenceException::kUnknown,
                                  "Negative acknowledgement from server."));
      on_failure(std::move(e));
    }
  }
}

void ConferenceSocketSignalingChannel::OnReconnectionTicket(
    const std::string& ticket) {
  LOG(LS_VERBOSE) << "On reconnection ticket: " << ticket;
  reconnection_ticket_ = ticket;
  uint64_t now(0);
// rtc::TimeMillis() seems not work well on iOS. It returns half of the actual
// value on iPhone 6.
#if defined(WEBRTC_IOS)
  now = CFAbsoluteTimeGetCurrent();
  now += kMachLinuxTimeDelta;
  now *= 1000;
#else
  now = rtc::TimeMillis();
#endif
  std::string ticket_decoded(
      rtc::Base64::Decode(reconnection_ticket_, rtc::Base64::DO_STRICT));
  Json::Value ticket_json;
  Json::Reader ticket_reader;
  if (!ticket_reader.parse(ticket_decoded, ticket_json)) {
    RTC_NOTREACHED();
    LOG(LS_WARNING) << "Cannot parse reconnection ticket.";
    return;
  }
  Json::Value expiration_json;
  std::string expiration_str;
  if (rtc::GetValueFromJsonObject(ticket_json, "notAfter", &expiration_json)) {
    expiration_str = expiration_json.asString();
    auto expiration_time = std::stoll(expiration_str);
    int delay(expiration_time - now);
    if (delay < 0) {
      LOG(LS_WARNING)
          << "Reconnection ticket expiration time is earlier than now.";
      delay = 5 * 60 * 1000;  // Set delay to 5 mins.
    }
    LOG(LS_VERBOSE) << "Reconnection ticket will expire in: " << delay / 1000
                    << "seconds";
    std::weak_ptr<ConferenceSocketSignalingChannel> weak_this =
        shared_from_this();
    std::thread([weak_this, delay]() {
      auto that = weak_this.lock();
      if (!that) {
        return;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(delay));
      that->RefreshReconnectionTicket();
    }).detach();
  }
}

void ConferenceSocketSignalingChannel::RefreshReconnectionTicket() {
  socket_client_->socket()->emit(
      kEventNameRefreshReconnectionTicket, nullptr,
      [=](sio::message::list const& ack) {
        if (ack.size() != 2) {
          RTC_NOTREACHED();
          LOG(LS_WARNING) << "Wired ack for refreshing reconnection ticket.";
          return;
        }
        std::string state = ack.at(0)->get_string();
        std::string message = ack.at(1)->get_string();
        if (state != "success") {
          LOG(LS_WARNING) << "Refresh reconnection ticket failed. Error: "
                          << message;
          return;
        };
        OnReconnectionTicket(message);
      });
}

void ConferenceSocketSignalingChannel::TriggerOnServerDisconnected() {
  if (disconnect_complete_) {
    disconnect_complete_();
  }
  disconnect_complete_ = nullptr;
  for (auto it = observers_.begin(); it != observers_.end(); ++it) {
    (*it)->OnServerDisconnected();
  }
}

void ConferenceSocketSignalingChannel::Emit(
    const std::string& name,
    const sio::message::list& message,
    const std::function<void(sio::message::list const&)> ack,
    const std::function<void(std::unique_ptr<ConferenceException>)>
        on_failure) {
  int message_id(0);
  {
    std::lock_guard<std::mutex> lock(outgoing_message_mutex_);
    message_id = outgoing_message_id_++;
  }
#if 0
  std::string sio_name = "signaling";
  sio::message::ptr request_name = sio::string_message::create(name);
  sio::message::list new_message(message);
  new_message.insert(0, request_name);
#endif
  // SioMessage sio_message(message_id, sio_name, new_message, ack, on_failure);
  SioMessage sio_message(message_id, name, message, ack, on_failure);
  outgoing_messages_.push(sio_message);
  std::weak_ptr<ConferenceSocketSignalingChannel> weak_this =
      shared_from_this();
  socket_client_->socket()->emit(
      name, message, [weak_this, message_id](sio::message::list const& msg) {
        LOG(LS_INFO) << "Received ack for message ID: " << message_id;
        auto that = weak_this.lock();
        if (!that) {
          LOG(LS_WARNING) << "Signaling channel was destroyed before ack.";
          return;
        }
        std::function<void(sio::message::list const&)> callback(nullptr);
        {
          std::lock_guard<std::mutex> lock(that->outgoing_message_mutex_);
          if (that->outgoing_messages_.empty()) {
            RTC_NOTREACHED();
            return;
          }
          /*RTC_DCHECK_EQ(message_id, that->outgoing_messages_.front().id)
              << "Unordered Socket.IO message.";*/
          while (that->outgoing_messages_.front().id < message_id) {
            LOG(LS_WARNING) << "Potential unordered Socket.IO message.";
            that->outgoing_messages_.pop();
          }
          if (that->outgoing_messages_.front().id > message_id) {
            LOG(LS_ERROR) << "Original message for " << message_id
                          << " is not found.";
          }
          callback = that->outgoing_messages_.front().ack;
          that->outgoing_messages_.pop();
        }
        if (callback) {
          callback(msg);
        }
      });
}

void ConferenceSocketSignalingChannel::DropQueuedMessages() {
  // TODO(jianjunz): Trigger on_failure in another thread. In current
  // implementation, failure callback MUST NOT acquire
  // |outgoing_message_mutex_|. Otherwise, deadlock may happen.
  std::lock_guard<std::mutex> lock(outgoing_message_mutex_);
  while (!outgoing_messages_.empty()) {
    if (outgoing_messages_.front().on_failure != nullptr) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown, "Failed to delivery message."));
      outgoing_messages_.front().on_failure(std::move(e));
    }
    outgoing_messages_.pop();
  }
}

void ConferenceSocketSignalingChannel::DrainQueuedMessages() {
  std::queue<SioMessage> temp_queue;
  {
    std::lock_guard<std::mutex> lock(outgoing_message_mutex_);
    std::swap(temp_queue, outgoing_messages_);
  }
  LOG(LS_INFO) << "outgoing_messages_ number after swap: "
               << outgoing_messages_.size();
  while (!temp_queue.empty()) {
    auto sio_message = temp_queue.front();
    // MUST release |outgoing_message_mutex_| before Emit because Emit acquires
    // mutex.
    Emit(sio_message.name, sio_message.message, sio_message.ack,
         sio_message.on_failure);
    temp_queue.pop();
  }
}
}
}
