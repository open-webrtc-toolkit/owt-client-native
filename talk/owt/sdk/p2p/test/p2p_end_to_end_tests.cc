// Copyright (C) <2022> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "test/gmock.h"
#include "test/gtest.h"

#include <memory>
#include "talk/owt/sdk/include/cpp/owt/base/logging.h"
#include "talk/owt/sdk/include/cpp/owt/p2p/p2pclient.h"
#include "talk/owt/sdk/p2p/test/simple_signaling_channel.h"
#include "third_party/webrtc/rtc_base/logging.h"
#include "third_party/webrtc/test/run_loop.h"

static const std::string kClient1Id = "client1";
static const std::string kClient2Id = "client2";

namespace owt {
namespace p2p {
namespace test {
class P2PEndToEndTest : public testing::Test {
 public:
  P2PEndToEndTest() {
    Logging::Severity(LoggingSeverity::kVerbose);
    P2PClientConfiguration configuration;
    // TODO: Duplicated client ID in SimpleSignalingChannel ctor and Connect.
    std::shared_ptr<P2PSignalingChannelInterface> sc1 =
        std::make_shared<SimpleSignalingChannel>(kClient1Id,
                                                 &signaling_server_);
    p1_ = std::make_unique<P2PClient>(configuration, sc1);
    std::shared_ptr<P2PSignalingChannelInterface> sc2 =
        std::make_shared<SimpleSignalingChannel>(kClient2Id,
                                                 &signaling_server_);
    p2_ = std::make_unique<P2PClient>(configuration, sc2);
  }

 protected:
  webrtc::test::RunLoop run_loop_;
  SimpleSignalingServer signaling_server_;
  std::shared_ptr<P2PClient> p1_;
  std::shared_ptr<P2PClient> p2_;
};

TEST_F(P2PEndToEndTest, ConnectAndGetStats) {
  p1_->Connect(
      "", kClient1Id, [](const std::string&) {}, nullptr);
  p2_->Connect(
      "", kClient2Id, [](const std::string&) {}, nullptr);
  p1_->AddAllowedRemoteId(kClient2Id);
  p2_->AddAllowedRemoteId(kClient1Id);
  p1_->Send(kClient2Id, "message", nullptr, [](std::unique_ptr<Exception> e) {
    RTC_LOG(INFO) << "Failed to send a message. " << e->Message();
  });
  RTC_LOG(LS_VERBOSE) << "Sent.";
  for (size_t i = 0; i < 20000000; i++) {
    p1_->GetConnectionStats(
        kClient2Id,
        [&](std::shared_ptr<owt::base::RTCStatsReport> report) {
          for (const auto& stat_rec : *report) {
            if (stat_rec.type == owt::base::RTCStatsType::kCandidatePair) {
              auto& stat =
                  stat_rec.cast_to<owt::base::RTCIceCandidatePairStats>();
              if (stat.nominated) {
                RTC_LOG(LS_INFO) << "Available outgoing bitrate: "
                                 << stat.available_outgoing_bitrate;
              }
            }
            if (i == 19999999) {
              run_loop_.Quit();
            }
          }
        },
        [](std::unique_ptr<Exception> e) {
          RTC_LOG(INFO) << "Failed to get stats. " << e->Message();
        });
  }
  run_loop_.Run();
}

}  // namespace test
}  // namespace p2p
}  // namespace owt