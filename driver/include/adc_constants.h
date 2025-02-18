#ifndef ADC_CONSTANTS_DOT_AITCH
#define ADC_CONSTANTS_DOT_AITCH

#include <stdint.h>


// See datasheet p. 60 for serial control line option
struct MCU_GPIO_REGISTER_PINS
{
  static constexpr uint8_t CS_BAR               = 0x01;     // Can be tied low to permanently enable
  static constexpr uint8_t SCLK                 = 0x02;
  static constexpr uint8_t DIN                  = 0x04;     // Keep low during data readback
  static constexpr uint8_t DOUT_DRDY_BAR        = 0x08;     // Follows DRDY_BAR when CS_BAR is low, disabled when high
  static constexpr uint8_t DRDY_BAR             = 0x0F;
};

// Timing values
struct ADS114S08_TIMING
{
  static constexpr float   nS_TO_mS             = 1000.0;
  static constexpr long    TICKS_PER_UNIT_TIME  = 1;        // You're gonna need the datasheet for your MCU to find its clock frequency
  static constexpr long    TD_SCCS              = 20;       // nS
  static constexpr long    TD_CSSC              = 20;       // nS
  static constexpr long    T_CLK                = 245;      // 1 / 244.140625 nS = 4.096 MHz
};

// ADC Commands - datasheet p. 63
struct ADS114S08_CMD
{
  static constexpr uint8_t NOP                  = 0x00;     // No operation                    0000 0000 (00h)
  static constexpr uint8_t WAKEUP               = 0x02;     // Wake-up from power-down mode    0000 001x (02h, 03h)
  static constexpr uint8_t POWERDOWN            = 0x04;     // Enter power-down mode           0000 010x (04h, 05h)
  static constexpr uint8_t RESET                = 0x06;     // Reset the device                0000 011x (06h, 07h)
  static constexpr uint8_t START                = 0x08;     // Start conversions               0000 100x (08h, 09h)
  static constexpr uint8_t STOP                 = 0x0A;     // Stop conversions                0000 101x (0Ah, 0Bh)

  static constexpr uint8_t SYOCAL               = 0x16;     // System offset calibration       0001 0110 (16h)
  static constexpr uint8_t SYGCAL               = 0x17;     // System gain calibration         0001 0111 (17h)
  static constexpr uint8_t SFOCAL               = 0x19;     // Self offset calibration         0001 1001 (19h)

  static constexpr uint8_t RDATA                = 0x12;     // Read data by command            0001 001x (12h / 13h)

  static constexpr uint8_t RREG_1ST             = 0x20;     // RREG_1ST | 000r rrrr sets starting reg. to read from
  static constexpr uint8_t RREG_2ND             = 0x00;     // RREG_2ND | 000n nnnn sets number of reg. to read MINUS 1

  static constexpr uint8_t WREG_1ST             = 0x40;     // WREG_1ST | 000r rrrr sets starting reg. to write to
  static constexpr uint8_t WREG_2ND             = 0x00;     // WREG_2ND | 000n nnnn sets number of reg. to write MINUS 1
};

// ADC Registers - datasheep p. 70
struct ADS114S08_REGISTERS
{
  static constexpr uint8_t ID                   = 0x00;
  static constexpr uint8_t STATUS               = 0x01;
  static constexpr uint8_t INPMUX               = 0x02;
  static constexpr uint8_t PGA                  = 0x03;
  static constexpr uint8_t DATARATE             = 0x04;
  static constexpr uint8_t REF                  = 0x05;
  static constexpr uint8_t IDACMAG              = 0x06;
  static constexpr uint8_t IDAC_MUX             = 0x07;
  static constexpr uint8_t VBIAS                = 0x08;
  static constexpr uint8_t SYS                  = 0x09;
  static constexpr uint8_t RESERVED0            = 0x0A;
  static constexpr uint8_t OFCAL0               = 0x0B;
  static constexpr uint8_t OFCAL1               = 0x0C;
  static constexpr uint8_t RESERVED1            = 0x0D;
  static constexpr uint8_t FSCAL0               = 0x0E;
  static constexpr uint8_t FSCAL1               = 0x0F;
  static constexpr uint8_t GPIODAT              = 0x10;
  static constexpr uint8_t GPIOCON              = 0x11;
};


#endif