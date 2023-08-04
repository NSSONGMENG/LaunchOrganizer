//
//  ProtectViewController.m
//  LaunchOrganizer
//
//  Created by song.meng on 2023/8/4.
//

#import "ProtectViewController.h"
#import "LaunchOrganizer.h"

@interface ProtectViewController ()

@end

@implementation ProtectViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    UILabel *labe = [[UILabel alloc] initWithFrame:CGRectMake(10, 200, self.view.frame.size.width - 20, 30)];
    labe.font = [UIFont systemFontOfSize:15];
    labe.textColor = [UIColor redColor];
    labe.textAlignment = NSTextAlignmentCenter;
    labe.text = self.title;
    [self.view addSubview:labe];
    
    
    UIButton *btn = [[UIButton alloc] initWithFrame:CGRectMake(100, 400, 200, 30)];
    [btn setTitle:@"abort" forState:UIControlStateNormal];
    [btn setTitleColor:[UIColor redColor] forState:UIControlStateNormal];
    [btn setBackgroundColor:[UIColor yellowColor]];
    btn.layer.cornerRadius = 5;
    [btn addTarget:self action:@selector(abortAction) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:btn];
    
    
    UIButton *btn1 = [[UIButton alloc] initWithFrame:CGRectMake(100, 450, 200, 30)];
    [btn1 setTitle:@"protect finish" forState:UIControlStateNormal];
    [btn1 setTitleColor:[UIColor redColor] forState:UIControlStateNormal];
    [btn1 setBackgroundColor:[UIColor yellowColor]];
    btn1.layer.cornerRadius = 5;
    [btn1 addTarget:self action:@selector(finishAciton) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:btn1];
}

- (void)abortAction {
    abort();
}

- (void)finishAciton {
    [LaunchOrganizer protectFinish];
}

@end
