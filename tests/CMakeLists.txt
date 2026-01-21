
# Enable testing
enable_testing()

set(CFL_ALL_TEST_TARGETS "")

# Helper macro to add tests easily
macro(add_cfl_test test_name source_file)
    add_executable(${test_name} ${source_file})
    target_link_libraries(${test_name} cfl-lib)
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME ${test_name} COMMAND ${test_name})
    list(APPEND CFL_ALL_TEST_TARGETS ${test_name})
endmacro()

# --- Group 1: Core Utilities & Data Structures ---
add_cfl_test(test_cfl_mem test_cfl_mem.c)
add_cfl_test(test_cfl_str test_cfl_str.c)
add_cfl_test(test_cfl_array test_cfl_array.c)
add_cfl_test(test_cfl_list test_cfl_list.c)
add_cfl_test(test_cfl_error test_cfl_error.c)


# --- Group 2: Advanced Data Structures ---
add_cfl_test(test_cfl_hash test_cfl_hash.c)
add_cfl_test(test_cfl_iterator test_cfl_iterator.c)
add_cfl_test(test_cfl_llist test_cfl_llist.c)
add_cfl_test(test_cfl_map test_cfl_map.c)
add_cfl_test(test_cfl_map_str test_cfl_map_str.c)
add_cfl_test(test_cfl_bitmap test_cfl_bitmap.c)
add_cfl_test(test_cfl_btree test_cfl_btree.c)

# --- Group 3: System & Concurrency ---
add_cfl_test(test_cfl_atomic test_cfl_atomic.c)
add_cfl_test(test_cfl_lock test_cfl_lock.c)
add_cfl_test(test_cfl_thread test_cfl_thread.c)
add_cfl_test(test_cfl_sync_queue test_cfl_sync_queue.c)
add_cfl_test(test_cfl_event test_cfl_event.c)
add_cfl_test(test_cfl_process test_cfl_process.c)
add_cfl_test(test_cfl_os test_cfl_os.c)

# --- Group 4: I/O & Complex Types ---
add_cfl_test(test_cfl_buffer test_cfl_buffer.c)
add_cfl_test(test_cfl_log test_cfl_log.c)
add_cfl_test(test_cfl_socket test_cfl_socket.c)
add_cfl_test(test_cfl_date test_cfl_date.c)
add_cfl_test(test_cfl_number test_cfl_number.c)
add_cfl_test(test_cfl_sql test_cfl_sql.c)

# Create a target that is built by default and runs all tests
add_custom_target(auto_test ALL
    COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG> --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running unit tests..."
)

# Ensure all tests are built before running them
add_dependencies(auto_test ${CFL_ALL_TEST_TARGETS})

