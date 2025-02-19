#ifndef SPI_EMULATOR_SPI_DOT_AITCH
#define SPI_EMULATOR_SPI_DOT_AITCH

#include <iostream>
#include <cstdint>

#include "i_spi_interface.h"
#include "device_driver.h"
#include "adc_emulator.h"

class SpiEmulator : public ISpiInterface
{
  uint16_t sim_data;

  uint8_t fake_copi_buffer;
  uint8_t fake_cipo_buffer;

public:

  SpiEmulator();
  SpiEmulator(bool simulate_startup_delay);
  ~SpiEmulator() = default;

  virtual void    init(uint8_t SPI_mode) override;
  virtual uint8_t transfer(uint8_t data) override;
  virtual void    write(uint8_t data) override;
  virtual uint8_t read(void) override;


  ////////////////////////// WARNING ////////////////////////
  // The following functions should never make their way into production code;
  // they are solely for testing the interface of the simulated SPI bus
  void test_loopback(uint8_t test_val)
  {
    fake_cipo_buffer = fake_copi_buffer;
    fake_copi_buffer = test_val;
  }

  uint8_t *get_pCopi()
  {
    return &fake_copi_buffer;
  }

  uint8_t *get_pCipo()
  {
    return &fake_cipo_buffer;
  }

  uint16_t get_raw_adc_test_val(uint8_t idx)
  {
    return adc.get_raw_adc_test_val(idx);
  }
  ///////////////////// END OF WARNING /////////////////////

private:

  ADS114S08_Emulator adc;
  friend uint16_t read_adc_channel(DeviceDriver& adc, uint8_t ch);
};


// TODO: just a placeholder to remind me that I may want to set up a thread in the app that writes
// a sine wave to the simulated SPI registers or something if I get really ambitious
// extern void simulate_adc_data(void);

#endif