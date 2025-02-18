#include "spi_emulator.h"
#include "device_driver.h"
#include "adc_constants.h"

#include <iterator>
#include <algorithm>
#include <iostream>


SpiEmulator::SpiEmulator():
  adc(&fake_copi_buffer, &fake_cipo_buffer)
{ ; }


// For simulation, just set the Clock Phase and Clock Polarity - see datasheet p. 88
void SpiEmulator::init(uint8_t SPI_mode)
{
  CPHA = SPI_mode & 0x01;
  CPOL = SPI_mode & 0x02;
  // On HW: use your HAL to set up hardware bus, interrupts, clock speed, MSB first/last, etc.
}


// Simulate full-duplex by clocking out a byte of data and returning the value clocked in
uint8_t SpiEmulator::transfer(uint8_t data)
{
  write(data);
  return read();
}


// Simulate clocking out a byte of data and then tell the emulated ADC to simulate 8 clock cycles
void SpiEmulator::write(uint8_t data)
{
  fake_copi_buffer = data;
  // std::cout << "sim spi wrote " << static_cast<uint16_t>(fake_copi_buffer) << std::endl;
  adc.simulate_op();
}

// Simulate clocking in a byte of data
uint8_t SpiEmulator::read(void)
{
  // std::cout << "sim spi read " << static_cast<uint16_t>(fake_cipo_buffer) << "\n" <<  std::endl;
  return fake_cipo_buffer;
}


