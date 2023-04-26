// Copyright (C) <2022> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "owt/base/videorendererinterface.h"
#include "talk/owt/sdk/base/peerconnectiondependencyfactory.h"
#include "owt/p2p/p2pclient.h"
#include "talk/owt/sdk/p2p/tests/fake_signaling_channel.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/webrtc/api/task_queue/default_task_queue_factory.h"
#include "third_party/webrtc/api/video/i420_buffer.h"
#include "third_party/webrtc/pc/test/frame_generator_capturer_video_track_source.h"
#include "third_party/webrtc/rtc_base/checks.h"
#include "third_party/webrtc/rtc_base/logging.h"
#include "third_party/webrtc/test/run_loop.h"
#include "third_party/webrtc/test/testsupport/frame_writer.h"

namespace owt {
namespace p2p {
namespace test {

using namespace owt::p2p;

class P2PClientMockObserver : public owt::p2p::P2PClientObserver {
 public:
  MOCK_METHOD2(OnMessageReceived, void(const std::string&, const std::string));
  MOCK_METHOD1(OnStreamAdded,
               void(std::shared_ptr<owt::base::RemoteStream> stream));
#ifdef OWT_CG_SERVER
  MOCK_METHOD1(OnPeerConnectionClosed, void(const std::string&));
#endif
};

// A sink dumps video frames in a given interval.
class VideoDumpSink : public owt::base::VideoRendererInterface {
 public:
  // If `runloop` is not nullptr, it'll quit after receving `quit_after` frames.
  VideoDumpSink(const std::string& output_filename,
                uint32_t interval,
                webrtc::test::RunLoop* runloop,
                uint32_t quit_after)
      : output_filename_(output_filename),
        interval_(interval),
        to_be_skipped_(0),
        runloop_(runloop),
        max_frames_(quit_after),
        frame_received_(0) {}
  void RenderFrame(std::unique_ptr<VideoBuffer> buffer) {
    if (to_be_skipped_ == 0) {
      webrtc::test::JpegFrameWriter frame_writer(output_filename_);
      rtc::scoped_refptr<webrtc::I420Buffer> image_buffer =
          webrtc::I420Buffer::Copy(
              buffer->resolution.width, buffer->resolution.height,
              buffer->buffer, buffer->resolution.width,
              buffer->buffer +
                  buffer->resolution.width * buffer->resolution.height,
              buffer->resolution.width / 2,
              buffer->buffer +
                  (buffer->resolution.width * buffer->resolution.height) / 4 *
                      5,
              buffer->resolution.width / 2);
      frame_writer.WriteFrame(
          webrtc::VideoFrame(
              image_buffer, webrtc::VideoRotation::kVideoRotation_0,
              webrtc::Clock::GetRealTimeClock()->TimeInMilliseconds()),
          100);
      to_be_skipped_ = interval_;
    } else {
      to_be_skipped_ += 1;
    }
    if (runloop_ && ++frame_received_ >= max_frames_) {
      RTC_LOG(LS_INFO)<<"Quit the runloop.";
      runloop_->Quit();
    }
  }
  // Render type that indicates the VideoBufferType the renderer would receive.
  VideoRendererType Type() override { return VideoRendererType::kI420; }

 private:
  const std::string output_filename_;
  uint32_t interval_;
  uint32_t to_be_skipped_;
  webrtc::test::RunLoop* runloop_;
  uint32_t max_frames_;
  uint32_t frame_received_;
};

class StreamDumpObserver : public owt::p2p::P2PClientObserver {
 public:
  StreamDumpObserver(VideoDumpSink* video_sink) : video_sink_(video_sink) {
    RTC_CHECK(video_sink);
  }
  void OnStreamAdded(std::shared_ptr<owt::base::RemoteStream> stream) override {
    RTC_LOG(LS_INFO) << "OnStreamAdded";
    stream->AttachVideoRenderer(*video_sink_);
  }

