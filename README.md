# CFL Library

**C Foundation Library** — A lightweight, cross-platform C library providing essential data structures and utilities commonly used in C projects. The goal is to offer an abstraction and simplification layer for everyday programming tasks.

![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)

---

## Features

| Module | Description |
|--------|-------------|
| **String** | Dynamic string implementation with utility functions |
| **Buffer** | Dynamic byte buffer with serialization support |
| **Synchronized Queue** | Thread-safe blocking queue for producer/consumer patterns |
| **Lock** | Mutex and condition variables for synchronization |
| **Thread** | Cross-platform thread management |
| **Array** | Dynamic array implementation |
| **List** | Linked list implementation |
| **Iterator** | Generic iterator pattern |
| **Socket** | Network socket abstraction |

---

## Installation

### Building with CMake

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Building with Maven (for integration with Java projects)

```bash
mvn clean install
```

---

## Usage Examples

### String (`cfl_str.h`)

The `CFL_STR` module provides a dynamic string type with automatic memory management and rich manipulation functions.

```c
#include "cfl_str.h"

int main() {
    // Creating strings
    CFL_STRP str = cfl_str_new(64);  // Create with initial capacity
    CFL_STRP copy = cfl_str_newBuffer("Hello, World!");

    // Setting values
    cfl_str_setValue(str, "Hello");
    cfl_str_appendChar(str, ' ');
    cfl_str_append(str, "CFL", " Library!", NULL);

    // Formatted strings
    cfl_str_setFormat(str, "Value: %d, Name: %s", 42, "test");

    // String operations
    cfl_str_toUpper(str);       // Convert to uppercase
    cfl_str_toLower(str);       // Convert to lowercase
    cfl_str_trim(str);          // Remove leading/trailing whitespace

    // Comparison
    if (cfl_str_bufferEquals(str, "expected value")) {
        // Strings are equal
    }

    if (cfl_str_startsWith(str, copy)) {
        // str starts with copy
    }

    // Searching
    CFL_INT32 pos = cfl_str_indexOf(str, 'L', 0);  // Find character
    if (pos >= 0) {
        char c = cfl_str_charAt(str, pos);  // Get character at position
    }

    // Substring extraction
    CFL_STRP sub = cfl_str_substr(str, 0, 5);  // Extract substring

    // Get raw pointer and length
    const char *ptr = cfl_str_getPtr(str);
    CFL_UINT32 len = cfl_str_length(str);

    // Cleanup
    cfl_str_free(sub);
    cfl_str_free(copy);
    cfl_str_free(str);

    return 0;
}
```

#### Stack-allocated String

```c
#include "cfl_str.h"

void example() {
    CFL_STR str = CFL_STR_EMPTY;  // Stack-allocated, empty
    cfl_str_initCapacity(&str, 128);

    cfl_str_setValue(&str, "Stack string");
    cfl_str_appendFormat(&str, " - ID: %d", 123);

    printf("%s\n", cfl_str_getPtr(&str));

    cfl_str_clear(&str);  // Clear contents but keep capacity
}
```

---

### Buffer (`cfl_buffer.h`)

The `CFL_BUFFER` module provides a dynamic byte buffer for binary data serialization with support for multiple data types and endianness control.

```c
#include "cfl_buffer.h"

int main() {
    // Create buffer
    CFL_BUFFERP buffer = cfl_buffer_newCapacity(1024);

    // Configure endianness
    cfl_buffer_setBigEndian(buffer, CFL_TRUE);  // Network byte order

    // Write various data types
    cfl_buffer_putInt32(buffer, 12345);
    cfl_buffer_putDouble(buffer, 3.14159);
    cfl_buffer_putBoolean(buffer, CFL_TRUE);
    cfl_buffer_putCharArray(buffer, "Hello");

    // Prepare for reading
    cfl_buffer_flip(buffer);  // length = position, position = 0

    // Read data back
    CFL_INT32 num = cfl_buffer_getInt32(buffer);
    double pi = cfl_buffer_getDouble(buffer);
    CFL_BOOL flag = cfl_buffer_getBoolean(buffer);
    char *text = cfl_buffer_getCharArray(buffer);

    printf("Read: %d, %.5f, %s, %s\n", num, pi,
           flag ? "true" : "false", text);

    // Check remaining bytes
    CFL_UINT32 remaining = cfl_buffer_remaining(buffer);

    // Reset for reuse
    cfl_buffer_reset(buffer);  // position = 0, length = 0

    // Cleanup
    free(text);
    cfl_buffer_free(buffer);

    return 0;
}
```

