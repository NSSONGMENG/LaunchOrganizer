//
//  LOCrashMonitor_signal.c
//  LaunchOrganizer
//
//  Created by song.meng on 2023/8/3.
//

#include "LOCrashMonitor_signal.h"
#include "LOCrashMonitor.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/** Our custom signal stack. The signal handler will use this as its stack. */
static stack_t lo_signalStack = {0};

/** Signal handlers that were installed before we installed ours. */
static struct sigaction* lo_previousSignalHandlers = NULL;

static const int lo_fatalSignals[] =
{
    SIGABRT,
    SIGBUS,
    SIGFPE,
    SIGILL,
    SIGPIPE,
    SIGSEGV,
    SIGSYS,
    SIGTRAP,
};

// ============================================================================
#pragma mark - Callbacks -
// ============================================================================

/** Our custom signal handler.
 * Restore the default signal handlers, record the signal information, and
 * write a crash report.
 * Once we're done, re-raise the signal and let the default handlers deal with
 * it.
 *
 * @param sigNum The signal that was raised.
 *
 * @param signalInfo Information about the signal.
 *
 * @param userContext Other contextual information.
 */
static void lo_handleSignal(int sigNum, siginfo_t* signalInfo, void* userContext)
{
    // TODO: 向上层报告错误
    lo_handleException("signal");
    
    // 转发
    const int* fatalSignals = lo_fatalSignals;
    int fatalSignalsCount = (sizeof(lo_fatalSignals) / sizeof(lo_fatalSignals[0]));
    
    for (int i = 0; i < fatalSignalsCount; i++) {
        if (fatalSignals[i] == sigNum) {
            // Try to reverse the damage
            struct sigaction *s = &lo_previousSignalHandlers[i];
            if (s) {
                if (s->sa_sigaction) {
                    s->sa_sigaction(sigNum, signalInfo, userContext);
                } else if (s->sa_handler) {
                    s->sa_handler(sigNum);
                }
            }
        }
    }
}

bool lo_installSignalHandler(void)
{
    if (lo_signalStack.ss_size == 0) {
        lo_signalStack.ss_size = SIGSTKSZ;
        lo_signalStack.ss_sp = malloc(lo_signalStack.ss_size);
    }

    if (sigaltstack(&lo_signalStack, NULL) != 0) {
        goto failed;
    }

    const int* fatalSignals = lo_fatalSignals;
    int fatalSignalsCount = (sizeof(lo_fatalSignals) / sizeof(lo_fatalSignals[0]));

    if (lo_previousSignalHandlers == NULL) {
        lo_previousSignalHandlers = malloc(sizeof(*lo_previousSignalHandlers) * (unsigned)fatalSignalsCount);
    }

    struct sigaction action = {{0}};
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
#if defined(__LP64__)
    action.sa_flags |= SA_64REGSET;
#endif
    sigemptyset(&action.sa_mask);
    action.sa_sigaction = &lo_handleSignal;

    for (int i = 0; i < fatalSignalsCount; i++) {
        if (sigaction(fatalSignals[i], &action, &lo_previousSignalHandlers[i]) != 0) {
            // Try to reverse the damage
            for (i--;i >= 0; i--) {
                sigaction(fatalSignals[i], &lo_previousSignalHandlers[i], NULL);
            }
            goto failed;
        }
    }

    return true;

failed:
    return false;
}

//static void uninstallSignalHandler(void)
//{
//    const int* fatalSignals = lo_fatalSignals;
//    int fatalSignalsCount = (sizeof(lo_fatalSignals) / sizeof(lo_fatalSignals[0]));
//
//    for (int i = 0; i < fatalSignalsCount; i++) {
//        sigaction(fatalSignals[i], &lo_previousSignalHandlers[i], NULL);
//    }
//
//    lo_signalStack = (stack_t){0};
//}