 private:
  VideoDumpSink* video_sink_;
};

class EndToEndTest : public ::testing::Test {
 public:
  EndToEndTest()
      : task_queue_factory_(webrtc::CreateDefaultTaskQueueFactory()),
        task_queue_(task_queue_factory_->CreateTaskQueue(
            "owt_e2e_test",
            webrtc::TaskQueueFactory::Priority::NORMAL)),
        signaling_channel_(std::make_shared<FakeSignalingChannel>(
            task_queue_factory_->CreateTaskQueue(
                "fake_signaling_channel",
                webrtc::TaskQueueFactory::Priority::NORMAL))),
        client1_(nullptr),
        client2_(nullptr) {
    rtc::LogMessage::SetLogToStderr(true);
    task_queue_->PostTask([this] {
      P2PClientConfiguration configuration;
      client1_ = std::make_shared<P2PClient>(configuration, signaling_channel_);
      client2_ = std::make_shared<P2PClient>(configuration, signaling_channel_);
      client1_->AddObserver(observer1_);
      client2_->AddObserver(observer2_);
      client1_->Connect("", "client1", nullptr,
                        [](std::unique_ptr<owt::base::Exception>) {
                          RTC_DCHECK_NOTREACHED();
                        });
      client2_->Connect("", "client2", nullptr,
                        [](std::unique_ptr<owt::base::Exception>) {
                          RTC_DCHECK_NOTREACHED();
                        });
      client1_->AddAllowedRemoteId("client2");
      client2_->AddAllowedRemoteId("client1");
    });
  }

 protected:
  std::unique_ptr<webrtc::TaskQueueFactory> task_queue_factory_;
  std::unique_ptr<webrtc::TaskQueueBase, webrtc::TaskQueueDeleter> task_queue_;
  std::shared_ptr<FakeSignalingChannel> signaling_channel_;
  std::shared_ptr<P2PClient> client1_;
  std::shared_ptr<P2PClient> client2_;
  P2PClientMockObserver observer1_;
  P2PClientMockObserver observer2_;
  webrtc::test::RunLoop loop_;
};

TEST_F(EndToEndTest, SendMessgeCanBeReceived) {
  task_queue_->PostTask([this] {
    client1_->Send("client2", "message", nullptr, nullptr);
    EXPECT_CALL(observer2_, OnMessageReceived("client1", testing::_))
        .WillOnce(testing::InvokeWithoutArgs([this] { loop_.Quit(); }));
  });
  loop_.Run();
}

rtc::scoped_refptr<MediaStreamInterface> CreateFakeMediaStream() {
  auto* pcdf = owt::base::PeerConnectionDependencyFactory::Get();
  rtc::scoped_refptr<MediaStreamInterface> media_stream =
      pcdf->CreateLocalMediaStream("fake-media-stream");
  auto source_config = webrtc::FrameGeneratorCapturerVideoTrackSource::Config();
  source_config.num_squares_generated = 3;
  rtc::scoped_refptr<webrtc::FrameGeneratorCapturerVideoTrackSource>
      video_track_source =
          pcdf->SignalingThreadForTesting()->BlockingCall([&source_config] {
            rtc::scoped_refptr<webrtc::FrameGeneratorCapturerVideoTrackSource>
                source = rtc::make_ref_counted<
                    webrtc::FrameGeneratorCapturerVideoTrackSource>(
                    source_config, webrtc::Clock::GetRealTimeClock(), false);
            source->Start();
            return source;
          });
  rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track =
      pcdf->CreateLocalVideoTrack("fake-video-track", video_track_source.get());
  media_stream->AddTrack(video_track);
  return media_stream;
}

TEST_F(EndToEndTest, VideoCall) {
  std::shared_ptr<owt::base::LocalStream> stream =
      std::make_shared<owt::base::LocalStream>(CreateFakeMediaStream().get(),
                                               StreamSourceInfo());
  VideoDumpSink sink("video_dump.jpg", 10, &loop_, 30);
  StreamDumpObserver observer(&sink);
  client2_->AddObserver(observer);

  task_queue_->PostTask([this, stream] {
    client1_->Publish("client2", stream, nullptr, nullptr);
  });
  loop_.Run();
}

TEST_F(EndToEndTest, OnPeerConnectionClosed) {
  task_queue_->PostTask([this] {
    client1_->Send("client2", "message", nullptr, nullptr);
    EXPECT_CALL(observer2_, OnMessageReceived("client1", testing::_))
        .WillOnce(testing::InvokeWithoutArgs([this] { loop_.Quit(); }));
  });
  loop_.Run();
  task_queue_->PostTask([this] {
    client1_->Stop("client2", nullptr, nullptr);
    EXPECT_CALL(observer2_, OnPeerConnectionClosed("client1"))
        .WillOnce(testing::InvokeWithoutArgs([this] { loop_.Quit(); }));
  });
  loop_.Run();
}
}  // namespace test
}  // namespace p2p
}  // namespace owt