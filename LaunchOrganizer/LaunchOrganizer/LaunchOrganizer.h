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


/// 冷启动任务
/// - Parameters:
///   - crashBlock: 连续crash次数信息，不可为`nil`
///   - l_action: 启动任务，不可为`nil`
///   - p_action: 保护任务，不可为`nil`
+ (void)finishingLaunching:(LaunchOrganizerCrashInfo)crashBlock
              launchAction:(dispatch_block_t)l_action
             protectAction:(dispatch_block_t)p_action;


/// 添加生命周期任务，任务调用后即删除
/// - Parameters:
///   - key: 该任务绑定的key，保护操作完成后可根据key调用该任务
///   - action: 生命周期任务，不可为`nil`
///   - p_action: 该生命周期事件对应的保护任务，可为`nil`
+ (void)addLifeActionWithKey:(NSString *)key
                      action:(dispatch_block_t)action
               protectAction:(dispatch_block_t)p_action;


/// 根据`key`调用对应生命周期的任务
/// - Parameter key: 生命周期时间key
+ (void)doActionWithKey:(NSString *)key;


/// 告知启动任务结束
+ (void)launchingFinished;


/// 告知保护任务执行完毕，此方法调用将自动调用冷启动任务
+ (void)protectFinish;

/// 捕获到异常。如有crash组件可通过该方法设置
+ (void)handleException:(char *)type;

@end

NS_ASSUME_NONNULL_END
