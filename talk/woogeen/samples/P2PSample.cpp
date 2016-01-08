// P2PSample.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include "woogeen/p2p/peerclient.h"
#include "woogeen/base/localcamerastreamparameters.h"
#include "woogeen/base/stream.h"
#include "p2psocketsignalingchannel.h"
#include "fileframegenerator.h"
#include "encodedframegenerator.h"

using namespace std;

int main(int argc, char** argv)
{
  using namespace woogeen::base;
  using namespace woogeen::p2p;
  std::shared_ptr<P2PSignalingChannelInterface> signaling_channel(new P2PSocketSignalingChannel());
  GlobalConfiguration::SetEncodedVideoFrameEnabled(true);
  PeerClientConfiguration configuration;
  configuration.media_codec.video_codec = MediaCodec::VideoCodec::VP8;
  //configuration.media_codec.video_codec = MediaCodec::VideoCodec::H264;
  std::shared_ptr<PeerClient> pc(new PeerClient(configuration, signaling_channel));
  cout << "Press Enter to connect peerserver." << std::endl;
  cin.ignore();
  /*std::string url = argv[1];
  std::string to = argv[2];*/
  std::string url = "http://10.239.3.197:8095";
  //std::string url = "http://192.168.23.227:8095";
  std::string to = "12";
  pc->Connect(url, nullptr, nullptr);
  cout << "Press Enter to invite remote user." << std::endl;
  cin.ignore();
  pc->Invite(to, nullptr, nullptr);
  cout << "Press Enter to publish local stream." << std::endl;
  cin.ignore();
  /*LocalCameraStreamParameters lcsp(LocalCameraStreamParameters(true, false));
  LocalCameraStream stream(std::make_shared<LocalCameraStreamParameters>(lcsp));
  std::shared_ptr<LocalCameraStream> shared_stream(std::make_shared<LocalCameraStream>(stream));*/

  LocalCustomizedStreamParameters lcsp(LocalCustomizedStreamParameters(true, false));
  //FileFrameGenerator* framer = new FileFrameGenerator(640, 480, 20);
  EncodedFrameGenerator* framer = new EncodedFrameGenerator(640, 480, 20);
  //LocalCameraStream stream(std::make_shared<LocalCameraStreamParameters>(lcsp));
  //std::shared_ptr<LocalCameraStream> shared_stream(std::make_shared<LocalCameraStream>(stream));
  LocalCustomizedStream stream(std::make_shared<LocalCustomizedStreamParameters>(lcsp), framer);
  std::shared_ptr<LocalCustomizedStream> shared_stream(std::make_shared<LocalCustomizedStream>(stream));

  pc->Publish(to, shared_stream, nullptr, nullptr);
  cout << "Press Enter to exit." << std::endl;
  cin.ignore();
  return 0;
}

