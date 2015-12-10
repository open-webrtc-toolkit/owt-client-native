// P2PSample.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include "talk/woogeen/sdk/p2p/peerclient.h"
#include "talk/woogeen/sdk/base/localcamerastreamparameters.h"
#include "talk/woogeen/sdk/base/stream.h"
#include "p2psocketsignalingchannel.h"
#include "fileframegenerator.h"

using namespace std;

int main(int argc, char** argv)
{
  using namespace woogeen;
  using namespace woogeensample;
  std::shared_ptr<P2PSignalingChannelInterface> signaling_channel(new P2PSocketSignalingChannel());
  PeerClientConfiguration configuration;
  configuration.encoded_video_frame_ = true;
  configuration.media_codec.video_codec = woogeen::MediaCodec::VideoCodec::H264;
  std::shared_ptr<PeerClient> pc(new PeerClient(configuration, signaling_channel));
  cout << "Press Enter to connect peerserver." << std::endl;
  cin.ignore();
  std::string url = argv[1];
  std::string to = argv[2];
  pc->Connect(url, nullptr, nullptr);
  cout << "Press Enter to invite remote user." << std::endl;
  cin.ignore();
  pc->Invite(to, nullptr, nullptr);
  cout << "Press Enter to publish local stream." << std::endl;
  cin.ignore();
  /*woogeen::LocalCameraStreamParameters lcsp(woogeen::LocalCameraStreamParameters(true, false));
  woogeen::LocalCameraStream stream(std::make_shared<woogeen::LocalCameraStreamParameters>(lcsp));
  std::shared_ptr<woogeen::LocalCameraStream> shared_stream(std::make_shared<woogeen::LocalCameraStream>(stream));*/

  woogeen::LocalCameraStreamParameters lcsp(woogeen::LocalCameraStreamParameters(true, false));
  FileFrameGenerator* framer = new FileFrameGenerator(640, 480, 20);
  woogeen::LocalRawStream stream(std::make_shared<woogeen::LocalCameraStreamParameters>(lcsp), framer);
  std::shared_ptr<woogeen::LocalRawStream> shared_stream(std::make_shared<woogeen::LocalRawStream>(stream));

  pc->Publish(to, shared_stream, nullptr, nullptr);
  cout << "Press Enter to exit." << std::endl;
  cin.ignore();
  return 0;
}

