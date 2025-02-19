#include <iostream>

#include "device_driver.h"
#include "spi_emulator.h"
#include "adc_constants.h"


// There's going to be a register to set the GPIOs for our MCU. We'll do some bitwise operations
// on the individual bits to set/clear individual pins. These are just for pretend.
// For the sake of simplicity, pretend all the GPIO we care about is on the same port.
volatile uint32_t FAKE_GPIO_REGISTER_PORT_A = 0;


// Kills some time
void delay_nanos(long nanoseconds)
{
  long long ticks = nanoseconds * ADS114S08_TIMING::TICKS_PER_UNIT_TIME;
  // Volatile so we don't optimize this away, even if compiler thinks it's pointless
  for (volatile long long n = 0; n < ticks; ++n)
  {
    ;
  }
}


// Given the settling time, you might want the main app to do a semtake or something
// in order to wait 2.2 mS before proceeding when you first power up
DeviceDriver::DeviceDriver(ISpiInterface& spiInterface):
  spi(spiInterface),
  num_channels(0),
  device_id(0)
{
  delay_nanos(static_cast<long>(2.2f * ADS114S08_TIMING::nS_TO_mS));
}

// If the CS pin is not tied low permanently, configure the microcontroller GPIO connected to CS as an output;
void DeviceDriver::initialize()
{
  // Configure the SPI interface of the microcontroller to SPI mode 1 (CPOL = 0, CPHA = 1);
  spi.init(0x01);

  // Read the status register using the RREG command to check that the RDY bit is 0; //Optional
  std::cout << "Waiting for ADC RDY bit to go low" << std::endl;
  while (read_register(ADS114S08_REGISTERS::STATUS) & (0x01 << 5))
  {
    std::cout << "." << std::endl;
    delay_nanos(1000000000);
  }

  // Clear the FL_POR flag by writing 00h to the status register; //Optional
  write_register(ADS114S08_REGISTERS::STATUS, 0x00);

  // TODO: Write the respective register configuration with the WREG command;
  // TODO: For verification, read back all configuration registers with the RREG command;

  // One thing I might do is add std::array<uint8_t, NUM_REGISTERS>cached_registers as a
  // member of this class. Then, for data values that won't change often but that need to
  // be visible to users of this class, only update it when necessary (e.g. to verify
  // configuration changes, etc.).
  //
  // So DeviceDriver::get_id() might return cached_registers[ADS114S08_REGISTERS::ID],
  // DeviceDriver::get_active_channel() might use cached_registers[ADS114S08_REGISTERS::INPMUX],
  // and you might have a function like DeviceDriver::update_cache() that would refresh the cache

  device_id = read_register(ADS114S08_REGISTERS::ID);
  device_id &= 0x07;
  switch(device_id)
  {
    // ADS114S08
    case 0x04:
      num_channels = 12;
      break;

    // ADS114S06
    case 0x05:
      num_channels = 6;
      break;

    // Didn't find device. Handle error.
    default:
      break;
  }

  reset();
  set_channel(0);
}


uint8_t DeviceDriver::get_num_channels(void)
{
  return num_channels;
}


uint8_t DeviceDriver::get_device_id(void)
{
  return device_id;
}

// Configure input MUX - see datasheet p. 73
// For single-ended reads, set ch_minus to Analog Common (default)
void DeviceDriver::set_channel(uint8_t ch_plus, uint8_t ch_minus)
{
  // High nybble sets positive channel, low nybble sets negative
  uint8_t set_val = (ch_plus << 4) | (ch_minus & 0x0f);
  write_register(ADS114S08_REGISTERS::INPMUX, set_val);
  // TODO: could be fun to emulate differential reads by "reading"
  // a different value for each pin and then subtracting one from the other.
  // Of course, I'd have to scour the datasheet to find out how it handles
  // negative values.
}


// ADC reset - see datasheet p. 88
void DeviceDriver::reset(void)
{
  FAKE_GPIO_REGISTER_PORT_A &= ~MCU_GPIO_REGISTER_PINS::CS_BAR;
  delay_nanos(ADS114S08_TIMING::TD_CSSC);

  spi.write(ADS114S08_CMD::RESET);
  delay_nanos(ADS114S08_TIMING::T_CLK * 4096);
}


// Retrieve data from ADC data-holding register - see datasheet p. 68
// - Writes from ADC Data-holding register
// - Can be read at any time
// - Data output cycles as long as SCLK continues
uint16_t DeviceDriver::read_adc_by_rdata_cmd()
{
  uint16_t msb = 0;
  uint16_t lsb = 0;

  FAKE_GPIO_REGISTER_PORT_A &= ~MCU_GPIO_REGISTER_PINS::CS_BAR;

  spi.write(ADS114S08_CMD::RDATA);
  // TODO: if status byte enabled, uint8_t status_byte = spi.transfer(ADS114S08_CMD::NOP);
  msb = spi.transfer(ADS114S08_CMD::NOP) << 8;
  lsb = spi.transfer(ADS114S08_CMD::NOP);
  // TODO: if CRC enabled, uint8_t crc_byte = spi.transfer(ADS114S08_CMD::NOP);
  FAKE_GPIO_REGISTER_PORT_A |= MCU_GPIO_REGISTER_PINS::CS_BAR;

  return msb | lsb;
}


