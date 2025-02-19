#include <gtest/gtest.h>
#include "spi_emulator.h"

TEST(SPITests, Send123)
{
  SpiEmulator spi;
  *spi.get_pCopi() = 0;  // <--DANGEROUS_POINTER_TO_WRITE_BUFFER!!!
  spi.write(123);
  ASSERT_EQ(*spi.get_pCopi(), 123);
}


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


TEST(SPITests, Receive123)
{
  SpiEmulator spi;
  *spi.get_pCipo() = 123;  // <--DANGEROUS_POINTER_TO_READ_BUFFER!!!
  ASSERT_EQ(spi.read(), 123);
}


TEST(SPITests, ReceiveWalkingBit)
{
  SpiEmulator spi;
  for (uint8_t n = 0; n < 8; ++n)
  {
    *spi.get_pCipo() = 0x01 << n;
    ASSERT_EQ(spi.read(), *spi.get_pCipo());  // <--DANGEROUS_POINTER_TO_READ_BUFFER!!!
  }
}


TEST(SPITests, ReceiveWalkingZero)
{
  SpiEmulator spi;
  for (uint8_t n = 0; n < 8; ++n)
  {
    *spi.get_pCipo() = ~(0x01 << n);
    ASSERT_EQ(spi.read(), *spi.get_pCipo());  // <--DANGEROUS_POINTER_TO_READ_BUFFER!!!
  }
}


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

