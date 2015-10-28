//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef conference_ConferenceSocketSignalingChannel_h
#define conference_ConferenceSocketSignalingChannel_h

#include <memory>
#include <unordered_map>
#include "talk/woogeen/include/sio_client.h"
#include "talk/woogeen/include/sio_message.h"
#include "talk/woogeen/sdk/conference/conferenceuser.h"
#include "talk/woogeen/sdk/conference/conferenceexception.h"

namespace woogeen {

class ConferenceSocketSignalingChannelObserver {
 public:
  virtual void OnStreamAdded(sio::message::ptr stream) = 0;
  virtual void OnUserJoined(
      std::shared_ptr<const woogeen::conference::User> user) = 0;
  virtual void OnUserLeft(
      std::shared_ptr<const woogeen::conference::User> user) = 0;
  virtual void OnStreamRemoved(sio::message::ptr stream) = 0;
  virtual void OnServerDisconnected() = 0;
  virtual void OnCustomMessage(std::string& from, std::string& message) = 0;
  virtual void OnSignalingMessage(sio::message::ptr message) = 0;
  // Notify the ID for a published stream.
  virtual void OnStreamId(const std::string& id, const std::string& label) = 0;
};

class ConferenceSocketSignalingChannel {
 public:
  explicit ConferenceSocketSignalingChannel();
  ~ConferenceSocketSignalingChannel();

  virtual void AddObserver(ConferenceSocketSignalingChannelObserver& observer);
  virtual void RemoveObserver(
      ConferenceSocketSignalingChannelObserver& observer);
  virtual void Connect(
      const std::string& token,
      std::function<void(sio::message::ptr room_info)> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  // Send publish or subscribe message to MCU.
  // If it publishes a stream, label should be MediaStream's label.
  // If it subscribe a stream, label should be nullptr.
  virtual void SendInitializationMessage(
      sio::message::ptr options,
      std::string publish_stream_label,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  virtual void SendSdp(
      sio::message::ptr message,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  virtual void SendStreamEvent(
      const std::string& event,
      const std::string& stream_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  virtual void SendCustomMessage(
      const std::string& message,
      const std::string& receiver,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  virtual void SendStreamControlMessage(
      const std::string& stream_id,
      const std::string& action,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  virtual void Disconnect(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

 protected:
  virtual void OnEmitAck(
      sio::message::list const& msg,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

 private:
  sio::client* socket_client_;
  std::vector<ConferenceSocketSignalingChannelObserver*> observers_;
};
}

#endif
