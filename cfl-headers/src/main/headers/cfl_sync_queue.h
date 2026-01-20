/**
 * @file cfl_sync_queue.h
 * @brief Thread-safe synchronous queue implementation.
 *
 * This module provides a blocking synchronous queue for inter-thread communication.
 * It supports blocking put/get operations, timeouts, and cancellation.
 */

#ifndef _CFL_SYNC_QUEUE_H_

#define _CFL_SYNC_QUEUE_H_

#include "cfl_types.h"
#include "cfl_lock.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Declares strongly-typed get and put functions for the queue.
 * @param datatype The C data type.
 * @param typename The type name suffix.
 */
#define DECLARE_GET_PUT(datatype, typename) \
   extern datatype cfl_sync_queue_get##typename(CFL_SYNC_QUEUEP queue); \
   extern datatype cfl_sync_queue_tryGet##typename(CFL_SYNC_QUEUEP queue, CFL_BOOL *took); \
   extern CFL_BOOL cfl_sync_queue_put##typename(CFL_SYNC_QUEUEP queue, datatype data); \
   extern CFL_BOOL cfl_sync_queue_tryPut##typename(CFL_SYNC_QUEUEP queue, datatype data)

/**
 * @brief Declares strongly-typed timeout get and put functions for the queue.
 * @param datatype The C data type.
 * @param typename The type name suffix.
 */
#define DECLARE_GET_PUT_TIMEOUT(datatype, typename) \
   extern datatype cfl_sync_queue_get##typename##Timeout(CFL_SYNC_QUEUEP queue, CFL_UINT32 timeout, CFL_BOOL *timesUp); \
   extern CFL_BOOL cfl_sync_queue_put##typename##Timeout(CFL_SYNC_QUEUEP queue, datatype data, CFL_UINT32 timeout)

/**
 * @brief Declares strongly-typed drain functions for the queue.
 * @param datatype The C data type.
 * @param typename The type name suffix.
 */
#define DECLARE_DRAIN(datatype, typename) \
   extern datatype cfl_sync_queue_drain##typename(CFL_SYNC_QUEUEP queue, CFL_BOOL *empty)

/**
 * @brief Union representing a generic item in the queue.
 */
typedef union _CFL_SYNC_QUEUE_ITEM {
   void      *asPointer; /**< Pointer value */
   CFL_BOOL   asBoolean; /**< Boolean value */
   CFL_INT8   asInt8;    /**< 8-bit signed integer */
   CFL_INT16  asInt16;   /**< 16-bit signed integer */
   CFL_INT32  asInt32;   /**< 32-bit signed integer */
   CFL_INT64  asInt64;   /**< 64-bit signed integer */
   CFL_UINT8  asUInt8;   /**< 8-bit unsigned integer */
   CFL_UINT16 asUInt16;  /**< 16-bit unsigned integer */
   CFL_UINT32 asUInt32;  /**< 32-bit unsigned integer */
   CFL_UINT64 asUInt64;  /**< 64-bit unsigned integer */
} CFL_SYNC_QUEUE_ITEM;

/**
 * @brief Synchronous Queue structure.
 */
typedef struct _CFL_SYNC_QUEUE {
   CFL_LOCK                lock;      /**< Lock for thread safety */
   CFL_CONDITION_VARIABLEP notEmpty;  /**< Condition variable for empty state */
   CFL_CONDITION_VARIABLEP notFull;   /**< Condition variable for full state */
   CFL_UINT32              size;      /**< Maximum capacity of the queue */
   CFL_UINT32              index;     /**< Current read index */
   CFL_UINT32              itemCount; /**< Number of items currently in queue */
   CFL_BOOL                canceled;  /**< Flag indicating if queue is canceled */
   CFL_SYNC_QUEUE_ITEM     data[1];   /**< Flexible array of queue items */
} CFL_SYNC_QUEUE, *CFL_SYNC_QUEUEP;

/**
 * @brief Creates a new synchronous queue.
 * @param size Maximum number of items the queue can hold.
 * @return Pointer to the new queue, or NULL on failure.
 */
extern CFL_SYNC_QUEUEP cfl_sync_queue_new(CFL_UINT32 size);

/**
 * @brief Frees a synchronous queue and its resources.
 * @param queue Pointer to the queue to free.
 */
extern void cfl_sync_queue_free(CFL_SYNC_QUEUEP queue);

/**
 * @brief Checks if the queue is empty.
 * @param queue Pointer to the queue.
 * @return CFL_TRUE if empty, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_sync_queue_isEmpty(CFL_SYNC_QUEUEP queue);

/**
 * @brief Checks if the queue is full.
 * @param queue Pointer to the queue.
 * @return CFL_TRUE if full, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_sync_queue_isFull(CFL_SYNC_QUEUEP queue);

/**
 * @brief Retrieves an item from the queue (blocking).
 * @param queue Pointer to the queue.
 * @return The item as a generic pointer.
 * @note Blocks if the queue is empty until an item is available or queue is canceled.
 */
extern void * cfl_sync_queue_get(CFL_SYNC_QUEUEP queue);

/**
 * @brief Attempts to retrieve an item without blocking.
 * @param queue Pointer to the queue.
 * @param took Output pointer set to CFL_TRUE if item was retrieved, CFL_FALSE otherwise.
 * @return The item as a generic pointer (valid only if *took is true).
 */
extern void * cfl_sync_queue_tryGet(CFL_SYNC_QUEUEP queue, CFL_BOOL *took);

/**
 * @brief Retrieves an item with a timeout.
 * @param queue Pointer to the queue.
 * @param timeout Timeout in milliseconds.
 * @param timesUp Output pointer set to CFL_TRUE if timeout occurred.
 * @return The item as a generic pointer (valid only if *timesUp is false).
 */
