/*
*
*Intel License
*
*/
#include <iostream>

#include "woogeen/conference/remotemixedstream.h"
#include "conferencesampleobserver.h"

using namespace std;
using namespace woogeen::base;
using namespace woogeen::conference;

ConferenceSampleObserver::ConferenceSampleObserver(shared_ptr<ConferenceClient> client)
  :client_(client) {
}

void ConferenceSampleObserver::OnStreamAdded(shared_ptr<RemoteMixedStream> stream) {
  remote_stream_ = stream;
  std::vector<VideoFormat> formats = stream->SupportedVideoFormats();
  SubscribeOptions options;
  for(auto it=formats.begin(); it!=formats.end(); it++){
    options.resolution.width = (*it).resolution.width;
    options.resolution.height = (*it).resolution.height;
    break;
  }
  client_->Subscribe(remote_stream_,
                     options,
                     [&](std::shared_ptr<RemoteStream> stream){
                       std::cout <<"Subscribe succeed"<<endl;
                     },
                     [=](std::unique_ptr<ConferenceException>){
                       std::cout <<"Subscribe failed"<<endl;
                     });
}
