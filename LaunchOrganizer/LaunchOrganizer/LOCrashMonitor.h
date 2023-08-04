//
//  LOCrashMonitor.h
//  LaunchOrganizer
//
//  Created by song.meng on 2023/8/3.
//

#ifndef LOCrashMonitor_h
#define LOCrashMonitor_h

#include <stdio.h>

void lo_handleException(char *type);

void lo_installExceptionMonitor(void);


#endif /* LOCrashMonitor_h */