extern void * cfl_sync_queue_getTimeout(CFL_SYNC_QUEUEP queue, CFL_UINT32 timeout, CFL_BOOL *timesUp);

/**
 * @brief Puts an item into the queue (blocking).
 * @param queue Pointer to the queue.
 * @param data The item to add (as void pointer).
 * @return CFL_TRUE on success, CFL_FALSE if queue is canceled.
 * @note Blocks if the queue is full until space is available.
 */
extern CFL_BOOL cfl_sync_queue_put(CFL_SYNC_QUEUEP queue, void *data);

/**
 * @brief Attempts to put an item without blocking.
 * @param queue Pointer to the queue.
 * @param data The item to add.
 * @return CFL_TRUE on success, CFL_FALSE if queue is full or canceled.
 */
extern CFL_BOOL cfl_sync_queue_tryPut(CFL_SYNC_QUEUEP queue, void *data);

/**
 * @brief Puts an item into the queue with a timeout.
 * @param queue Pointer to the queue.
 * @param data The item to add.
 * @param timeout Timeout in milliseconds.
 * @return CFL_TRUE on success, CFL_FALSE on timeout or if canceled.
 */
extern CFL_BOOL cfl_sync_queue_putTimeout(CFL_SYNC_QUEUEP queue, void *data, CFL_UINT32 timeout);

/**
 * @brief Cancels the queue, releasing all waiting threads.
 * @param queue Pointer to the queue.
 */
extern void cfl_sync_queue_cancel(CFL_SYNC_QUEUEP queue);

/**
 * @brief Checks if the queue has been canceled.
 * @param queue Pointer to the queue.
 * @return CFL_TRUE if canceled, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_sync_queue_canceled(CFL_SYNC_QUEUEP queue);

/**
 * @brief Removes all items from the queue.
 * @param queue Pointer to the queue.
 * @param empty Output pointer set to CFL_TRUE if queue became empty.
 * @return The last removed item (as pointer).
 * @note This looks like it might retrieve one item while trying to empty?
 *       Typically drain would return a list or clear everything. Use with caution based on implementation.
 */
extern void *cfl_sync_queue_drain(CFL_SYNC_QUEUEP queue, CFL_BOOL *empty);

/**
 * @brief Waits until the queue is not empty (internal helper).
 * @param queue Pointer to the queue.
 * @param timeout Timeout in milliseconds.
 * @param timesUp Check for timeout.
 * @return Status helper.
 */
extern CFL_UINT32 cfl_sync_queue_waitNotEmptyTimeout(CFL_SYNC_QUEUEP queue, CFL_UINT32 timeout, CFL_BOOL *timesUp);

/**
 * @brief Waits until the queue is not empty (internal helper).
 * @param queue Pointer to the queue.
 * @return Status helper.
 */
extern CFL_UINT32 cfl_sync_queue_waitNotEmpty(CFL_SYNC_QUEUEP queue);

/**
 * @brief Waits until the queue is empty (internal helper).
 * @param queue Pointer to the queue.
 * @param timeout Timeout in milliseconds.
 * @param timesUp Check for timeout.
 * @return Status helper.
 */
extern CFL_UINT32 cfl_sync_queue_waitEmptyTimeout(CFL_SYNC_QUEUEP queue, CFL_UINT32 timeout, CFL_BOOL *timesUp);

/**
 * @brief Waits until the queue is empty (internal helper).
 * @param queue Pointer to the queue.
 * @return Status helper.
 */
extern CFL_UINT32 cfl_sync_queue_waitEmpty(CFL_SYNC_QUEUEP queue);

/* Typed helper declarations */

DECLARE_GET_PUT(CFL_BOOL    , Boolean);
DECLARE_GET_PUT(CFL_INT8    , Int8   );
DECLARE_GET_PUT(CFL_INT16   , Int16  );
DECLARE_GET_PUT(CFL_INT32   , Int32  );
DECLARE_GET_PUT(CFL_INT64   , Int64  );
DECLARE_GET_PUT(CFL_UINT8   , UInt8  );
DECLARE_GET_PUT(CFL_UINT16  , UInt16 );
DECLARE_GET_PUT(CFL_UINT32  , UInt32 );
DECLARE_GET_PUT(CFL_UINT64  , UInt64 );

DECLARE_GET_PUT_TIMEOUT(CFL_BOOL    , Boolean);
DECLARE_GET_PUT_TIMEOUT(CFL_INT8    , Int8   );
DECLARE_GET_PUT_TIMEOUT(CFL_INT16   , Int16  );
DECLARE_GET_PUT_TIMEOUT(CFL_INT32   , Int32  );
DECLARE_GET_PUT_TIMEOUT(CFL_INT64   , Int64  );
DECLARE_GET_PUT_TIMEOUT(CFL_UINT8   , UInt8  );
DECLARE_GET_PUT_TIMEOUT(CFL_UINT16  , UInt16 );
DECLARE_GET_PUT_TIMEOUT(CFL_UINT32  , UInt32 );
DECLARE_GET_PUT_TIMEOUT(CFL_UINT64  , UInt64 );

DECLARE_DRAIN(CFL_BOOL    , Boolean);
DECLARE_DRAIN(CFL_INT8    , Int8   );
DECLARE_DRAIN(CFL_INT16   , Int16  );
DECLARE_DRAIN(CFL_INT32   , Int32  );
DECLARE_DRAIN(CFL_INT64   , Int64  );
DECLARE_DRAIN(CFL_UINT8   , UInt8  );
DECLARE_DRAIN(CFL_UINT16  , UInt16 );
DECLARE_DRAIN(CFL_UINT32  , UInt32 );
DECLARE_DRAIN(CFL_UINT64  , UInt64 );

#ifdef __cplusplus
}
#endif

#endif
