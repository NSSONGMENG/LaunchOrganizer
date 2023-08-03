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
#include <stdbool.h>


// ============================================================================
#pragma mark - Globals -
// ============================================================================

//static volatile bool g_isEnabled = false;

//static KSCrash_MonitorContext g_monitorContext;
//static KSStackCursor g_stackCursor;


/** Our custom signal stack. The signal handler will use this as its stack. */
static stack_t lo_signalStack = {0};

/** Signal handlers that were installed before we installed ours. */
static struct sigaction* lo_previousSignalHandlers = NULL;

static char g_eventID[37];

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
static void handleSignal(int sigNum, siginfo_t* signalInfo, void* userContext)
{
//    if(g_isEnabled)
//    {
//        thread_act_array_t threads = NULL;
//        mach_msg_type_number_t numThreads = 0;
//        ksmc_suspendEnvironment(&threads, &numThreads);
//        kscm_notifyFatalExceptionCaptured(false);
//
//        KSLOG_DEBUG("Filling out context.");
//        KSMC_NEW_CONTEXT(machineContext);
//        ksmc_getContextForSignal(userContext, machineContext);
//        kssc_initWithMachineContext(&g_stackCursor, KSSC_MAX_STACK_DEPTH, machineContext);
//
//        KSCrash_MonitorContext* crashContext = &g_monitorContext;
//        memset(crashContext, 0, sizeof(*crashContext));
//        crashContext->crashType = KSCrashMonitorTypeSignal;
//        crashContext->eventID = g_eventID;
//        crashContext->offendingMachineContext = machineContext;
//        crashContext->registersAreValid = true;
//        crashContext->faultAddress = (uintptr_t)signalInfo->si_addr;
//        crashContext->signal.userContext = userContext;
//        crashContext->signal.signum = signalInfo->si_signo;
//        crashContext->signal.sigcode = signalInfo->si_code;
//        crashContext->stackCursor = &g_stackCursor;
//
//        kscm_handleException(crashContext);
//        ksmc_resumeEnvironment(threads, numThreads);
//    }
//
//    KSLOG_DEBUG("Re-raising signal for regular handlers to catch.");
    
    // TODO: 向上层报告错误
    lo_handleException();
    
    raise(sigNum);
}


// ============================================================================
#pragma mark - API -
// ============================================================================

static bool lo_installSignalHandler(void)
{
    if (lo_signalStack.ss_size == 0) {
        lo_signalStack.ss_size = SIGSTKSZ;
        lo_signalStack.ss_sp = malloc(lo_signalStack.ss_size);
    }

    if (sigaltstack(&lo_signalStack, NULL) != 0) {
        goto failed;
    }

    const int* fatalSignals = lo_fatalSignals;
    int fatalSignalsCount = sizeof(fatalSignals) / sizeof(int*);

    if (lo_previousSignalHandlers == NULL) {
        lo_previousSignalHandlers = malloc(sizeof(*lo_previousSignalHandlers) * (unsigned)fatalSignalsCount);
    }

    struct sigaction action = {{0}};
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
#if defined(__LP64__)
    action.sa_flags |= SA_64REGSET;
#endif
    sigemptyset(&action.sa_mask);
    action.sa_sigaction = &handleSignal;

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
//    int fatalSignalsCount = sizeof(fatalSignals) / sizeof(int*);
//
//    for (int i = 0; i < fatalSignalsCount; i++) {
//        sigaction(fatalSignals[i], &lo_previousSignalHandlers[i], NULL);
//    }
//
//    lo_signalStack = (stack_t){0};
//}

