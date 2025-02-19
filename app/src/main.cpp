// /app/main.cpp
//
// Application to excercise the ADS114S08 device driver:
//   - changes ADC channels
//   - reads ADC values
//   - reads and writes register values
#include "spi_emulator.h"
#include "device_driver.h"

#include <iostream>
#include <iomanip>


void print_hex(uint16_t val)
{
  std::cout << "0x" << std::setfill('0') << std::setw(4) << std::uppercase << std::hex << val << std::dec;
}


uint16_t read_adc_channel(DeviceDriver& adc, uint8_t ch, SpiEmulator spi)
{
  adc.set_channel(ch);
  uint16_t value = adc.read_adc_by_rdata_cmd();

  std::cout << "APP reading ADC channel " << (int)ch;
  std::cout << "... Expected: ";
  print_hex(spi.get_raw_adc_test_val(ch));
  std::cout << " | Received: ";
  print_hex(value);
  std::cout << std::endl;

  return value;
}


uint8_t read_register(DeviceDriver& adc, uint8_t addr)
{
    uint16_t value = adc.read_register(addr);

    std::cout << "APP received ";
    print_hex(value);
    std::cout << " from ADC register " << static_cast<uint16_t>(addr) << std::endl;

    return value;
}


void write_register(DeviceDriver& adc, uint8_t addr, uint8_t write_val)
{
  adc.write_register(addr, write_val);

  // If you need to confirm that the value was written successfully
  uint16_t value = adc.read_register(addr);

  std::cout << "App wrote ";
  print_hex(value);
  std::cout << " to ADC register " << static_cast<uint16_t>(value) << std::endl;
}


int main()
{
  SpiEmulator spi(1);
  DeviceDriver driver(spi);

  driver.initialize();

  std::cout << "---------------------------" << std::endl;

  for (auto adc_channel = 0; adc_channel < driver.get_num_channels(); ++adc_channel)
  {
    read_adc_channel(driver, adc_channel, spi);
  }

  std::cout << "---------------------------" << std::endl;

  for (auto reg_addr = 0; reg_addr < DeviceDriver::NUM_REGISTERS; ++reg_addr)
  {
    (void)read_register(driver, reg_addr);
  }

  std::cout << "---------------------------" << std::endl;

  uint8_t non_consecutives0[] = {0, 1, 17, 2, 4, 11, 6, 8, 14, 5, 10, 9, 3, 13, 7, 15, 12, 16};
  for (auto reg_addr: non_consecutives0)
  {
    // Just write the register number into the register itself so it's easy to verify
    write_register(driver, reg_addr, reg_addr);
  }

  std::cout << "---------------------------" << std::endl;

  uint8_t non_consecutives1[] = {17, 2, 1, 15, 12, 0, 4, 5, 10, 9, 3, 14, 16, 13, 7, 11, 6, 8};
  for (auto reg_addr: non_consecutives1)
  {
    (void)read_register(driver, reg_addr);
  }

  std::cout << "---------------------------" << std::endl;

  return 0;
}