#### Working with Strings in Buffer

```c
#include "cfl_buffer.h"
#include "cfl_str.h"

void serialize_strings() {
    CFL_BUFFERP buffer = cfl_buffer_new();
    CFL_STRP str = cfl_str_newBuffer("Serialized string");

    // Write string to buffer
    cfl_buffer_putString(buffer, str);

    // Prepare for reading
    cfl_buffer_flip(buffer);

    // Read string from buffer
    CFL_STRP readStr = cfl_buffer_getString(buffer);

    // Or copy to existing string
    CFL_STR destStr = CFL_STR_EMPTY;
    cfl_buffer_rewind(buffer);
    cfl_buffer_copyString(buffer, &destStr);

    cfl_str_free(readStr);
    cfl_str_free(str);
    cfl_buffer_free(buffer);
}
```

---

### Lock (`cfl_lock.h`)

The `CFL_LOCK` module provides platform-independent mutex locks and condition variables for thread synchronization.

#### Basic Mutex Lock

```c
#include "cfl_lock.h"

// Global shared resource
int shared_counter = 0;
CFL_LOCK counter_lock;

void increment_safely() {
    cfl_lock_acquire(&counter_lock);
    shared_counter++;
    cfl_lock_release(&counter_lock);
}

int main() {
    cfl_lock_init(&counter_lock);

    // Use in multiple threads...
    increment_safely();

    return 0;
}
```

#### Try-Lock Pattern (Non-blocking)

```c
#include "cfl_lock.h"

CFL_LOCKP lock;

void try_do_work() {
    if (cfl_lock_tryAcquire(lock)) {
        // Lock acquired - do work
        do_critical_section();
        cfl_lock_release(lock);
    } else {
        // Lock not available - do something else
        do_alternative_work();
    }
}
```

#### Condition Variables (Producer/Consumer)

```c
#include "cfl_lock.h"
#include <stdbool.h>

CFL_LOCK lock;
CFL_CONDITION_VARIABLE dataReady;
bool hasData = false;
int data = 0;

void producer() {
    cfl_lock_acquire(&lock);

    data = 42;
    hasData = true;

    cfl_lock_conditionWake(&dataReady);  // Wake one consumer
    cfl_lock_release(&lock);
}

void consumer() {
    cfl_lock_acquire(&lock);

    while (!hasData) {
        cfl_lock_conditionWait(&lock, &dataReady);
    }

    printf("Received: %d\n", data);
    hasData = false;

    cfl_lock_release(&lock);
}

int main() {
    cfl_lock_init(&lock);
    cfl_lock_initConditionVar(&dataReady);

    // Start producer and consumer threads...

    return 0;
}
```

#### Condition Variable with Timeout

```c
#include "cfl_lock.h"

void wait_with_timeout(CFL_LOCKP lock, CFL_CONDITION_VARIABLEP cond) {
    cfl_lock_acquire(lock);

    CFL_UINT8 result = cfl_lock_conditionWaitTimeout(lock, cond, 5000);  // 5 seconds

    switch (result) {
        case CFL_LOCK_SUCCESS:
            printf("Condition signaled!\n");
            break;
        case CFL_LOCK_TIMEOUT:
            printf("Timeout expired\n");
            break;
        case CFL_LOCK_ERROR:
            printf("Error occurred\n");
            break;
    }

    cfl_lock_release(lock);
}
```

