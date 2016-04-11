//
//Intel License
//
// testclient.cc : Defines the entry point for the console application.
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
  std::string audiopath("");
  std::string videopath("");
  std::string roomId("");
  std::string scheme("http://");
  std::string suffix("/createToken");
  ConferenceClientConfiguration configuration;
  EncodedMimeType type;
  int width;
  int height;
  int fps;

  if(argc >= 2){
    std::string hosturl(argv[1]);
    scheme.append(hosturl);
  }else{
    std::string fixed("10.239.44.59:3001");
    scheme.append(fixed);
  }

  if(argc >= 3){
    std::string roomStr(argv[2]);
    roomId.append(roomStr);
  }

  if(argc >= 4){
    std::string codec(argv[3]);
    if (codec.find("h264") != std::string::npos) {
      configuration.media_codec.video_codec = MediaCodec::VideoCodec::H264;
      type = EncodedMimeType::ENCODED_H264;
    }else{
      configuration.media_codec.video_codec = MediaCodec::VideoCodec::VP8;
      type = EncodedMimeType::ENCODED_VP8;
    }
  }else{
    configuration.media_codec.video_codec = MediaCodec::VideoCodec::VP8;
    type = EncodedMimeType::ENCODED_VP8;
  }

  if(argc >= 5){
    std::string res(argv[4]);
    if (res.find("1080") != std::string::npos) {
      width = 1920;
      height = 1080;
    }else if (res.find("720") != std::string::npos){
      width = 1280;
      height = 720;
    }else if (res.find("vga") != std::string::npos){
      width = 640;
      height = 480;
    }else{
      width = 320;
      height = 240;
    }
  }else{
    width = 640;
    height = 480;
  }

   if(argc >= 6){
    fps = atoi(argv[5]);
  }else{
    fps = 20;
  }

  if(argc >= 7){
    std::string path(argv[6]);
    audiopath.append(path);
    audiopath.append("/audio_long16.pcm");
    videopath.append(path);
    videopath.append("/source.video");
  }else{
    audiopath.append("./audio_long16.pcm");
    videopath.append("./source.video");
  }

  std::unique_ptr<woogeen::base::AudioFrameGeneratorInterface> audio_generator(FileAudioFrameGenerator::Create(audiopath));
  if (audio_generator == nullptr){
    return -1;
  }
  GlobalConfiguration::SetEncodedVideoFrameEnabled(true);
  GlobalConfiguration::SetCustomizedAudioInputEnabled(true, std::move(audio_generator));

  IceServer ice;
  ice.urls.push_back("stun:61.152.239.56");
  ice.username = "";
  ice.password = "";
  vector<IceServer> ice_servers;
  ice_servers.push_back(ice);
  configuration.ice_servers = ice_servers;

  scheme.append(suffix);
  std::shared_ptr<ConferenceClient> room(new ConferenceClient(configuration));

  ConferenceSampleObserver *observer = new ConferenceSampleObserver(room);
  room->AddObserver(*observer);
//  cout << "Press Enter to connect room." << std::endl;
 // cin.ignore();

  string token = getToken(scheme, roomId);
  LocalCustomizedStreamParameters lcsp(LocalCustomizedStreamParameters(true, true));
  //FileFrameGenerator* framer = new FileFrameGenerator(640, 480, 20);
  EncodedFrameGenerator* framer = new EncodedFrameGenerator(videopath, width, height, fps, type);
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

