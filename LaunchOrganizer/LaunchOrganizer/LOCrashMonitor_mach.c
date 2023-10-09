//
//  LOCrashMonitor_mach.c
//  LaunchOrganizer
//
//  Created by song.meng on 2023/8/3.
//

#include "LOCrashMonitor_mach.h"
#include "LOCrashMonitor.h"
#include <mach/mach.h>
#include <pthread.h>
#include <signal.h>


// ============================================================================
#pragma mark - Constants -
// ============================================================================

static const char* kThreadPrimary = "LOCrash Exception Handler (Primary)";
static const char* kThreadSecondary = "LOCrash Exception Handler (Secondary)";

#if __LP64__
    #define MACH_ERROR_CODE_MASK 0xFFFFFFFFFFFFFFFF
#else
    #define MACH_ERROR_CODE_MASK 0xFFFFFFFF
#endif

// ============================================================================
#pragma mark - Types -
// ============================================================================

/** A mach exception message (according to ux_exception.c, xnu-1699.22.81).
 */
#pragma pack(4)
typedef struct {
    /** Mach header. */
    mach_msg_header_t          header;

    // Start of the kernel processed data.

    /** Basic message body data. */
    mach_msg_body_t            body;

    /** The thread that raised the exception. */
    mach_msg_port_descriptor_t thread;

    /** The task that raised the exception. */
    mach_msg_port_descriptor_t task;

    // End of the kernel processed data.

    /** Network Data Representation. */
    NDR_record_t               NDR;

    /** The exception that was raised. */
    exception_type_t           exception;

    /** The number of codes. */
    mach_msg_type_number_t     codeCount;

    /** Exception code and subcode. */
    // ux_exception.c defines this as mach_exception_data_t for some reason.
    // But it's not actually a pointer; it's an embedded array.
    // On 32-bit systems, only the lower 32 bits of the code and subcode
    // are valid.
    mach_exception_data_type_t code[0];

    /** Padding to avoid RCV_TOO_LARGE. */
    char                       padding[512];
} LOMachExceptionMessage;
#pragma pack()

/** A mach reply message (according to ux_exception.c, xnu-1699.22.81).
 */
#pragma pack(4)
typedef struct {
    /** Mach header. */
    mach_msg_header_t header;

    /** Network Data Representation. */
    NDR_record_t      NDR;

    /** Return code. */
    kern_return_t     returnCode;
} LOMachReplyMessage;
#pragma pack()

// ============================================================================
#pragma mark - Globals -
// ============================================================================

/** Holds exception port info regarding the previously installed exception
 * handlers.
 */
static struct {
    exception_mask_t        masks[EXC_TYPES_COUNT];
    exception_handler_t     ports[EXC_TYPES_COUNT];
    exception_behavior_t    behaviors[EXC_TYPES_COUNT];
    thread_state_flavor_t   flavors[EXC_TYPES_COUNT];
    mach_msg_type_number_t  count;
} lo_previousExceptionPorts;

/** Our exception port. */
static mach_port_t lo_exceptionPort = MACH_PORT_NULL;

/** Primary exception handler thread. */
static pthread_t lo_primaryPThread;
static thread_t lo_primaryMachThread;

/** Secondary exception handler thread in case crash handler crashes. */
static pthread_t lo_secondaryPThread;
static thread_t lo_secondaryMachThread;

static char lo_primaryEventID[37];
static char lo_secondaryEventID[37];

// ============================================================================
#pragma mark - Utility -
// ============================================================================

thread_t lothread_self(void) {
    thread_t thread_self = mach_thread_self();
    mach_port_deallocate(mach_task_self(), thread_self);
    return thread_self;
}

/** Restore the original mach exception ports.
 */
static void lo_restoreExceptionPorts(void)
{
    if (lo_previousExceptionPorts.count == 0) {
        return;
    }

    const task_t thisTask = mach_task_self();
    kern_return_t kr;

    // Reinstall old exception ports.
    for (mach_msg_type_number_t i = 0; i < lo_previousExceptionPorts.count; i++) {
        kr = task_set_exception_ports(thisTask,
                                      lo_previousExceptionPorts.masks[i],
                                      lo_previousExceptionPorts.ports[i],
                                      lo_previousExceptionPorts.behaviors[i],
                                      lo_previousExceptionPorts.flavors[i]);
        if (kr != KERN_SUCCESS) {
//            KSLOG_ERROR("task_set_exception_ports: %s", mach_error_string(kr));
        }
    }

    lo_previousExceptionPorts.count = 0;
}


// ============================================================================
#pragma mark - Handler -
// ============================================================================

