//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/woogeen/sdk/conference/objc/public/RTCConferenceUser.h"

#include "talk/woogeen/sdk/conference/conferenceuser.h"

/*
// This class represent a user's permission
@interface RTCConferencePermission (Internal)

-(instancetype)initWithNativePermission:(std::shared_ptr<const
woogeen::conference::Permission>)permission;
-(void)setNativePermission:(std::shared_ptr<const
woogeen::conference::Permission>)permission;
-(std::shared_ptr<const woogeen::conference::Permission>)nativePermission;

@end
*/

/// This class represent an attendee in a conference.
@interface RTCConferenceUser (Internal)

- (instancetype)initWithNativeUser:
    (std::shared_ptr<const woogeen::conference::User>)user;
- (void)setNativeUser:(std::shared_ptr<const woogeen::conference::User>)user;
- (std::shared_ptr<const woogeen::conference::User>)nativeUser;

@end
