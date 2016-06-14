//
//Intel License
//
// conferencesample.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <chrono>
#include <ctime>
#include <unistd.h>

#include "talk/woogeen/include/asio_token.h"
#include "woogeen/base/localcamerastreamparameters.h"
#include "woogeen/base/stream.h"
#include "woogeen/conference/conferenceclient.h"
#include "conferenceobserver.h"
#ifdef USE_FILE_SOURCE
#include "fileframegenerator.h"
#endif

using namespace std;

/*//Callbacks for the room join API
function<void(shared_ptr<User>)>join_room_success {
}

function<void(unique_ptr<ConferenceException>)>join_room_failure {
}*/

int main(int argc, char** argv)
{
  daemon(1,1);
  using namespace woogeen::base;
  using namespace woogeen::conference;
  GlobalConfiguration::SetEncodedVideoFrameEnabled(false);

  ConferenceClientConfiguration configuration;
  IceServer ice;
  ice.urls.push_back("stun:61.152.239.56");
  ice.username = "";
  ice.password = "";
  vector<IceServer> ice_servers;
  ice_servers.push_back(ice);
  configuration.ice_servers = ice_servers;
  configuration.media_codec.video_codec = MediaCodec::VideoCodec::VP8;
  //configuration.media_codec.audio_codec = MediaCodec::AudioCodec::ISAC;
  shared_ptr<ConferenceClient> room(new ConferenceClient(configuration));
  ConferenceSampleObserver* observer = new ConferenceSampleObserver(room);
  room->AddObserver(*observer);


  string scheme("http://");
  string suffix("/createToken");
  if (argc >= 2) {
    string hosturl(argv[1]);
    scheme.append(hosturl);
  } else {
    string fixed("10.239.44.59:3001");
    scheme.append(fixed);
  }
  scheme.append(suffix);

  string roomId("");
  if(argc == 3){
    string roomStr(argv[2]);
    roomId.append(roomStr);
  }

  string token = getToken(scheme, roomId);

#ifdef USE_FILE_SOURCE
  FileFrameGenerator* framer = new FileFrameGenerator(640, 480, 20);
  shared_ptr<LocalCustomizedStreamParameters> lcsp(new LocalCustomizedStreamParameters(true, true));
  shared_ptr<LocalStream> shared_stream(new LocalCustomizedStream(lcsp, framer));
/* //LocalCustomizedStream sample code
  const string audiopath = "./audio_long16.pcm";
  const string videopath = "./source.video";
  unique_ptr<AudioFrameGeneratorInterface> audio_generator(FileAudioFrameGenerator::Create(audiopath));
  if (audio_generator == nullptr){
    return -1;
  }
  GlobalConfiguration::SetEncodedVideoFrameEnabled(true);
  GlobalConfiguration::SetCustomizedAudioInputEnabled(true, move(audio_generator));
  LocalCustomizedStreamParameters lcsp(LocalCustomizedStreamParameters(true, true));
  EncodedFrameGenerator* framer = new EncodedFrameGenerator(videopath, 640, 480, 20, EncodedMimeType::ENCODED_VP8);
  LocalCustomizedStream stream(make_shared<LocalCustomizedStreamParameters>(lcsp), framer);
  shared_ptr<LocalCustomizedStream> shared_stream(make_shared<LocalCustomizedStream>(stream));
*/
#else
  shared_ptr<LocalCameraStreamParameters> lscp(new LocalCameraStreamParameters(true, true));
  lscp->Resolution(640, 480);
  shared_ptr<LocalStream> shared_stream(new LocalCameraStream(*lscp));
#endif

  if (token != "") {
      room->Join(token,
        [=](shared_ptr<User> user) {
          cout << "Join succeeded!" << endl;
          room->Publish(shared_stream,
            [=]{
                 cout <<" ============Publish succeeded==========" << endl;
               },
            [=](unique_ptr<ConferenceException> err) {
                 cout <<" ============Publish failed===========" << endl;
               }
          );
        },
        [=](unique_ptr<ConferenceException> err) {
          cout << "Join failed!" << endl;
        }
      );
  } else
    cout << "Create token error!" << endl;

  int interval = 0;
  while(1) {
    sleep(1);
    ++interval;
    if (interval%5 == 0) {
      room->GetConnectionStats(shared_stream,
                              [&](std::shared_ptr<ConnectionStats> stats) {
                                time_t last = chrono::system_clock::to_time_t(stats->time_stamp);
                                cout << "GetConnectionStats Generated at time of "<< std::ctime(&last) << endl;
                                cout <<"\n available send bandwidth: " << stats->video_bandwidth_stats.available_send_bandwidth
                                     <<"\n available receive bandwidth: " << stats->video_bandwidth_stats.available_receive_bandwidth
                                     <<"\n transmit bitrate: " << stats->video_bandwidth_stats.transmit_bitrate
                                     <<"\n retransmit_bitrate: " << stats->video_bandwidth_stats.retransmit_bitrate << endl;
                              },
                              [=](unique_ptr<ConferenceException>) {
                                cout << "GetConnectionStats failed" << endl;
                              });
    }
  }

  if(observer) {
    delete observer;
    observer = nullptr;
  }

  return 0;
}
