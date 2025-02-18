#ifndef ISPI_INTERFACE_DOT_AITCH
#define ISPI_INTERFACE_DOT_AITCH

#include <stdint.h>


// Abstract Base Class for hardware or simulated SPI interface
class ISpiInterface
{
protected:

  uint8_t CPHA;
  uint8_t CPOL;

public:
    virtual ~ISpiInterface() = default;

    // Abstract methods for SPI communication
    virtual void    init(uint8_t SPI_mode) = 0;
    virtual uint8_t transfer(uint8_t data) = 0;
    virtual void    write(uint8_t data) = 0;
    virtual uint8_t read(void) = 0;
};

#endif