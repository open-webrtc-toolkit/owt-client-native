// Copyright (C) <2022> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "owt/p2p/p2pclient.h"
#include "talk/owt/sdk/p2p/tests/fake_signaling_channel.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/webrtc/api/task_queue/default_task_queue_factory.h"
#include "third_party/webrtc/rtc_base/checks.h"
#include "third_party/webrtc/rtc_base/logging.h"
#include "third_party/webrtc/test/run_loop.h"

namespace owt {
namespace p2p {
namespace test {

using namespace owt::p2p;

class P2PClientMockObserver : public owt::p2p::P2PClientObserver {
 public:
  MOCK_METHOD2(OnMessageReceived, void(const std::string&, const std::string));
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
}  // namespace test
}  // namespace p2p
}  // namespace owt