//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/woogeen/sdk/conference/objc/RTCConferenceUser+Internal.h"

/*
@implementation RTCConferencePermission {
  std::shared_ptr<const woogeen::conference::Permission> _nativePermission;
}

-(BOOL)canPublish{
  return _nativePermission->CanPublish()? YES : NO;
}

-(BOOL)canSubscribe{
  return _nativePermission->CanSubscribe()? YES : NO;
}

-(BOOL)canRecord{
  return _nativePermission->CanRecord()? YES : NO;
}

@end

@implementation RTCConferencePermission (Internal)

-(instancetype)initWithNativePermission:(std::shared_ptr<const
woogeen::conference::Permission>)permission {
  self=[super init];
  [self setNativePermission: permission];
  return self;
}

-(void)setNativePermission:(std::shared_ptr<const
woogeen::conference::Permission>)permission {
  _nativePermission=permission;
}

-(std::shared_ptr<const woogeen::conference::Permission>)nativePermission{
  return _nativePermission;
}

@end
*/

@implementation RTCConferenceUser {
  std::shared_ptr<const woogeen::conference::User> _nativeUser;
}

- (NSString*)getUserId {
  return [NSString stringWithCString:_nativeUser->Id().c_str()
                            encoding:[NSString defaultCStringEncoding]];
}

- (NSString*)getName {
  return [NSString stringWithCString:_nativeUser->Name().c_str()
                            encoding:[NSString defaultCStringEncoding]];
}

- (NSString*)getRole {
  return [NSString stringWithCString:_nativeUser->Role().c_str()
                            encoding:[NSString defaultCStringEncoding]];
}

/*
-(RTCConferencePermission*)getPermissions{
  return [[RTCConferencePermission alloc] initWithNativePermission:
std::make_shared<woogeen::conference::Permission>(_nativeUser->Permissions())];
}*/

@end

@implementation RTCConferenceUser (Internal)

- (instancetype)initWithNativeUser:
    (std::shared_ptr<const woogeen::conference::User>)user {
  self = [super init];
  [self setNativeUser:user];
  return self;
}

- (void)setNativeUser:(std::shared_ptr<const woogeen::conference::User>)user {
  _nativeUser = user;
}

- (std::shared_ptr<const woogeen::conference::User>)nativeUser {
  return _nativeUser;
}

@end
