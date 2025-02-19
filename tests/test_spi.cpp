#include <gtest/gtest.h>
#include "spi_emulator.h"


// Tests (simulated) writing a value to the SPI bus by calling SpiEmulator::write() for a single value and then checking the internal buffer to verify the value was written correctly
TEST(SPITests, Send123)
{
  SpiEmulator spi;
  *spi.get_pCopi() = 0;  // <--DANGEROUS_POINTER_TO_WRITE_BUFFER!!!
  spi.write(123);
  ASSERT_EQ(*spi.get_pCopi(), 123);
}


// Similar to the previous test, except that 0b00000001 is written, followed by 0b00000010, etc.
TEST(SPITests, SendWalkingBit)
{
  SpiEmulator spi;
  *spi.get_pCopi() = 0;  // <--DANGEROUS_POINTER_TO_WRITE_BUFFER!!!

  for (uint8_t n = 0; n < 8; ++n)
  {
    const uint8_t exp = (0x01 << n);
    spi.write(exp);
    ASSERT_EQ(*spi.get_pCopi(), exp);
  }
}


// Similar to the previous test, except that 0b11111110 is written, followed by 0b11111101, etc.
TEST(SPITests, SendWalkingZero)
{
  SpiEmulator spi;
  *spi.get_pCopi() = 0;  // <--DANGEROUS_POINTER_TO_WRITE_BUFFER!!!

  for (uint8_t n = 0; n < 8; ++n)
  {
    const uint8_t exp = ~(0x01 << n);
    spi.write(exp);
    ASSERT_EQ(*spi.get_pCopi(), exp);
  }
}


// Tests (simulated) receiving a value from the SPI bus by forcing a single value into the receive buffer via a pointer and then verifying the same value is received by a call to SpiEmulator::read()
TEST(SPITests, Receive123)
{
  SpiEmulator spi;
  *spi.get_pCipo() = 123;  // <--DANGEROUS_POINTER_TO_READ_BUFFER!!!
  ASSERT_EQ(spi.read(), 123);
}


// Similar to the previous test, except that 0b00000001 is expected, followed by 0b00000010, etc.
TEST(SPITests, ReceiveWalkingBit)
{
  SpiEmulator spi;
  for (uint8_t n = 0; n < 8; ++n)
  {
    *spi.get_pCipo() = 0x01 << n;
    ASSERT_EQ(spi.read(), *spi.get_pCipo());  // <--DANGEROUS_POINTER_TO_READ_BUFFER!!!
  }
}


// Similar to the previous test, except that 0b11111110 is expected, followed by 0b11111101, etc.
TEST(SPITests, ReceiveWalkingZero)
{
  SpiEmulator spi;
  for (uint8_t n = 0; n < 8; ++n)
  {
    *spi.get_pCipo() = ~(0x01 << n);
    ASSERT_EQ(spi.read(), *spi.get_pCipo());  // <--DANGEROUS_POINTER_TO_READ_BUFFER!!!
  }
}


// Simulates writing 255 to the bus, then writing all the numbers from 0 to 255. The output is "looped back" to the input, so that value of each read() should be the same value written by test_loopback() on the previous cycle
TEST(SPITests, LoopBack)
{
  SpiEmulator spi;
  // Pre-fill the output buffer with 255
  spi.write(255);
  for (uint8_t n = 0; true; ++n)
  {
    // Clock the output value back the the input and set up the value that will
    // be clocked out on the NEXT cycle
    spi.test_loopback(n);

    ASSERT_EQ(uint8_t(n - 1), spi.read());

    // If you increment n when it's == 255, it rolls back to zero and the loop never ends.
    // We still want (n - 1) to evaluate to 255, when n is zero so we'd prefer to keep
    // using a uint8_t. Check loop for exit condition here instead
    if (n == 255) break;
  }
}

