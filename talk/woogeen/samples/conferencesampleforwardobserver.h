/*
*
*Intel License
*
*/

#ifndef CONFERENCE_SAMPLE_FORWARD_OBSERVER_H_
#define CONFERENCE_SAMPLE_FORWARD_OBSERVER_H_

#include "woogeen/base/stream.h"
#include "woogeen/conference/conferenceclient.h"

class ConferenceSampleForwardObserver
	: public woogeen::conference::ConferenceClientObserver {
 public:
  ConferenceSampleForwardObserver(std::shared_ptr<woogeen::conference::ConferenceClient> client);
  virtual ~ConferenceSampleForwardObserver(){}
  void OnStreamAdded(std::shared_ptr<woogeen::base::RemoteCameraStream> stream) override;

 private:
  std::shared_ptr<woogeen::base::RemoteStream> remote_stream_;
  std::shared_ptr<woogeen::conference::ConferenceClient> client_;
};

#endif
