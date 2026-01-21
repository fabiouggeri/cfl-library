cmake_minimum_required(VERSION 3.10)

# set the project name
project(cfl-library
        VERSION 1.10.0)

set(CMAKE_C_STANDARD 99)

add_library(cfl-lib STATIC
            cfl-lib/src/main/c/cfl_array.c
            cfl-lib/src/main/c/cfl_atomic.c
            cfl-lib/src/main/c/cfl_bitmap.c
            cfl-lib/src/main/c/cfl_btree.c
            cfl-lib/src/main/c/cfl_buffer.c
            cfl-lib/src/main/c/cfl_date.c
            cfl-lib/src/main/c/cfl_error.c
            cfl-lib/src/main/c/cfl_event.c
            cfl-lib/src/main/c/cfl_hash.c
            cfl-lib/src/main/c/cfl_iterator.c
            cfl-lib/src/main/c/cfl_list.c
            cfl-lib/src/main/c/cfl_llist.c
            cfl-lib/src/main/c/cfl_lock.c
            cfl-lib/src/main/c/cfl_log.c
            cfl-lib/src/main/c/cfl_map.c
            cfl-lib/src/main/c/cfl_map_str.c
            cfl-lib/src/main/c/cfl_mem.c
            cfl-lib/src/main/c/cfl_process.c
            cfl-lib/src/main/c/cfl_socket.c
            cfl-lib/src/main/c/cfl_sql.c
            cfl-lib/src/main/c/cfl_str.c
            cfl-lib/src/main/c/cfl_sync_queue.c
            cfl-lib/src/main/c/cfl_thread.c
            cfl-lib/src/main/c/cfl_number.c )

target_include_directories(cfl-lib PUBLIC
                    ${PROJECT_SOURCE_DIR}/cfl-headers/src/main/headers
)

if(UNIX AND NOT APPLE)
    target_link_libraries(cfl-lib PUBLIC m)
endif()

enable_testing()

# Add tests directory
add_subdirectory(tests)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/cfl-headers/src/main/headers/"
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(TARGETS cfl-lib
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_VERBATIM_VARIABLES YES)
include(CPack)