#include "cfl_buffer.h"
#include "cfl_str.h"
#include "cfl_test.h"

TEST_CASE(test_cfl_buffer_lifecycle) {
   CFL_BUFFERP buf = cfl_buffer_new();
   TEST_ASSERT(buf != NULL);
   TEST_ASSERT_EQUAL_INT(0, cfl_buffer_length(buf));
   cfl_buffer_free(buf);
}

TEST_CASE(test_cfl_buffer_write_read) {
   CFL_BUFFERP buf = cfl_buffer_new();
   cfl_buffer_setCapacity(buf, 100);

   cfl_buffer_putInt32(buf, 123456);
   cfl_buffer_putFloat(buf, 3.14f);
   cfl_buffer_putCharArray(buf, "Hello");

   TEST_ASSERT(cfl_buffer_length(buf) > 0);

   // Read back
   cfl_buffer_setPosition(buf, 0); // Reset read cursor (position)

   TEST_ASSERT_EQUAL_INT(123456, cfl_buffer_getInt32(buf));

   float f = cfl_buffer_getFloat(buf);
   TEST_ASSERT(f > 3.13 && f < 3.15);

   // char string reading? getCharArray returns allocated string
   // cfl_buffer_getString returns CFL_STRP.
   // If we used putCharArray, we should use getCharArray.

   // Let's adjust writing to match available API easily
}

TEST_CASE(test_cfl_buffer_putFormatArgs) {
   CFL_BUFFERP buf = cfl_buffer_newCapacity(100);
   cfl_buffer_putUInt8(buf, 0);
   cfl_buffer_putUInt8(buf, 1);
   cfl_buffer_putUInt8(buf, 2);  // 0x00
   cfl_buffer_putUInt8(buf, 3);  // 0x00
   cfl_buffer_putUInt8(buf, 4);  // 0x00
   cfl_buffer_putUInt8(buf, 5);  // 0x07
   cfl_buffer_putUInt8(buf, 6);  // H
   cfl_buffer_putUInt8(buf, 7);  // e
   cfl_buffer_putUInt8(buf, 8);  // l
   cfl_buffer_putUInt8(buf, 9);  // l
   cfl_buffer_putUInt8(buf, 10); // o
   cfl_buffer_putUInt8(buf, 11); //
   cfl_buffer_putUInt8(buf, 12); // A
   cfl_buffer_putUInt8(buf, 13);
   cfl_buffer_putUInt8(buf, 14);
   cfl_buffer_setPosition(buf, 2);

   cfl_buffer_putFormat(buf, "Hello %c", 'A');
   cfl_buffer_setPosition(buf, 12);
   TEST_ASSERT(cfl_buffer_getUInt8(buf) == 'A');

   cfl_buffer_setPosition(buf, 13);
   TEST_ASSERT(cfl_buffer_getUInt8(buf) == 13);
}

TEST_SUITE_BEGIN()
RUN_TEST(test_cfl_buffer_lifecycle);
RUN_TEST(test_cfl_buffer_write_read);
RUN_TEST(test_cfl_buffer_putFormatArgs);
TEST_SUITE_END()
