// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include "webrtc/rtc_base/task_queue.h"
#include "talk/owt/sdk/base/ios/networkmonitorios.h"
using namespace rtc;
namespace owt {
namespace base {
NetworkMonitorIos::NetworkMonitorIos() : reachability_(nullptr) {
  Init();
}
NetworkMonitorIos::~NetworkMonitorIos() {
  Stop();
  CFRelease(reachability_);
}
void NetworkMonitorIos::StartReachabilityNotifications() {
  RTC_DCHECK(reachability_);
  SCNetworkReachabilityContext reachability_context = {
      0,     // version
      this,  // user data
      NULL,  // retain
      NULL,  // release
      NULL   // description
  };
  if (SCNetworkReachabilitySetCallback(reachability_,
                                       &NetworkMonitorIos::ReachabilityCallback,
                                       &reachability_context)) {
    RTC_LOG(LS_INFO) << "SCNetworkReachabilitySetCallback";
  }
  if (!SCNetworkReachabilitySetCallback(
          reachability_, &NetworkMonitorIos::ReachabilityCallback,
          &reachability_context)) {
    RTC_LOG(LS_ERROR) << "Could not set network reachability callback";
    reachability_ = nullptr;
  } else if (!SCNetworkReachabilitySetDispatchQueue(
                 reachability_, dispatch_get_global_queue(
                                    DISPATCH_QUEUE_PRIORITY_DEFAULT, 0))) {
    RTC_LOG(LS_ERROR)
        << "Could not schedule network reachability on dispatch queue.";
    reachability_ = nullptr;
  }
  RTC_LOG(LS_INFO) << "StartReachabilityNotifications";
}
void NetworkMonitorIos::StopReachabilityNotifications() {
  RTC_DCHECK(reachability_);
  SCNetworkReachabilitySetDispatchQueue(reachability_, nullptr);
}
// static
void NetworkMonitorIos::ReachabilityCallback(SCNetworkReachabilityRef target,
                                             SCNetworkConnectionFlags flags,
                                             void* notifier) {
  RTC_LOG(LS_INFO) << "NetworkMonitorIos::ReachabilityCallback";
  NetworkMonitorIos* network_monitor_ios =
      static_cast<NetworkMonitorIos*>(notifier);
  network_monitor_ios->OnNetworksChanged();
}
rtc::AdapterType NetworkMonitorIos::GetAdapterType(
    const std::string& interface_name) {
  RTC_NOTREACHED();
  return rtc::AdapterType::ADAPTER_TYPE_UNKNOWN;
}
void NetworkMonitorIos::Init() {
  // Check reachability for 0.0.0.0.
  struct sockaddr_in addr;
  addr.sin_len = sizeof(addr);
  addr.sin_family = AF_INET;
  reachability_ = SCNetworkReachabilityCreateWithAddress(
      kCFAllocatorDefault, reinterpret_cast<struct sockaddr*>(&addr));
}
void NetworkMonitorIos::Start() {
  StartReachabilityNotifications();
}
void NetworkMonitorIos::Stop() {
  StopReachabilityNotifications();
}
rtc::NetworkMonitorInterface* NetworkMonitorFactoryIos::CreateNetworkMonitor() {
  return new NetworkMonitorIos();
}
}
}
