/*
*
*Intel License
*
*/
#include <iostream>

#include "conferencesampleforwardobserver.h"

using namespace std;
using namespace woogeen::base;
using namespace woogeen::conference;

ConferenceSampleForwardObserver::ConferenceSampleForwardObserver(shared_ptr<ConferenceClient> client)
  :client_(client) {
}

void ConferenceSampleForwardObserver::OnStreamAdded(shared_ptr<RemoteCameraStream> stream) {
  remote_stream_ = stream;
  SubscribeOptions options;
  std::cout <<"Subscribe forward stream"<<endl;
  client_->Subscribe(remote_stream_,
                     options,
                     [&](std::shared_ptr<RemoteStream> stream){
                       std::cout <<"Subscribe succeed"<<endl;
                     },
                     [=](std::unique_ptr<ConferenceException>){
                       std::cout <<"Subscribe failed"<<endl;
                     });
}
