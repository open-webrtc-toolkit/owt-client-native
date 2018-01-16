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

#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
#import "ICS/ICSNetwork.h"

RTC_EXPORT
@interface ICSMediaChannelStats : NSObject

/// Synchronization source, defined in RTC3550
@property(nonatomic, readonly) NSString* ssrc;
/// Codec name.
@property(nonatomic, readonly) NSString* codecName;
/// Represents the track.id property.
@property(nonatomic, readonly) NSString* trackIdentifier;

@end

RTC_EXPORT
@interface ICSAudioSenderStats : ICSMediaChannelStats

/// Total number of bytes sent for this SSRC.
@property(nonatomic, readonly) NSUInteger bytesSent;
/// Total number of RTP packets sent for this SSRC.
@property(nonatomic, readonly) NSUInteger packetsSent;
/// Total number of RTP packets lost for this SSRC.
@property(nonatomic, readonly) NSUInteger packetsLost;
/// Estimated round trip time (milliseconds) for this SSRC based on the RTCP
/// timestamp.
@property(nonatomic, readonly) NSUInteger roundTripTime;

@end

RTC_EXPORT
@interface ICSAudioReceiverStats : ICSMediaChannelStats

/// Total number of bytes received for this SSRC.
@property(nonatomic, readonly) NSUInteger bytesReceived;
/// Total number of RTP packets received for this SSRC.
@property(nonatomic, readonly) NSUInteger packetsReceived;
/// Total number of RTP packets lost for this SSRC.
@property(nonatomic, readonly) NSUInteger packetsLost;
/// Audio delay estimated with unit of millisecond.
@property(nonatomic, readonly) NSUInteger estimatedDelay;

@end

RTC_EXPORT
@interface ICSVideoSenderStats : ICSMediaChannelStats

/// Total number of bytes sent for this SSRC.
@property(nonatomic, readonly) NSUInteger bytesSent;
/// Total number of RTP packets sent for this SSRC.
@property(nonatomic, readonly) NSUInteger packetsSent;
/// Total number of RTP packets lost for this SSRC.
@property(nonatomic, readonly) NSUInteger packetsLost;
/// Count the total number of Full Intra Request (FIR) packets received by the
/// sender. This metric is only valid for video and is sent by receiver.
@property(nonatomic, readonly) NSUInteger firCount;
/// Count the total number of Packet Loss Indication (PLI) packets received by
/// the sender and is sent by receiver.
@property(nonatomic, readonly) NSUInteger pliCount;
/// Count the total number of Negative ACKnowledgement (NACK) packets received
/// by the sender and is sent by receiver.
@property(nonatomic, readonly) NSUInteger nackCount;
/// Video frame resolution sent.
@property(nonatomic, readonly) CGSize frameResolution;
/// Video frame rate sent.
@property(nonatomic, readonly) NSUInteger frameRate;
/// Video adapt reason.
@property(nonatomic, readonly) NSUInteger adaptChanges;
/// Estimated round trip time (milliseconds) for this SSRC based on the RTCP
/// timestamp.
@property(nonatomic, readonly) NSUInteger roundTripTime;

@end

RTC_EXPORT
@interface ICSVideoReceiverStats : ICSMediaChannelStats

/// Total number of bytes received for this SSRC.
@property(nonatomic, readonly) NSUInteger bytesReceived;
/// Total number of RTP packets received for this SSRC.
@property(nonatomic, readonly) NSUInteger packetsReceived;
/// Total number of RTP packets lost for this SSRC.
@property(nonatomic, readonly) NSUInteger packetsLost;
/// Count the total number of Full Intra Request (FIR) packets received by the
/// sender. This metric is only valid for video and is sent by receiver.
@property(nonatomic, readonly) NSUInteger firCount;
/// Count the total number of Packet Loss Indication (PLI) packets received by
/// the sender and is sent by receiver.
@property(nonatomic, readonly) NSUInteger pliCount;
/// Count the total number of Negative ACKnowledgement (NACK) packets received
/// by the sender and is sent by receiver.
@property(nonatomic, readonly) NSUInteger nackCount;
/// Video frame resolution received.
@property(nonatomic, readonly) CGSize frameResolution;
/// Video frame rate received.
@property(nonatomic, readonly) NSUInteger frameRateReceived;
/// Video frame rate output.
@property(nonatomic, readonly) NSUInteger frameRateOutput;
/// Current video delay with unit of millisecond
@property(nonatomic, readonly) NSUInteger delay;

@end

RTC_EXPORT
/// Video bandwidth statistics
@interface ICSVideoBandwidthStats : NSObject

/// Available video bandwidth for sending. Unit: bps.
@property(nonatomic, readonly) NSUInteger availableSendBandwidth;
/// Available video bandwidth for receiving. Unit: bps.
@property(nonatomic, readonly) NSUInteger availableReceiveBandwidth;
/// Video bitrate of transmit. Unit: bps.
@property(nonatomic, readonly) NSUInteger transmitBitrate;
/// Video bitrate of retransmit. Unit: bps.
@property(nonatomic, readonly) NSUInteger retransmitBitrate;
/// Target encoding bitrate, unit: bps.
@property(nonatomic, readonly) NSUInteger targetEncodingBitrate;
/// Actual encoding bitrate, unit: bps.
@property(nonatomic, readonly) NSUInteger actualEncodingBitrate;

@end

RTC_EXPORT
/// Define ICE candidate report.
@interface ICSIceCandidateStats : NSObject
/// The ID of this stats report.
@property(nonatomic, readonly) NSString* statsId;
/// The IP address of the candidate.
@property(nonatomic, readonly) NSString* ip;
/// The port number of the candidate.
@property(nonatomic, readonly) NSUInteger port;
/// Candidate type.
@property(nonatomic, readonly) RTCIceCandidateType candidateType;
/// Transport protocol.
@property(nonatomic, readonly) RTCTransportProtocolType protocol;
/// Calculated as defined in RFC5245.
@property(nonatomic, readonly) NSUInteger priority;

@end

RTC_EXPORT
/// Define ICE candidate pair report.
@interface ICSIceCandidatePairStats : NSObject
/// The ID of this stats report.
@property(nonatomic, readonly) NSString* statsId;
/// Indicate whether transport is active.
@property(nonatomic, readonly) BOOL isActive;
/// Local candidate of this pair.
@property(nonatomic, readonly) ICSIceCandidateStats* localIceCandidate;
/// Remote candidate of this pair.
@property(nonatomic, readonly) ICSIceCandidateStats* remoteIceCandidate;

@end

/// Connection statistics
RTC_EXPORT
@interface ICSConnectionStats : NSObject

/// Time stamp of connection statistics generation.
@property(nonatomic, readonly) NSDate* timeStamp;
/// Reports for media channels. Element can be one of the following types:
/// ICSAudioSenderStats, ICSVideoSenderStats, ICSAudioReceiverStats,
/// ICSVideoReceiverStats.
@property(nonatomic, readonly) NSArray<ICSMediaChannelStats*>* mediaChannelStats;
/// Video bandwidth statistics.
@property(nonatomic, readonly) ICSVideoBandwidthStats* videoBandwidthStats;
/// Reports for local ICE candidate stats.
@property(nonatomic, readonly) NSArray<ICSIceCandidateStats*>* localIceCandidateStats;
/// Reports for remote ICE candidate stats.
@property(nonatomic, readonly) NSArray<ICSIceCandidateStats*>* remoteIceCandidateStats;
/// Reports for ICE candidate pair stats.
@property(nonatomic, readonly) NSArray<ICSIceCandidatePairStats*>* iceCandidatePairStats;

@end