#### Reentrant Lock

```c
#include "cfl_lock.h"

CFL_RLOCK reentrantLock;

void recursive_function(int depth) {
    cfl_rlock_acquire(&reentrantLock);  // Same thread can acquire multiple times

    printf("Depth: %d, Lock count: %u\n", depth, cfl_rlock_getCount(&reentrantLock));

    if (depth > 0) {
        recursive_function(depth - 1);
    }

    cfl_rlock_release(&reentrantLock);  // Must release same number of times
}

int main() {
    cfl_rlock_init(&reentrantLock);
    recursive_function(5);
    return 0;
}
```

---

### Thread (`cfl_thread.h`)

The `CFL_THREAD` module provides cross-platform thread creation and management.

#### Basic Thread Creation

```c
#include "cfl_thread.h"
#include <stdio.h>

void worker_function(void *param) {
    int *value = (int *)param;
    printf("Worker thread received: %d\n", *value);

    // Simulate work
    cfl_thread_sleep(1000);  // Sleep 1 second

    printf("Worker finished\n");
}

int main() {
    CFL_THREADP thread = cfl_thread_newWithDescription(worker_function, "Worker Thread");

    int data = 42;

    if (cfl_thread_start(thread, &data)) {
        printf("Thread started successfully\n");

        // Wait for thread to complete
        cfl_thread_wait(thread);

        printf("Thread status: %d\n", cfl_thread_status(thread));
    }

    cfl_thread_free(thread);
    return 0;
}
```

#### Thread with Timeout Wait

```c
#include "cfl_thread.h"

void long_running_task(void *param) {
    cfl_thread_sleep(10000);  // 10 seconds
}

int main() {
    CFL_THREADP thread = cfl_thread_new(long_running_task);
    cfl_thread_start(thread, NULL);

    // Wait up to 5 seconds
    if (cfl_thread_waitTimeout(thread, 5000)) {
        printf("Thread completed in time\n");
    } else {
        printf("Thread still running, killing it...\n");
        cfl_thread_kill(thread);
    }

    cfl_thread_free(thread);
    return 0;
}
```

#### Thread-Local Variables

```c
#include "cfl_thread.h"

// Define a thread-local variable
CFL_THREAD_VAR(int, myThreadVar);

void thread_func(void *param) {
    // Set thread-local value
    cfl_thread_varSetInt32(&myThreadVar, (CFL_INT32)(intptr_t)param);

    // Each thread has its own copy
    CFL_INT32 value = cfl_thread_varGetInt32(&myThreadVar);
    printf("Thread %d value: %d\n", (int)(intptr_t)param, value);
}

int main() {
    CFL_THREADP t1 = cfl_thread_new(thread_func);
    CFL_THREADP t2 = cfl_thread_new(thread_func);

    cfl_thread_start(t1, (void*)1);
    cfl_thread_start(t2, (void*)2);

    cfl_thread_wait(t1);
    cfl_thread_wait(t2);

    cfl_thread_free(t1);
    cfl_thread_free(t2);

    return 0;
}
```

---

### Synchronized Queue (`cfl_sync_queue.h`)

The `CFL_SYNC_QUEUE` module provides a thread-safe blocking queue for producer/consumer patterns.

#### Basic Producer/Consumer

```c
#include "cfl_sync_queue.h"
#include "cfl_thread.h"
#include <stdio.h>

CFL_SYNC_QUEUEP queue;

void producer(void *param) {
    for (int i = 1; i <= 10; i++) {
        cfl_sync_queue_putInt32(queue, i);
        printf("Produced: %d\n", i);
    }
    // Signal end
    cfl_sync_queue_putInt32(queue, -1);
}

void consumer(void *param) {
    CFL_INT32 value;
    while (1) {
        value = cfl_sync_queue_getInt32(queue);
        if (value == -1) break;
        printf("Consumed: %d\n", value);
    }
}

int main() {
    queue = cfl_sync_queue_new(5);  // Queue size of 5

    CFL_THREADP prod = cfl_thread_new(producer);
    CFL_THREADP cons = cfl_thread_new(consumer);

    cfl_thread_start(prod, NULL);
    cfl_thread_start(cons, NULL);

    cfl_thread_wait(prod);
    cfl_thread_wait(cons);

    cfl_thread_free(prod);
    cfl_thread_free(cons);
    cfl_sync_queue_free(queue);

    return 0;
}
```

