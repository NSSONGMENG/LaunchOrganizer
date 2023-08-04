//
//  LOCrashMonitor.c
//  LaunchOrganizer
//
//  Created by song.meng on 2023/8/3.
//

#include "LOCrashMonitor.h"
#include "LOCrashMonitor_mach.h"
#include "LOCrashMonitor_signal.h"
#import "LaunchOrganizer.h"
#include <errno.h>
#include <string.h>
#include <sys/sysctl.h>
#include <unistd.h>

bool lodebug_isBeingTraced(void)
{
    struct kinfo_proc procInfo;
    size_t structSize = sizeof(procInfo);
    int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()};
    
    if(sysctl(mib, sizeof(mib)/sizeof(*mib), &procInfo, &structSize, NULL, 0) != 0)
    {
        return false;
    }
    
    return (procInfo.kp_proc.p_flag & P_TRACED) != 0;
}


void lo_handleException(char *type) {
    [LaunchOrganizer handleException:type];
}


static NSUncaughtExceptionHandler   *lo_privioidExceptionHandler = NULL;
static void handleUncaughtException(NSException* exception) {
    [LaunchOrganizer handleException:"oc_exception"];
    
    if (lo_privioidExceptionHandler) {
        lo_privioidExceptionHandler(exception);
    }
}

void lo_installExceptionMonitor(void) {
    lo_installSignalHandler();
    
    if (!lodebug_isBeingTraced()) {
        lo_installExceptionHandler();
    }
    
    lo_privioidExceptionHandler = NSGetUncaughtExceptionHandler();
    NSSetUncaughtExceptionHandler(&handleUncaughtException);
}
