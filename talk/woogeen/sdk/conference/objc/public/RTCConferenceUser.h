//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "Foundation/Foundation.h"

// This class represent a user's permission
@interface RTCConferencePermission : NSObject

// Indicates whether the user can publish streams to conference.
-(BOOL)canPublish;
// Indicates whether the user can subscribe streams from conference.
-(BOOL)canSubscribe;
// Indicates whether the user can record conference.
-(BOOL)canRecord;

@end

/// This class represent an attendee in a conference.
@interface RTCConferenceUser : NSObject

// Get user's ID.
-(NSString*)getUserId;
// Get user's name.
-(NSString*)getName;
// Get user's role.
-(NSString*)getRole;
// Get user's permission.
-(NSString*)getPermissions;

@end
