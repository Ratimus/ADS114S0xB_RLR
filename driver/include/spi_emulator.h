#ifndef SPI_EMULATOR_SPI_DOT_AITCH
#define SPI_EMULATOR_SPI_DOT_AITCH

#include <iostream>
#include <cstdint>

#include "i_spi_interface.h"
#include "adc_emulator.h"

class SpiEmulator : public ISpiInterface
{
  uint16_t sim_data;

  uint8_t fake_copi_buffer;
  uint8_t fake_cipo_buffer;

public:

  SpiEmulator();
  ~SpiEmulator() = default;

  virtual void    init(uint8_t SPI_mode) override;
  virtual uint8_t transfer(uint8_t data) override;
  virtual void    write(uint8_t data) override;
  virtual uint8_t read(void) override;

private:

  ADS114S08_Emulator adc;
};


// TODO: just a placeholder to remind me that I may want to set up a thread in the app that writes
// a sine wave to the simulated SPI registers or something if I get really ambitious
// extern void simulate_adc_data(void);

#endif