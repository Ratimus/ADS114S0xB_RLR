#include <gtest/gtest.h>
#include <ctime>

#include "spi_emulator.h"
#include "device_driver.h"


TEST(DeviceDriverTests, test_init)
{
  SpiEmulator spi;
  DeviceDriver driver(spi);

  ASSERT_EQ(0, driver.get_device_id());

  driver.initialize();

  ASSERT_NE(0, driver.get_device_id());
}


TEST(DeviceDriverTests, test_set_channel_and_read)
{
  SpiEmulator spi;
  DeviceDriver driver(spi);
  driver.initialize();

  uint8_t non_consecutives0[] = {0, 1, 2, 4, 11, 6, 8, 5, 10, 9, 3, 7};
  uint8_t non_consecutives1[] = {2, 1, 0, 4, 5, 10, 9, 3, 7, 11, 6, 8};
  uint16_t readings[12] = {0};
  for (uint8_t n = 0; n < driver.get_num_channels(); ++n)
  {
    uint8_t randomized_index = non_consecutives0[n];
    driver.set_channel(randomized_index);
    readings[randomized_index] = driver.read_adc_by_rdata_cmd();
  }

  for (uint8_t n = 0; n < driver.get_num_channels(); ++n)
  {
    uint8_t randomized_index = non_consecutives1[n];
    driver.set_channel(randomized_index);
    ASSERT_EQ(readings[randomized_index], driver.read_adc_by_rdata_cmd());
  }
}


TEST(DeviceDriverTests, test_read_and_write_reg)
{
  SpiEmulator spi;
  DeviceDriver driver(spi);
  driver.initialize();
  std::srand(std::time(0));

  uint8_t expected[driver.NUM_REGISTERS] = {0};
  for (uint8_t reg_addr = 0; reg_addr < driver.NUM_REGISTERS; ++reg_addr)
  {
    const uint8_t write_val = std::rand() % 256;
    expected[reg_addr] = write_val;
    driver.write_register(reg_addr, write_val);
  }

  for (uint8_t reg_addr = 0; reg_addr < driver.NUM_REGISTERS; ++reg_addr)
  {
    const uint8_t ex = expected[reg_addr];
    ASSERT_EQ(ex, driver.read_register(reg_addr));
  }
}

