#ifndef ADC_EMULATOR_DOT_AITCH
#define ADC_EMULATOR_DOT_AITCH

#include <array>
#include <list>
#include <stdint.h>
#include <unordered_map>

class ADS114S08_Emulator
{
    // Simulate 8 clock cycles by writing 8 bits in (from controller to peripheral) on COPI
    // and out (peripheral to controller) on CIPO
    volatile uint8_t *const COPI;
    volatile uint8_t *const CIPO;

    uint8_t reg_pointer;
    uint8_t read_counter;
    uint8_t write_counter;

    // For storing the first byte of two-byte combos (i.e. RREG and WREG)
    uint8_t input_register;

    // Magic number from datasheet. Defined as a constant in DeviceDriver
    std::array<uint8_t, 18>  reg_addrs;
    std::list<uint8_t>       output_buffer;
    std::array<uint16_t, 12> FAKE_VOLTAGES;

    std::unordered_map<uint8_t, uint8_t> registers;

    volatile uint16_t storage_buffer;
    uint16_t          generate_adc_value();

    uint8_t simulate_spi_read(void);
    void    simulate_spi_write(uint8_t data);
    void    simulate_outgoing_data(void);

    void store_new_data(uint8_t data);
    void handle_two_byte_command(uint8_t data);
    void handle_rdata_command(void);

    bool simulate_startup_delay;

  public:
    ADS114S08_Emulator(uint8_t *const copi, uint8_t *const cipo, bool simulate_startup_delay = false);

    void simulate_op();
    void reset();

    uint16_t get_raw_adc_test_val(uint8_t idx) { return FAKE_VOLTAGES.at(idx); }
};

#endif
