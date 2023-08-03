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


static void *handler;

void lo_handleException(void) {
    [LaunchOrganizer handleException];
}

void lo_installExceptionMonitor(void) {
    lo_installSignalHandler();
    lo_installExceptionHandler();
}
