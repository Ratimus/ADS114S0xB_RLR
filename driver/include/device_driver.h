// Baremetal device driver that can:
//   initialize the device
//   - DeviceDriver::initialize()
//
//   valid read/write SPI
//   - ISpiInterface::read() & ISpiInterface::write()
//
//   ability to read/write all the registers on the device
//   - DeviceDriver::read_register() & DeviceDriver::write_register()
//
//   read valid ADC values from device
//   - DeviceDriver::read_adc()

#ifndef DEVICE_DRIVER_DOT_AITCH
#define DEVICE_DRIVER_DOT_AITCH

#include "i_spi_interface.h"
#include <vector>

struct MCU_GPIO_REGISTER_PINS;
struct ADS114S08_TIMING;
struct ADS114S08_CMD;
struct ADS114S08_REGISTERS;

class DeviceDriver
{
  ISpiInterface& spi;
  uint8_t num_channels;

public:

  inline static const uint8_t NUM_REGISTERS = 18;

  DeviceDriver(ISpiInterface& spiInterface);
  ~DeviceDriver() = default;

  uint8_t   get_num_channels(void);

  void      reset(void);
  void      initialize(void);

  // Default negative channel = GND
  void      set_channel(uint8_t ch_plus, uint8_t ch_minus = 0x0c);
  uint16_t  read_adc_by_rdata_cmd(void);

  void      write_register(uint8_t reg, uint8_t value);
  uint8_t   read_register(uint8_t reg);

  // Just for fun
  void      read_multi_registers(uint8_t start_reg, uint8_t num_reads, std::vector<uint8_t>& vec);
  uint8_t   read_register_alt(uint8_t reg);

};


#endif