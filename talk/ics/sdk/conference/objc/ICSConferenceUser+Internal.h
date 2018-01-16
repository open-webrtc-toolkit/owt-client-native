//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/ICS/ICSConferenceUser.h"

#include "talk/ics/sdk/include/cpp/ics/conference/user.h"

/*
// This class represent a user's permission
@interface ICSConferencePermission (Internal)

-(instancetype)initWithNativePermission:(std::shared_ptr<const
ics::conference::Permission>)permission;
-(void)setNativePermission:(std::shared_ptr<const
ics::conference::Permission>)permission;
-(std::shared_ptr<const ics::conference::Permission>)nativePermission;

@end
*/

/// This class represent an attendee in a conference.
@interface ICSConferenceUser (Internal)

- (instancetype)initWithNativeUser:
    (std::shared_ptr<const ics::conference::User>)user;
- (void)setNativeUser:(std::shared_ptr<const ics::conference::User>)user;
- (std::shared_ptr<const ics::conference::User>)nativeUser;

@end
