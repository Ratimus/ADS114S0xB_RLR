#include "adc_emulator.h"
#include "adc_constants.h"
#include <array>
#include <iostream>
#include <iomanip>


ADS114S08_Emulator::ADS114S08_Emulator(uint8_t * const copi,
                                       uint8_t * const cipo,
                                       bool simulate_startup_delay):
  simulate_startup_delay(simulate_startup_delay),
  COPI(copi),
  CIPO(cipo)
{
  reset();
}


uint8_t ADS114S08_Emulator::simulate_spi_read(void)
{
  return *COPI;
}


void ADS114S08_Emulator::simulate_spi_write(uint8_t data)
{
  *CIPO = data;
}


void ADS114S08_Emulator::simulate_outgoing_data(void)
{
  if (!output_buffer.empty())
  {
    uint8_t val = output_buffer.back();
    output_buffer.pop_back();
    simulate_spi_write(val);
  }
  else if (read_counter)
  {
    --read_counter;
    if (reg_pointer < registers.size())
    {
      if (simulate_startup_delay && (reg_pointer == ADS114S08_REGISTERS::STATUS) )
      {
        registers[reg_pointer] |= 0x01 << 5;
        static uint8_t tries = 3;
        if (tries)
        {
          --tries;
        }
        else
        {
          registers[reg_pointer] &= ~(0x01 << 5);
        }
      }
      simulate_spi_write(registers.at(reg_pointer));
      ++reg_pointer;
    }
    else
    {
      simulate_spi_write(0);
    }
  }
  else
  {
    simulate_spi_write(0);
  }
}


void ADS114S08_Emulator::store_new_data(uint8_t data)
{
  --write_counter;
  if (reg_pointer < registers.size())
  {
    registers[reg_pointer] = data;
    ++reg_pointer;
  }
  // else
  // Nowhere to put the data, so just ignore it
}


void ADS114S08_Emulator::handle_two_byte_command(uint8_t data)
{
  uint8_t read_or_write_count = (data & 0x1f) + 1;
  bool write = (input_register & ADS114S08_CMD::WREG_1ST);

  // Clear this since we've now received both CMD bytes
  input_register = 0;

  // Handle WREG by setting the write_counter using the received byte
  if (write)
  {
    write_counter = read_or_write_count;
  }
  else
  {
    read_counter = read_or_write_count;
  }
}


void ADS114S08_Emulator::handle_rdata_command(void)
{
  uint8_t  inmux_reg = registers.at(ADS114S08_REGISTERS::INPMUX);
  uint16_t pos_input = inmux_reg >> 4;
  uint16_t neg_input = inmux_reg & 0x0f;

  storage_buffer     = FAKE_VOLTAGES.at(pos_input);

  std::cout << "IN+ = ";
  if (pos_input < 12) std::cout << pos_input;
  else if (pos_input == 12) std::cout << "GND";
  else std::cout << "RESERVED";

  std::cout << ", IN- = ";
  if (neg_input < 12) std::cout << neg_input;
  else if (neg_input == 12) std::cout << "GND";
  else std::cout << "RESERVED";
  std::cout << std::endl;

  // Just emulating single-ended reads for now

  uint8_t msb = (storage_buffer >> 8);
  uint8_t lsb = (storage_buffer & 0xff);

  output_buffer.push_back(lsb);
  output_buffer.push_back(msb);
}