/** Our exception handler thread routine.
 * Wait for an exception message, uninstall our exception port, record the
 * exception information, and write a report.
 */
static void* lo_handleExceptions(void* const userData)
{
    LOMachExceptionMessage exceptionMessage = {{0}};
    LOMachReplyMessage replyMessage = {{0}};

    for(;;) {
        // Wait for a message.
        kern_return_t kr = mach_msg(&exceptionMessage.header,
                                    MACH_RCV_MSG,
                                    0,
                                    sizeof(exceptionMessage),
                                    lo_exceptionPort,
                                    MACH_MSG_TIMEOUT_NONE,
                                    MACH_PORT_NULL);
        if (kr == KERN_SUCCESS) {
            // TODO: 向上层报告错误
            lo_handleException("mach");
            break;
        }
    }

    replyMessage.header = exceptionMessage.header;
    replyMessage.NDR = exceptionMessage.NDR;
    replyMessage.returnCode = KERN_FAILURE;

    mach_msg(&replyMessage.header,
             MACH_SEND_MSG,
             sizeof(replyMessage),
             0,
             MACH_PORT_NULL,
             MACH_MSG_TIMEOUT_NONE,
             MACH_PORT_NULL);

    return NULL;
}


// ============================================================================
#pragma mark - API -
// ============================================================================

static void lo_uninstallExceptionHandler(void)
{
    // NOTE: Do not deallocate the exception port. If a secondary crash occurs
    // it will hang the process.
    
    lo_restoreExceptionPorts();
    
    thread_t thread_self = lothread_self();
    
    if (lo_primaryPThread != 0 && lo_primaryMachThread != thread_self) {
        pthread_cancel(lo_primaryPThread);
        lo_primaryMachThread = 0;
        lo_primaryPThread = 0;
    }
    
    if (lo_secondaryPThread != 0 && lo_secondaryMachThread != thread_self) {
        pthread_cancel(lo_secondaryPThread);
        lo_secondaryMachThread = 0;
        lo_secondaryPThread = 0;
    }
    
    lo_exceptionPort = MACH_PORT_NULL;
}

bool lo_installExceptionHandler(void)
{
    bool attributes_created = false;
    pthread_attr_t attr;

    kern_return_t kr;
    int error;

    const task_t thisTask = mach_task_self();
    exception_mask_t mask = EXC_MASK_BAD_ACCESS |
    EXC_MASK_BAD_INSTRUCTION |
    EXC_MASK_ARITHMETIC |
    EXC_MASK_SOFTWARE |
    EXC_MASK_BREAKPOINT;

    kr = task_get_exception_ports(thisTask,
                                  mask,
                                  lo_previousExceptionPorts.masks,
                                  &lo_previousExceptionPorts.count,
                                  lo_previousExceptionPorts.ports,
                                  lo_previousExceptionPorts.behaviors,
                                  lo_previousExceptionPorts.flavors);
    if (kr != KERN_SUCCESS) {
        goto failed;
    }

    if (lo_exceptionPort == MACH_PORT_NULL) {
        kr = mach_port_allocate(thisTask,
                                MACH_PORT_RIGHT_RECEIVE,
                                &lo_exceptionPort);
        if (kr != KERN_SUCCESS) {
            goto failed;
        }

        kr = mach_port_insert_right(thisTask,
                                    lo_exceptionPort,
                                    lo_exceptionPort,
                                    MACH_MSG_TYPE_MAKE_SEND);
        if (kr != KERN_SUCCESS) {
            goto failed;
        }
    }

    kr = task_set_exception_ports(thisTask,
                                  mask,
                                  lo_exceptionPort,
                                  (int)(EXCEPTION_DEFAULT | MACH_EXCEPTION_CODES),
                                  THREAD_STATE_NONE);
    if (kr != KERN_SUCCESS) {
        goto failed;
    }

    pthread_attr_init(&attr);
    attributes_created = true;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    error = pthread_create(&lo_secondaryPThread,
                           &attr,
                           &lo_handleExceptions,
                           (void*)kThreadSecondary);
    if (error != 0) {
        goto failed;
    }
    
    lo_secondaryMachThread = pthread_mach_thread_np(lo_secondaryPThread);

    error = pthread_create(&lo_primaryPThread,
                           &attr,
                           &lo_handleExceptions,
                           (void*)kThreadPrimary);
    if (error != 0) {
        goto failed;
    }
    
    pthread_attr_destroy(&attr);
    lo_primaryMachThread = pthread_mach_thread_np(lo_primaryPThread);

    return true;


failed:
    if (attributes_created) {
        pthread_attr_destroy(&attr);
    }
    lo_uninstallExceptionHandler();
    return false;
}

