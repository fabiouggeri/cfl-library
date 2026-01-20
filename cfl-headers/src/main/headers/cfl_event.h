/**
 * @file cfl_event.h
 * @brief Event synchronization primitive.
 *
 * This module provides an event object for thread synchronization, allowing
 * threads to wait for a signal from another thread. Supports manual and
 * auto-reset events.
 */

#ifndef CFL_EVENT_H_

#define CFL_EVENT_H_

#include "cfl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Event wait result: Failed */
#define CFL_EVENT_FAIL 0
/** @brief Event wait result: Event was set */
#define CFL_EVENT_SET 1
/** @brief Event wait result: Timeout occurred */
#define CFL_EVENT_TIMEOUT 2

/** @brief Opaque handle to an event object */
typedef void *CFL_EVENTP;

/**
 * @brief Creates a new event object.
 * @param name Optional name for the event (can be NULL).
 * @param manualReset If CFL_TRUE, event remains signaled until manually reset.
 *                    If CFL_FALSE, event automatically resets after releasing a
 * waiting thread.
 * @return Handle to the new event, or NULL if creation fails.
 */
extern CFL_EVENTP cfl_event_new(char *name, CFL_BOOL manualReset);

/**
 * @brief Frees an event object.
 * @param event Handle to the event to free.
 */
extern void cfl_event_free(CFL_EVENTP event);

/**
 * @brief Signals the event (sets it to signaled state).
 *
 * Wakes up waiting threads depending on the reset mode:
 * - Auto-reset: Wakes one waiting thread.
 * - Manual-reset: Wakes all waiting threads.
 *
 * @param event Handle to the event.
 */
extern void cfl_event_set(CFL_EVENTP event);

/**
 * @brief Resets the event (sets it to non-signaled state).
 * @param event Handle to the event.
 */
extern void cfl_event_reset(CFL_EVENTP event);

/**
 * @brief Waits indefinitely for the event to be signaled.
 * @param event Handle to the event.
 * @return CFL_TRUE if the event was signaled, CFL_FALSE on error.
 */
extern CFL_BOOL cfl_event_wait(CFL_EVENTP event);

/**
 * @brief Waits indefinitely for the event to be signaled (returns status code).
 * @param event Handle to the event.
 * @return CFL_EVENT_SET on success, CFL_EVENT_FAIL on error.
 */
extern CFL_UINT8 cfl_event_wait2(CFL_EVENTP event);

/**
 * @brief Waits for the event with a timeout.
 * @param event Handle to the event.
 * @param timeout Maximum time to wait in milliseconds.
 * @return CFL_TRUE if the event was signaled, CFL_FALSE on timeout or error.
 */
extern CFL_BOOL cfl_event_waitTimeout(CFL_EVENTP event, CFL_INT32 timeout);

/**
 * @brief Waits for the event with a timeout (returns status code).
 * @param event Handle to the event.
 * @param timeout Maximum time to wait in milliseconds.
 * @return CFL_EVENT_SET if signaled, CFL_EVENT_TIMEOUT on timeout,
 * CFL_EVENT_FAIL on error.
 */
extern CFL_UINT8 cfl_event_waitTimeout2(CFL_EVENTP event, CFL_INT32 timeout);

#ifdef __cplusplus
}
#endif

#endif