// This is just me playing around and testing the fidelity of my emulated ADC
void DeviceDriver::read_multi_registers(uint8_t start_reg,
                                        uint8_t num_reads,
                                        std::vector<uint8_t>& vec)
{
  uint8_t five_bit_addr = (start_reg & 0x1f);
  uint8_t five_bit_size = (num_reads - 1) & 0x1f;

  spi.write(ADS114S08_CMD::RREG_1ST | five_bit_addr);
  spi.write(ADS114S08_CMD::RREG_2ND | five_bit_size);

  vec.clear();
  vec.reserve(num_reads);
  for (uint8_t n = 0; n < num_reads; ++n)
  {
    vec[n] = spi.transfer(ADS114S08_CMD::NOP);
  }
}


// Read a byte
uint8_t DeviceDriver::read_register(uint8_t reg_addr)
{
  uint8_t num_reads = 1;
  uint8_t five_bit_addr = (reg_addr & 0x1f);
  uint8_t five_bit_size = (num_reads - 1) & 0x1f;

  spi.write(ADS114S08_CMD::RREG_1ST | five_bit_addr);
  spi.write(ADS114S08_CMD::RREG_2ND | five_bit_size);

  return spi.transfer(ADS114S08_CMD::NOP);
}


// Write a byte
void DeviceDriver::write_register(uint8_t reg_addr, uint8_t write_val)
{
  uint8_t num_writes = 1;
  uint8_t five_bit_addr = reg_addr & 0x1f;
  uint8_t five_bit_size = (num_writes - 1) & 0x1f;

  spi.write(ADS114S08_CMD::WREG_1ST | five_bit_addr);
  spi.write(ADS114S08_CMD::WREG_2ND | five_bit_size);
  spi.write(write_val);
}



///////////////////////////////////////////////////////////////////////////////
// bonus content!
///////////////////////////////////////////////////////////////////////////////

// Wouldn't do this in an actual device driver because of the dynamic memory
// allocation going on under the hood, I used this for verifying the fidelity
// of my ADC Emulator
uint8_t DeviceDriver::read_register_alt(uint8_t reg)
{
  std::vector<uint8_t> vec;
  static uint8_t read_count = 1;
  read_multi_registers(reg, read_count, vec);

  return vec[0];
}


// TaskHandle_t direct_read_adc_task_handle(nulltptr);

// Here's the interrupt service routine that fires when the DRDY_BAR pin goes low
void adc_ready_isr()
{
  // static BaseType_t xHigherPriorityTaskWoken  = pdFALSE;
  // xTaskNotifyGiveFromISR(direct_read_adc_task_handle, &xHigherPriorityTaskWoken);
  // // portYIELD_FROM_ISR(xHigherPriorityTaskWoken );  // Switch to ADC task if required (optional)
}


// Direct Read  - see datasheet p. 67
// Writes directly from (continuously updating) OUTPUT SHIFT REGISTER
// No serial activity shall occur from the falling edge of DRDY to the readback
// Keep Din low (or ADC will read in the command - full duplex)
// Finish reading before next DRDY falling edge
// Might get a STATUS byte
// Will get two data bytes
// Might get a CRC
// Cycle continues as long as SCLK continues
// Stop reading 28 clock periods before next DRDY falling edge
void direct_read_adc_task(void *param)
{
  // FAKE_GPIO_REGISTER_PORT_A &= ~MCU_GPIO_REGISTER_PINS::CS_BAR;
  //
  // uint16_t adc_val = 0;
  // adc_read_sem = xSemaphoreCreateBinary();
  // uint16_t msb = 0;
  // uint16_t lsb = 0;
  // while (1)
  // {
  //    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  //    // If status byte enabled, uint8_t status_byte = spi.transfer(ADS114S08_CMD::NOP);
  //    adc_val = spi.transfer(ADS114S08_CMD::NOP) << 8 | spi.transfer(ADS114S08_CMD::NOP);
  //    // If CRC enabled, uint8_t crc_byte = spi.transfer(ADS114S08_CMD::NOP);
  //    xQueueSend(adc_reads_queue, &adc_val, NULL);
  // }
}

// Blueprint for a FreeRTOS task to do continuous reads:
// in void main()
// {
//    . . .
//    start_adc_task();
//    // attach an interrupt to DRDY_PIN, FALLING edge, with adc_ready_isr as the ISR
//    vTaskStartScheduler();
void start_adc_task()
{
  // xTaskCreate
  // (
  //    direct_read_adc_task,         // Task target
  //    "ADC Read Task",              // Name
  //    1024,                         // Stack size (just a value I threw in there)
  //    nullptr,                      // Input params
  //    10,                           // Priority
  //    &direct_read_adc_task_handle  // Task handle
  // );
}

