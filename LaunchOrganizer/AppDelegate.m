//
//  AppDelegate.m
//  LaunchOrganizer
//
//  Created by song.meng on 2023/8/2.
//

#import "AppDelegate.h"
#import "LaunchOrganizer.h"
#import "ViewController.h"
#import "ProtectViewController.h"

@interface AppDelegate ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    _window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    
    __block NSString *text = @"";
    [LaunchOrganizer finishingLaunching:^BOOL(NSInteger count) {
        NSLog(@"crash count: %ld", count);
        BOOL protect = count > 1;
        
        text = protect ? [NSString stringWithFormat:@"保护中 %ld", count] : @"正常启动";
        
        return protect;
    } launchAction:^{
        NSLog(@" 正常启动任务 ");
        
        ViewController *vc = [[ViewController alloc] init];
        self->_window.rootViewController = vc;
        
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            // 启动任务完成，认为启动任务没有异常
            // 这个时间最好是配置及修复脚本能拉下来的时间，优先保证正常的修复手段能够起作用，不宜过长也不宜过短
            [LaunchOrganizer launchingFinished];
            NSLog(@" 启动完成 ");
        });
    } protectAction:^{
        NSLog(@" 启动保护任务 ");
        
        ProtectViewController * vc = [ProtectViewController new];
        vc.title = text;
        self->_window.rootViewController = vc;
        
//        [LaunchOrganizer protectFinish];  // 保护操作完成
//        [LaunchOrganizer doActionWithKey:@"applicationDidEnterBackground"];
    }];
    
    [_window makeKeyAndVisible];
    
    return YES;
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    [LaunchOrganizer addLifeActionWithKey:@"applicationDidBecomeActive" action:^{
        // 正常的生命周期任务
    } protectAction:^{
        // 受保护状态下需要执行的特殊任务，可以为nil
    }];
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    [LaunchOrganizer addLifeActionWithKey:@"applicationDidEnterBackground" action:^{
        // 正常的生命周期任务
    } protectAction:^{
        // 受保护状态下需要执行的特殊任务，可以为nil
    }];
}

@end