#### Non-Blocking Operations

```c
#include "cfl_sync_queue.h"

void non_blocking_example(CFL_SYNC_QUEUEP queue) {
    // Try to put without blocking
    if (cfl_sync_queue_tryPutInt32(queue, 42)) {
        printf("Successfully added to queue\n");
    } else {
        printf("Queue is full\n");
    }

    // Try to get without blocking
    CFL_BOOL took;
    CFL_INT32 value = cfl_sync_queue_tryGetInt32(queue, &took);
    if (took) {
        printf("Got value: %d\n", value);
    } else {
        printf("Queue is empty\n");
    }
}
```

#### Timeout Operations

```c
#include "cfl_sync_queue.h"

void timeout_example(CFL_SYNC_QUEUEP queue) {
    // Wait up to 5 seconds to get a value
    CFL_BOOL timedOut;
    CFL_INT32 value = cfl_sync_queue_getInt32Timeout(queue, 5000, &timedOut);

    if (!timedOut) {
        printf("Got: %d\n", value);
    } else {
        printf("Timed out waiting for data\n");
    }

    // Wait up to 5 seconds to put a value
    if (cfl_sync_queue_putInt32Timeout(queue, 100, 5000)) {
        printf("Successfully added\n");
    } else {
        printf("Timed out - queue full\n");
    }
}
```

#### Draining the Queue

```c
#include "cfl_sync_queue.h"

void drain_example(CFL_SYNC_QUEUEP queue) {
    CFL_BOOL empty;

    // Drain all values non-blocking
    while (1) {
        CFL_INT32 value = cfl_sync_queue_drainInt32(queue, &empty);
        if (empty) break;

        printf("Drained: %d\n", value);
    }
}
```

#### Canceling Queue Operations

```c
#include "cfl_sync_queue.h"

CFL_SYNC_QUEUEP queue;

void shutdown_handler() {
    // Cancel all blocking operations on the queue
    cfl_sync_queue_cancel(queue);
}

void worker(void *param) {
    while (!cfl_sync_queue_canceled(queue)) {
        void *data = cfl_sync_queue_get(queue);
        if (data == NULL && cfl_sync_queue_canceled(queue)) {
            break;  // Queue was canceled
        }
        process(data);
    }
}
```

---

## API Reference

For detailed API documentation, refer to the header files in `cfl-headers/src/main/headers/`:

| Header | Description |
|--------|-------------|
| `cfl_str.h` | Dynamic strings |
| `cfl_buffer.h` | Binary data buffers |
| `cfl_lock.h` | Mutexes and condition variables |
| `cfl_thread.h` | Thread management |
| `cfl_sync_queue.h` | Thread-safe queues |
| `cfl_array.h` | Dynamic arrays |
| `cfl_list.h` | Linked lists |
| `cfl_iterator.h` | Iterator pattern |
| `cfl_socket.h` | Network sockets |
| `cfl_hash.h` | Hash maps |
| `cfl_date.h` | Date/time utilities |

---

## Platform Support

| Platform | Compiler | Status |
|----------|----------|--------|
| Windows | MSVC | ✅ Supported |
| Windows | MinGW | ✅ Supported |
| Linux | GCC | ✅ Supported |
| Linux | Clang | ✅ Supported |

---

## License

Licensed under the [Apache License 2.0](LICENSE).

---

## Contributing

Contributions are welcome! Feel free to submit issues and pull requests.

This library is not intended to be a complete solution for all projects, but rather a foundation for common tasks. Feel free to use and improve it.
