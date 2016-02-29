// P2PSample.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include "woogeen/p2p/peerclient.h"
#include "woogeen/base/localcamerastreamparameters.h"
#include "woogeen/base/stream.h"
#include "p2psocketsignalingchannel.h"
#include "fileframegenerator.h"
#include "encodedframegenerator.h"
#include "directframegenerator.h"


using namespace std;

int main(int argc, char** argv)
{
  using namespace woogeen::base;
  using namespace woogeen::p2p;
  std::shared_ptr<P2PSignalingChannelInterface> signaling_channel(new P2PSocketSignalingChannel());
  //GlobalConfiguration::SetEncodedVideoFrameEnabled(true);
  GlobalConfiguration::SetEncodedVideoFrameEnabled(false);
  PeerClientConfiguration configuration;
  configuration.media_codec.video_codec = MediaCodec::VideoCodec::VP8;
  //configuration.media_codec.video_codec = MediaCodec::VideoCodec::H264;
  std::shared_ptr<PeerClient> pc(new PeerClient(configuration, signaling_channel));
  cout << "Press Enter to connect peerserver." << std::endl;
  cin.ignore();
  /*std::string url = argv[1];
  std::string to = argv[2];*/
  std::string url = "http://10.239.196.5:8095";
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
#if 1
  DirectFrameGenerator::Options options;
  options.width = 1280;
  options.height = 720;
  options.fps = 20;
  options.useLocal = false;
  options.type = FrameGeneratorInterface::VideoFrameCodec::H264;
  options.url = "rtsp://admin:admin@10.239.10.113:554/cam/realmonitor?channel=1&subtype=0";
  //options.url = "rtsp://10.239.10.23/rtsp_tunnel?h26x=4&line=1&inst=1";
  std::shared_ptr<DirectFrameGenerator> generator;
  generator.reset(new DirectFrameGenerator(options));
  std::shared_ptr<LocalCustomizedStreamParameters> lcsp;
  lcsp.reset(new LocalCustomizedStreamParameters(true, true));
  std::shared_ptr<LocalStream> shared_stream;
  shared_stream.reset(new LocalCustomizedStream(lcsp, generator.get()));
#else
  FileFrameGenerator* framer = new FileFrameGenerator(640, 480, 20);
  //EncodedFrameGenerator* framer = new EncodedFrameGenerator(1920, 1080, 20, ENCODED_H264);
  std::shared_ptr<LocalCustomizedStreamParameters> lcsp;
  lcsp.reset(new LocalCustomizedStreamParameters(true, true));
  std::shared_ptr<LocalStream> shared_stream;
  shared_stream.reset(new LocalCustomizedStream(lcsp, framer));
#endif
  pc->Publish(to, shared_stream, nullptr, nullptr);
  cout << "Press Enter to exit." << std::endl;
  cin.ignore();
  return 0;
}

