//
//  AppDelegate.m
//  LaunchOrganizer
//
//  Created by song.meng on 2023/8/2.
//

#import "AppDelegate.h"
#import "LaunchOrganizer.h"

@interface AppDelegate ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    
    [LaunchOrganizer finishingLaunching:^BOOL(NSInteger count) {
        return count > 1;
    } launchAction:^{
        NSLog(@" 正常启动任务 ");
    } protectAction:^{
        NSLog(@" 启动保护任务 ");
    }];
    
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        // 启动任务完成，认为启动任务没有异常
        // 这个时间最好是配置及修复脚本能拉下来的时间，优先保证正常的修复手段能够起作用，不宜过长也不宜过短
        [LaunchOrganizer launchingFinished];
    });
    
    return YES;
}


#pragma mark - UISceneSession lifecycle


- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)connectingSceneSession options:(UISceneConnectionOptions *)options {
    // Called when a new scene session is being created.
    // Use this method to select a configuration to create the new scene with.
    return [[UISceneConfiguration alloc] initWithName:@"Default Configuration" sessionRole:connectingSceneSession.role];
}


- (void)application:(UIApplication *)application didDiscardSceneSessions:(NSSet<UISceneSession *> *)sceneSessions {
    // Called when the user discards a scene session.
    // If any sessions were discarded while the application was not running, this will be called shortly after application:didFinishLaunchingWithOptions.
    // Use this method to release any resources that were specific to the discarded scenes, as they will not return.
}


@end
