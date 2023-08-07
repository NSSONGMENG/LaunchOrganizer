

## [设计文档](https://alidocs.dingtalk.com/i/nodes/QOG9lyrgJP3A2XOXhGEn3EL4VzN67Mw4?utm_scene=person_space)

# 启动任务管理

## 该框架的作用：
    应对因代码健壮性不足导致启动阶段出现线上用户大面积连续崩溃且无法执行及时有效的修复手段这种高危现象。手段是通过对启动流程涉及的任务进行整体托管，在出现问题时可以选择性挂起启动流程，并进入特殊的保护状态。
    保护状态中，开发者可以执行特定的逻辑，比如清除缓存、更新/清除离线配置配置、删除无用数据库/表、拉热修修复脚本、请求特定接口上报问题或拉去预定义配置等等。
    待保护操作完成就可以执行启动流程，而不需要用户重启App。

## 冷启动保护原理
   ***如何决定冷启动crash：***在冷启动后的某个标志动作或时间内出现crash则认为出现了启动crash，否则认为正常启动。 
    
    出现冷启动crash则本地记录一个数字，且每次crash该数字自增1。若非连续，则正常启动后该标记清零。
    启动后开发者会得到这个连续crash数字，可能是0也可能是一个比较大的数，这个数字代表了在冷启动“特定时间和标志性动作”之前出现了连续的启动崩溃，需要开发者根据数字判断是否要进入保护流程。
    若进入保护流程，正常的启动任务被托管，保护流程中用户应自定义UI和提示，提示用户正在尝试修复中。
    
    ***如何确定启动完成的标志或时间：***
    举几个例子：
     1. 所有必要的启动任务执行完毕
     2. 修复脚本拉取完毕
     3. 离线配置拉取完毕
     
     ***如何确定修复任务执行完毕：***
     举几个例子：
     1. 缓存、无用库/表清除完毕
     2. 修复脚本拉取完毕
     3. 离线配置更新完毕
    
    比较关键的是如何定义启动完成和修复完成的标志，因为如果过长可能会包含一些业务上的非关键crash，如果过短可能会导致修复手段的有效性打折扣，因此回答以上两个问题需要结合自己业务的特点和需求通盘考虑，而不能人云亦云的用xx秒作为标志。

## 特点
 - 轻量：任务管理代码不足200行，crash检测代码不足500行
 - 简单：任务以大家熟知的dispatch_block_t组织
 - 扩展性佳：启动任务和保护任务易区分，相对于传统的hook方式可读性更好；由开发者决定启动流程及保护操作完成，而不是传统的时限限制，调整成本低
 
 
 ## API
 ```Objective-C
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
 ```
 
 
 ## 使用
 
 一般来讲遇到连续crash启动保护策略完成后需要执行的生命周期方法有两个: `didFinishLaunchingWithOptionsh`和`applicationDidBecomeActive`，其中当开发者告知保护操作完成后会自动调用`didFinishLaunchingWithOptionsh`的启动任务，其他的需要用户自己调用以便得到更好的灵活性。
 因为用户可以根据需要把启动任务进行更细节的拆分，通过`addLifeActionWithKey:`方法对应到不同的key-action中，这样可以根据配置进行更细节的配置。如：保护操作中可拉离线配置来决定哪些启动任务需要屏蔽，这样可以通过后台配置快速屏蔽问题任务以达到止损的目的。
 
 ```Objective-C
 - (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    _window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    
    [LaunchOrganizer finishingLaunching:^BOOL(NSInteger count) {
        NSLog(@"crash count: %ld", count);
        return count > 1;
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
//        [LaunchOrganizer doActionWithKey:@"applicationDidBecomeActive"];   // 执行become activity任务
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

 
 ```
