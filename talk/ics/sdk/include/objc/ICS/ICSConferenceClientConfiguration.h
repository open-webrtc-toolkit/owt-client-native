/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ICS_CONFERENCE_OBJC_ICSCONFERENCECLIENTCONFIGURATION_H_
#define ICS_CONFERENCE_OBJC_ICSCONFERENCECLIENTCONFIGURATION_H_

#import <Foundation/Foundation.h>
#import <WebRTC/RTCConfiguration.h>

/// Configuration for creating a ICSConferenceClient
/**
  This configuration is used while creating ICSConferenceClient. Changing this
  configuration does NOT impact ICSConferenceClient already created.
*/
RTC_EXPORT
@interface ICSConferenceClientConfiguration : NSObject

@property(nonatomic, strong, readwrite) NSArray* ICEServers;
/**
 @brief Max outgoing audio bandwidth, unit: kbps.
 @details Please be noticed different codecs may support different bitrate
 ranges. If you set a bandwidth limitation which is not supported by selected
 codec, connection will fail.
 */
@property(nonatomic, readwrite) NSInteger maxAudioBandwidth;
/**
 @brief Max outgoing video bandwidth, unit: kbps.
 @details Please be noticed different codecs may support different bitrate
 ranges. If you set a bandwidth limitation which is not supported by selected
 codec, connection will fail.
 */
@property(nonatomic, readwrite) NSInteger maxVideoBandwidth;

/**
 @brief Candidate collection policy.
 @details If you do not want cellular network when WiFi is available, please use
 RTCCandidateNetworkPolicyLowCost. Using low cost policy may not have good
 network experience. Default policy is collecting all candidates.
 */
@property(nonatomic, assign, readwrite)
    RTCCandidateNetworkPolicy candidateNetworkPolicy;

@end

#endif  // ICS_CONFERENCE_OBJC_ICSCONFERENCECLIENTCONFIGURATION_H_
