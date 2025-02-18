#include "adc_emulator.h"
#include "adc_constants.h"
#include <array>
#include <iostream>
#include <iomanip>


static void print_hex(uint16_t val)
{
  std::cout << "0x" << std::setfill('0') << std::setw(4) << std::uppercase << std::hex << val << std::dec;
}

ADS114S08_Emulator::ADS114S08_Emulator(uint8_t * const copi, uint8_t * const cipo):
  COPI(copi),
  CIPO(cipo)
{
  reset();
}


uint8_t ADS114S08_Emulator::simulate_spi_read(void)
{
  // std::cout << "ADC spi recvd " << static_cast<uint16_t>(*COPI) << std::endl;
  return *COPI;
}


void ADS114S08_Emulator::simulate_spi_write(uint8_t data)
{
  *CIPO = data;
  // std::cout << "ADC spi wrote " << static_cast<uint16_t>(*CIPO) << std::endl;
}


void ADS114S08_Emulator::simulate_op()
{
  static bool new_rdata = false;
  static bool one_byte  = false;

  uint8_t data = simulate_spi_read();

  if ( (data == ADS114S08_CMD::RDATA) || (data == 1 + ADS114S08_CMD::RDATA))
  {
    while (!output_buffer.empty())
    {
      output_buffer.pop_back();
    }

    uint8_t  inmux_reg = registers.at(ADS114S08_REGISTERS::INPMUX);
    uint16_t pos_input = inmux_reg >> 4;
    uint16_t neg_input = inmux_reg & 0x0f;

    // Just emulating single-ended reads for now
    storage_buffer = FAKE_VOLTAGES.at(pos_input);

    std::cout << "IN+ = ";
    if (pos_input < 12) std::cout << pos_input;
    else if (pos_input == 12) std::cout << "GND";
    else std::cout << "RESERVED";

    std::cout << ", IN- = ";
    if (neg_input < 12) std::cout << neg_input;
    else if (neg_input == 12) std::cout << "GND";
    else std::cout << "RESERVED";

    // std::cout << "Register " << int(reg.first) << ": 0x" << std::hex << int(reg.second) << std::dec << std::endl;
    std::cout << ", raw adc value = ";
    print_hex(storage_buffer);
    std::cout << std::endl;
    // << std::hex << storage_buffer << std::dec <<
    // std::cout << ", raw adc value = " << storage_buffer << std::endl;
    uint8_t msb = (storage_buffer >> 8);
    uint8_t lsb = (storage_buffer & 0xff);
    output_buffer.push_back(lsb);
    output_buffer.push_back(msb);
    return;
  }

  if (data == ADS114S08_CMD::NOP)
  {
    // std::cout << "data == ADS114S08_CMD::NOP" << std::endl;
    if (!output_buffer.empty())
    {
      uint8_t val = output_buffer.back();
      output_buffer.pop_back();
      simulate_spi_write(val);
      return;
    }

    simulate_spi_write(0);
  }

  if (!input_register)
  {
    // std::cout << "New command received" << std::endl;

    // Check for RREG / WREG
    uint8_t tmp = (data >> 5) << 5;

    // TODO: you could make these constants :-/
    uint8_t read_cmd  = (ADS114S08_CMD::RREG_1ST >> 5) << 5;
    uint8_t write_cmd = (ADS114S08_CMD::WREG_1ST >> 5) << 5;

    if ((tmp == read_cmd) || (tmp == write_cmd))
    {
      if (!read_counter && !write_counter)
      {
        // std::cout << "ADC R/W REG_0 CMD recvd!" << std::endl;
        reg_pointer = (data & 0x1f);
        // std::cout << "ADC reg ptr set to " << static_cast<uint16_t>(reg_pointer) << std::endl;
        input_register = data;
        return;
      }
    }
  }
  else
  {
    if (input_register & ADS114S08_CMD::WREG_1ST)
    {
      write_counter = (data & 0x1f) + 1;
      // std::cout << "ADC write counter set to " << static_cast<uint16_t>(write_counter) << std::endl;
    }
    else
    {
      // This is probably not how the actual IC works, i.e. I am just clearing the output register
      // rather than attemting to simulate faults
      while (!output_buffer.empty())
      {
        output_buffer.pop_back();
      }

      read_counter = (data & 0x1f) + 1;
      // std::cout << "ADC read counter set to " << static_cast<uint16_t>(read_counter) << std::endl;
      while (read_counter)
      {
        --read_counter;
        if (reg_pointer < registers.size())
        {
          if (reg_pointer == ADS114S08_REGISTERS::STATUS)
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
          output_buffer.push_front(registers[reg_pointer]);
          ++reg_pointer;
        }
        else
        {
          // TODO: I read in the datasheet that if you request to read past the end of the stack,
          // you just get zeros. Can't remember where it was, but a reference would be good
          output_buffer.push_front(0);
        }
      }
    }
    input_register = 0;
    return;
  }

  if (write_counter)
  {
    --write_counter;
    if (reg_pointer < registers.size())
    {
      registers.at(reg_pointer) = data;
      ++reg_pointer;
    }
    else
    {
      // Nowhere to put the data, so just ignore it
    }
  }
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

  registers.clear();
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
