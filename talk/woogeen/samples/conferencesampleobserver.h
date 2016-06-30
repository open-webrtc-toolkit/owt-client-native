/*
*
*Intel License
*
*/

#ifndef CONFERENCE_SAMPLE_OBSERVER_H_
#define CONFERENCE_SAMPLE_OBSERVER_H_

#include "woogeen/base/stream.h"
#include "woogeen/conference/conferenceclient.h"

class ConferenceSampleObserver
	: public woogeen::conference::ConferenceClientObserver {
 public:
  ConferenceSampleObserver(std::shared_ptr<woogeen::conference::ConferenceClient> client);
  virtual ~ConferenceSampleObserver(){}
  void OnStreamAdded(std::shared_ptr<woogeen::conference::RemoteMixedStream> stream) override;

 private:
  std::shared_ptr<woogeen::base::RemoteStream> remote_stream_;
  std::shared_ptr<woogeen::conference::ConferenceClient> client_;
};

#endif