void ADS114S08_Emulator::simulate_op()
{
  // Simulate clocking out 8 bits of data (depends only on previous state, so we
  // can get this out of the way first
  simulate_outgoing_data();

  const uint8_t data = simulate_spi_read();

  // If we're expecting data with which to update our registers, do something with it
  if (write_counter)
  {
    return store_new_data(data);
  }

  // If previously received byte 0 of a WREG or RREG command, this is byte 1
  if (input_register)
  {
    return handle_two_byte_command(data);
  }

  // Handle RDATA by loading conversion data to the output buffer and then
  // clocking it out on the next two byte transfers
  if ((data & ~0x01) == ADS114S08_CMD::RDATA)
  {
    return handle_rdata_command();
  }

  if (data == ADS114S08_CMD::NOP)
  {
    return;
  }

  // Check for RREG / WREG
  uint8_t tmp = data & ~0b11111;

  // TODO: you could make these constants :-/
  uint8_t read_cmd  = (ADS114S08_CMD::RREG_1ST & ~0b11111);
  uint8_t write_cmd = (ADS114S08_CMD::WREG_1ST & ~0b11111);

  // Handle byte 0 of RREG or WREG command
  if ((tmp == read_cmd) || (tmp == write_cmd))
  {
    reg_pointer = (data & 0x1f);
    input_register = data;
    return;
  }

  // TODO: emulate responses to whatever additional commands you want
}


void ADS114S08_Emulator::reset()
{
  storage_buffer   = 0;
  reg_pointer      = 0;
  read_counter     = 0;
  write_counter    = 0;
  input_register   = 0;

  while (!output_buffer.empty())
  {
    output_buffer.pop_back();
  }

  // Python: all_the_addrs = [addr for addr in filter(lambda x: not x.startswith('_'), dir(ADS114S08_REGISTERS))]
  // but it's probably 300 times slower
  uint8_t all_the_addrs[] =
  {
    ADS114S08_REGISTERS::ID,
    ADS114S08_REGISTERS::STATUS,
    ADS114S08_REGISTERS::INPMUX,
    ADS114S08_REGISTERS::PGA,
    ADS114S08_REGISTERS::DATARATE,
    ADS114S08_REGISTERS::REF,
    ADS114S08_REGISTERS::IDACMAG,
    ADS114S08_REGISTERS::IDAC_MUX,
    ADS114S08_REGISTERS::VBIAS,
    ADS114S08_REGISTERS::SYS,
    ADS114S08_REGISTERS::RESERVED0,
    ADS114S08_REGISTERS::OFCAL0,
    ADS114S08_REGISTERS::OFCAL1,
    ADS114S08_REGISTERS::RESERVED1,
    ADS114S08_REGISTERS::FSCAL0,
    ADS114S08_REGISTERS::FSCAL1,
    ADS114S08_REGISTERS::GPIODAT,
    ADS114S08_REGISTERS::GPIOCON
  };

  // registers.clear();
  // Defaults - see datasheet p. 70
  for (auto addr: all_the_addrs)
  {
    uint8_t reg_val = 0;
    switch(addr)
    {
      case ADS114S08_REGISTERS::ID:
        reg_val = 0x04;  // ID for ADS114S08
        break;

      case ADS114S08_REGISTERS::STATUS:
        reg_val = 0x80;
        break;

      case ADS114S08_REGISTERS::INPMUX:
        reg_val = 0x01;
        break;

      case ADS114S08_REGISTERS::DATARATE:
        reg_val = 0x14;
        break;

      case ADS114S08_REGISTERS::REF:
        reg_val = 0x10;
        break;

      case ADS114S08_REGISTERS::IDAC_MUX:
        reg_val = 0xff;
        break;

      case ADS114S08_REGISTERS::SYS:
        reg_val = 0x10;
        break;

      case ADS114S08_REGISTERS::FSCAL1:
        reg_val = 0x40;
        break;

      default:
        break;
    }
    registers[addr] = reg_val;
  }

  for (auto& channel_reading: FAKE_VOLTAGES)
  {
    channel_reading = generate_adc_value();
  }
}


// Just a lazy hack to generate some pseudo random-looking numbers without needing another library
uint16_t ADS114S08_Emulator::generate_adc_value()
{
  static uint16_t some_num = 0b1001011010101001;
  some_num = (some_num << 1) | (some_num >> 15);
  some_num += 7;
  return some_num;
}
