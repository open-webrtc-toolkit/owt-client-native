// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OMS_CONFERENCE_REMOTEMIXEDSTREAM_H_
#define OMS_CONFERENCE_REMOTEMIXEDSTREAM_H_
#include "oms/base/commontypes.h"
#include "oms/base/stream.h"
namespace oms {
namespace conference {
/// Observer class for remote mixed stream.
class RemoteMixedStreamObserver : public oms::base::StreamObserver {
 public:
  virtual void OnVideoLayoutChanged(){};
};
/// This class represent a mixed remote stream.
class RemoteMixedStream : public oms::base::RemoteStream {
 public:
  /** @cond **/
  RemoteMixedStream(const std::string& id,
                    const std::string& from,
                    const std::string& viewport,
                    const oms::base::SubscriptionCapabilities& subscription_capabilities,
                    const oms::base::PublicationSettings& publication_settings);
  /** @endcond **/
  /// Add an observer for conferenc client.
  void AddObserver(RemoteMixedStreamObserver& observer);
  /// Remove an object from conference client.
  void RemoveObserver(RemoteMixedStreamObserver& observer);
  /**
    @brief Returns an attribute of mixed streams which distinguishes them from
    other mixed streams a conference room provides.
    @details A conference room, since Intel CS for WebRTC v3.4 and later, has
    been extended to support multiple presentations of the mixed audio and video
    for variant purposes. For example, in remote education scenario, the teacher
    and students may subscribe different mixed streams with view of 'teacher'
    and 'student' respectively in the same class conference room. It is also the
    label of a mixed stream indicating its peculiarity with a meaningful
    string-typed value, which must be unique within a room.
  */
  std::string Viewport();
 protected:
  virtual void OnVideoLayoutChanged();
 private:
  const std::string viewport_;
  std::vector<std::reference_wrapper<RemoteMixedStreamObserver>> observers_;
  friend class oms::conference::ConferenceClient;
};
}
}
#endif  // OMS_CONFERENCE_REMOTEMIXEDSTREAM_H_
