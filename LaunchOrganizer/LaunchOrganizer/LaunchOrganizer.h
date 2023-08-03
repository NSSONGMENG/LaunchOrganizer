//
//  LaunchOrganizer.h
//  LaunchOrganizer
//
//  Created by song.meng on 2023/8/2.
//
//  思路：https://alidocs.dingtalk.com/i/nodes/QOG9lyrgJP3A2XOXhGEn3EL4VzN67Mw4?utm_scene=person_space
//

#import <Foundation/Foundation.h>


typedef BOOL (^LaunchOrganizerCrashInfo) (NSInteger count);

NS_ASSUME_NONNULL_BEGIN

@interface LaunchOrganizer : NSObject


+ (void)finishingLaunching:(LaunchOrganizerCrashInfo)crashBlock
              launchAction:(dispatch_block_t)l_action
             protectAction:(dispatch_block_t)p_action;


+ (void)addLifeActionWithKey:(NSString *)key
                      action:(dispatch_block_t)action
               protectAction:(dispatch_block_t)p_action;

+ (void)doActionWithKey:(NSString *)key;

+ (void)launchingFinished;

+ (void)protectFinish;

/// 捕获到异常。如有crash组件可通过该方法设置
+ (void)handleException;

@end

NS_ASSUME_NONNULL_END
