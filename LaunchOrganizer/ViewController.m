//
//  ViewController.m
//  LaunchOrganizer
//
//  Created by song.meng on 2023/8/2.
//

#import "ViewController.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    UILabel *labe = [[UILabel alloc] initWithFrame:CGRectMake(100, 200, 300, 30)];
    labe.font = [UIFont systemFontOfSize:15];
    labe.textColor = [UIColor redColor];
    labe.text = @"正常启动";
    [self.view addSubview:labe];
        
    self.view.backgroundColor = [UIColor whiteColor];
    
    
    
    UIButton *btn = [[UIButton alloc] initWithFrame:CGRectMake(100, 400, 200, 30)];
    [btn setTitle:@"abort" forState:UIControlStateNormal];
    [btn setTitleColor:[UIColor redColor] forState:UIControlStateNormal];
    [btn setBackgroundColor:[UIColor yellowColor]];
    btn.layer.cornerRadius = 5;
    [btn addTarget:self action:@selector(abortAction) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:btn];
    
    
    UIButton *btn1 = [[UIButton alloc] initWithFrame:CGRectMake(100, 450, 200, 30)];
    [btn1 setTitle:@"array[100]" forState:UIControlStateNormal];
    [btn1 setTitleColor:[UIColor redColor] forState:UIControlStateNormal];
    [btn1 setBackgroundColor:[UIColor yellowColor]];
    btn1.layer.cornerRadius = 5;
    [btn1 addTarget:self action:@selector(outOfSize) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:btn1];
}

- (void)abortAction {
    abort();
}

- (void)outOfSize {
    @[][1];
}

@end
