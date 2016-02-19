//
//Intel License
//
// P2PSample.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include "woogeen/conference/conferenceclient.h"
#include "woogeen/base/localcamerastreamparameters.h"
#include "woogeen/base/stream.h"
#include "p2psocketsignalingchannel.h"
#include "fileframegenerator.h"
#include "encodedframegenerator.h"
#include "fileaudioframegenerator.h"
#include "talk/woogeen/include/asio_token.h"
#include "conferenceobserver.h"
#include <unistd.h>

using namespace std;

/*std::function<void(std::shared_ptr<User>)>join_room_success {

}

std::function<void(std::unique_ptr<ConferenceException>)>join_room_failure {

}*/

int main(int argc, char** argv)
{
  daemon(1,1);
  using namespace woogeen::base;
  using namespace woogeen::conference;
  const std::string path = "./audio_long16.pcm";
  std::unique_ptr<woogeen::base::AudioFrameGeneratorInterface> audio_generator(FileAudioFrameGenerator::Create(path));
  if (audio_generator == nullptr){
    return -1;
  }
  GlobalConfiguration::SetEncodedVideoFrameEnabled(true);
  GlobalConfiguration::SetEncodedAudioFrameEnabled(true, std::move(audio_generator));
  ConferenceClientConfiguration configuration;
  IceServer ice;
  ice.urls.push_back("stun:61.152.239.56");
  ice.username = "";
  ice.password = "";
  vector<IceServer> ice_servers;
  ice_servers.push_back(ice);
  configuration.ice_servers = ice_servers;
  configuration.media_codec.video_codec = MediaCodec::VideoCodec::VP8;
  //configuration.media_codec.video_codec = MediaCodec::VideoCodec::H264;
  std::string scheme("http://");
  std::string suffix("/createToken");
  if(argc >= 2){
    std::string hosturl(argv[1]);
    scheme.append(hosturl);
  }else{
    std::string fixed("10.239.44.59:3001");
    scheme.append(fixed);
  }
  scheme.append(suffix);
  std::shared_ptr<ConferenceClient> room(new ConferenceClient(configuration));

  ConferenceSampleObserver *observer = new ConferenceSampleObserver(room);
  room->AddObserver(*observer);
//  cout << "Press Enter to connect room." << std::endl;
 // cin.ignore();
  std::string roomId("");
  if(argc == 3){
    std::string roomStr(argv[2]);
    roomId.append(roomStr);
  }
  string token = getToken(scheme, roomId);
  LocalCustomizedStreamParameters lcsp(LocalCustomizedStreamParameters(true, true));
  //FileFrameGenerator* framer = new FileFrameGenerator(640, 480, 20);
  EncodedFrameGenerator* framer = new EncodedFrameGenerator(640, 480, 20);
  //LocalCameraStream stream(std::make_shared<LocalCameraStreamParameters>(lcsp));
  //std::shared_ptr<LocalCameraStream> shared_stream(std::make_shared<LocalCameraStream>(stream));
  LocalCustomizedStream stream(std::make_shared<LocalCustomizedStreamParameters>(lcsp), framer);
  std::shared_ptr<LocalCustomizedStream> shared_stream(std::make_shared<LocalCustomizedStream>(stream));

  if (token != "") {
      room->Join(token,
          [=](std::shared_ptr<User> user) {
              room->Publish(shared_stream,
              [=] {
                      cout <<" ============Publish succeed==========";
                  },
              [=](std::unique_ptr<ConferenceException> err) {
                  cout <<" ============Publish failed===========";
              });

              cout << "Join succeeded!" << endl;
                    },
                    [=](std::unique_ptr<ConferenceException> err) {
                      cout << "Join failed!" << endl;
                    });
  } else {
     cout << "create token error!" << endl;
  }

  while(1){
    sleep(1);
  }
//  cout << "Press Enter to publish local stream." << std::endl;
//  cin.ignore();
/*
  std::shared_ptr<woogeen::base::LocalCameraStreamParameters> lcsp;
  lcsp.reset(new LocalCameraStreamParameters(true, true));
  lcsp->Resolution(640, 480);
  std::shared_ptr<LocalCameraStream> shared_stream;
  shared_stream.reset(new LocalCameraStream(*lcsp));
  //LocalCameraStreamParameters lcsp(LocalCameraStreamParameters(true, false));
//  lcsp.Resolution(640, 480);
  //LocalCameraStream stream(std::make_shared<LocalCameraStreamParameters>(lcsp));
  //std::shared_ptr<LocalCameraStream> shared_stream(std::make_shared<LocalCameraStream>(stream));
*/

 // cout << "Press Enter to exit." << std::endl;
  //cin.ignore();
  if(observer) {
    delete observer;
    observer = nullptr;
  }
  return 0;
}

