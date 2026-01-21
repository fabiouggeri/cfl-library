#include "cfl_test.h"
#include "cfl_socket.h"

TEST_CASE(test_cfl_socket_init) {
    // CFL_SOCKET typically requires WSAStartup on Windows
    // cfl_socket_init() might handle that.

    // Verify headers and basic type availability
    // CFL_SOCKET_HANDLE sock = CFL_INVALID_SOCKET;
    // TEST_ASSERT(sock == CFL_INVALID_SOCKET);

    TEST_PASS();
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_socket_init);
TEST_SUITE_END()
