#include <gtest/gtest.h>
#include <ctime>

#include "spi_emulator.h"
#include "device_driver.h"


// Instantiates device driver, verifies device_id and number of channels are both 0.
// Initializes driver and verifies device_id is as expected in accordance with the
// datasheet and that the number of channels reported is correct for the device.
TEST(DeviceDriverTests, test_init)
{
  const uint8_t ADS114S08_DEVICE_ID = 0x04;
  const uint8_t ADS114S08_NUM_CHANNELS = 12;

  SpiEmulator spi;
  DeviceDriver driver(spi);

  ASSERT_EQ(0, driver.get_device_id());
  ASSERT_EQ(0, driver.get_num_channels());

  driver.initialize();

  ASSERT_EQ(ADS114S08_DEVICE_ID, driver.get_device_id());
  ASSERT_EQ(ADS114S08_NUM_CHANNELS, driver.get_num_channels());
}


// Writes a randomly-generated uint8_t to each register and then goes back through
// each register to confirm the value stored is equal to the value that was written.
// NOTE: ADC emulator does not currently simulate read-only register values, so this
// test would need to be updated to run on actual hardware or if read-only registers
// are implemented in the emulator
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


// Cycles through all available ADC channels in non-consecutive order and stores the
// recorded values (which are randomly generated by the ADC emulator upon simulated reset).
// The channels are again cycled through, this time in a different order, and the same
// values recorded during the previous cycle are expected. If this test were to be
// implemented in hardware, constant voltage sources would be used instead of randomly-
// generated values and it would be unreasonable to expect the exact same values, so range-
// based expected values would need to be implemented in the test.
TEST(DeviceDriverTests, test_read_and_write_reg)
{
  SpiEmulator spi;
  DeviceDriver driver(spi);
  driver.initialize();

  for (int16_t val = 255; val >= 0; --val)
  {
    for (uint8_t reg_addr = 0; reg_addr < driver.NUM_REGISTERS; ++reg_addr)
    {
      const uint8_t expected = static_cast<uint8_t>(val);
      driver.write_register(reg_addr, expected);

      const uint8_t result = driver.read_register(reg_addr);
      ASSERT_EQ(expected, result);
    }
  }
}

