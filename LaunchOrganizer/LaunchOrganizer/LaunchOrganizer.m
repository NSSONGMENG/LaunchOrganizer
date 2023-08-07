//
//  LaunchOrganizer.m
//  LaunchOrganizer
//
//  Created by song.meng on 2023/8/2.
//

#import "LaunchOrganizer.h"
#include "LOCrashMonitor.h"

#pragma mark - crash -
@interface LaunchOrganizer (crash)

+ (NSInteger)repeatCrashCount;
+ (void)increaseCrashCount;
+ (void)resetCrashCount;

@end

@implementation LaunchOrganizer (crash)

static NSString *crashKey = @"LaunchOrganizerCrashCount";

+ (NSInteger)repeatCrashCount {
    return [[NSUserDefaults standardUserDefaults] integerForKey:crashKey];
}

+ (void)increaseCrashCount {
    NSInteger count = [[NSUserDefaults standardUserDefaults] integerForKey:crashKey];
    [[NSUserDefaults standardUserDefaults] setInteger:count + 1 forKey:crashKey];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

+ (void)resetCrashCount {
    [[NSUserDefaults standardUserDefaults] setInteger:0 forKey:crashKey];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

@end


#pragma mark - main -

@interface LaunchOrganizer()

@property (nonatomic, strong) NSMutableDictionary   *actionMap;     // key - life_action
@property (nonatomic, strong) NSMutableDictionary   *protectMap;    // key - protect_action

@property (nonatomic, assign) BOOL  isProtecting;   // 正在保护标记
@property (nonatomic, assign) BOOL  launchFinished;   // 正在保护标记

@end

@implementation LaunchOrganizer

static NSString * didFinishLaunchingKey = @"didFinishLaunchingWithOptions";

+ (instancetype)shared {
    static LaunchOrganizer *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [LaunchOrganizer new];
        instance.actionMap = [NSMutableDictionary dictionary];
        instance.protectMap = [NSMutableDictionary dictionary];
    });
    return instance;
}

+ (void)finishingLaunching:(LaunchOrganizerCrashInfo)crashBlock
              launchAction:(dispatch_block_t)l_action
             protectAction:(dispatch_block_t)p_action {
    NSParameterAssert(crashBlock != nil);
    NSParameterAssert(l_action != nil);
    NSParameterAssert(p_action != nil);
    
    if (!crashBlock || !l_action || !p_action) return;
    
    // 若App版本发生变化，则清除crash记录
    NSString *buildV = [[NSUserDefaults standardUserDefaults] stringForKey:@"privious_build_version"];
    NSString *curV = [NSBundle mainBundle].infoDictionary[@"CFBundleVersion"];

    if (buildV != curV) {
        [[NSUserDefaults standardUserDefaults] setObject:curV forKey:@"privious_build_version"];
        [self resetCrashCount];
    }
    
    LaunchOrganizer *orgnizer = [LaunchOrganizer shared];
    
    lo_installExceptionMonitor();
    
    NSInteger count = [self repeatCrashCount];
    orgnizer.isProtecting = crashBlock(count);
    
    if (orgnizer.isProtecting) {
        p_action();
        // 暂时保存供保护操作完成后调用
        orgnizer.actionMap[didFinishLaunchingKey] = l_action;
    } else {
        l_action();
    }
}


+ (void)addLifeActionWithKey:(NSString *)key
                      action:(dispatch_block_t)action
               protectAction:(dispatch_block_t)p_action {
    NSParameterAssert([key isKindOfClass:[NSString class]] && key.length > 0);
    NSParameterAssert(action != nil);
    
    if (![key isKindOfClass:[NSString class]] || key.length < 1 || !action) return;
    
    LaunchOrganizer * orgnizer = [self shared];
    if (orgnizer.isProtecting) {
        orgnizer.actionMap[key] = action;
        
        if (p_action) {
            p_action();
        }
    } else {
        action();
    }
}

+ (void)doActionWithKey:(NSString *)key {
    NSParameterAssert([key isKindOfClass:[NSString class]] && key.length > 0);
    
    if ([key isKindOfClass:[NSString class]]) {
        LaunchOrganizer *orgnizer = [LaunchOrganizer shared];
        dispatch_block_t action = orgnizer.actionMap[key];
        if (action) action();
    }
}

+ (void)launchingFinished {
    LaunchOrganizer *orgnizer = [LaunchOrganizer shared];
    NSAssert(!orgnizer.isProtecting, @"【LaunchOrganizer error】保护期间不允许调用该方法，请在正常启动任务完成后调用");
    NSAssert([NSThread isMainThread], @"【LaunchOrganizer error】确保在主线程调用launchingFinished()");
        
    if (!orgnizer.isProtecting) {
        [self resetCrashCount];         // 重置crash count
        orgnizer.launchFinished = YES;
    }
}


+ (void)protectFinish {
    NSAssert([NSThread isMainThread], @"【LaunchOrganizer error】确保在主线程调用protectFinish()");
    
    LaunchOrganizer *orgnizer = [self shared];
    orgnizer.isProtecting = NO;
    
    // 主动调用一次启动事件
    dispatch_block_t launchAction = orgnizer.actionMap[didFinishLaunchingKey];
    if (launchAction) {
        orgnizer.actionMap[didFinishLaunchingKey] = nil;

        if ([NSThread isMainThread]) {
            launchAction();
        } else {
            dispatch_async(dispatch_get_main_queue(), launchAction);
        }
    }
}

//static int crashIndex = 0;
+ (void)handleException:(char *)type {
    LaunchOrganizer *orgnizer = [LaunchOrganizer shared];
    
    if (type == NULL) {
        type = "null";
    }
    
//    crashIndex ++;
//    NSString *key = [NSString stringWithFormat:@"crash_index_%d", crashIndex];
//    [[NSUserDefaults standardUserDefaults] setValue:[NSString stringWithUTF8String:type] forKey:key];
    
    if (!orgnizer.launchFinished) {
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            [self increaseCrashCount];
        });
    }
}

@end



