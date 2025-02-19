# ADS114S0xB Device Driver for Embedded Platforms by Ryan Richardson (aka `Ratimus`)

This project demonstrates the general approach to developing a device driver for embedded platforms, allowing the microcontroller to communicate with and control an ADS114S0xB Analog-to-Digital Converter (ADC) via the Serial Peripheral Interface (SPI).

## Project Structure

### `/driver`
Bare-metal device driver implementing the following functionality:
- Initialization of the device
- Reading and writing data and commands over the SPI bus
- Reading and writing the registers on the device
- Reading ADC values from the device

### `/app`
User-space application that uses the driver to:
- Read ADC values
- Read register values
- Change ADC channels

### `/tests`
Automated tests using the GoogleTest framework and built using the CTest facility provided by CMake

## Building and Running the Project

### Prerequisites:
- Linux OS (I used Windows Subsystem for Linux (WSL) with Ubuntu)
- Project targets C++17
- CMake 3.1 or greater
- GCC compiler
- Git

The following instructions are for Ubuntu (including Ubuntu under WSL). If you're using another distro or Windows stand-alone, use the equivalent commands for your OS. Feel free to skip any of the steps in part 1 if you already have CMake and/or git set up on your machine.

### Step 1: Set Up Your Build Environment

Install CMake, the GCC compiler, and other required tools:
```bash
$ sudo apt-get update
$ sudo apt-get install build-essential
```

Install git:
```bash
$ sudo apt-get install git
```

If the previous step fails, you might need to tell your OS how to find git using the following command and then trying that step again:
```bash
$ sudo add-apt-repository ppa:git-core/ppa -y && sudo apt-get update && sudo apt-get upgrade && sudo apt-get install git -y
```

### Step 2: Clone this repository to your machine and set up the project

Navigate to the destination folder where you want to clone this project. If you don't know where you want to store it, here's how you'd create a folder to store this (and other) projects:
```bash
$ mkdir ~/repos
$ cd ~/repos
```

Clone this repository to the current folder:
```bash
$ git clone https://github.com/Ratimus/ADS114S0xB_RLR
```

Create the <build> folder and navigate into it:
```bash
$ cd ADS114S0xB_RLR
$ mkdir build
$ cd build
```

### Step 3: Build it
```bash
$ cmake ..
$ cmake --build .
```

### Step 4: Run it
Run the app:
```bash
$ app/app
```

Run all tests:
```
$ ctest
```

## Software architecture:
The `ADS114S08_Emulator` is a software emulation of some of the basic functionality of an ADS114S0x Analog to Digital Integrated Circuit (IC). This IC communicates as a peripheral on a Serial Peripheral Interface (SPI) bus. In order to simulate this, only a single function is used to model interactions between the ADC and the SPI controller, `void ADS114S08_Emulator::simulate_op(void);`. Each call to this function mimics 8 serial clock cycles on the SPI bus. A `reset(void)` function is also available to emulate the power-up bahavior of the IC.

The SPI bus and controller are also emulated in software via an instance of the `SpiEmulator` class. This class inherits its interface from the abstract base class `ISpiInterface`. The `DeviceDriver` stores a reference to an instance of the `ISpiInterface`, so the emulated bus could be replaced by a hardware SPI component as long as it implements the interface (yay dependency injection). Communication between the driver and the SPI controller is via `read(void)`, `write(uint8_t)`, and `transfer(uint8_t)` functions. The `write()` operation simulates writing out 8 bits on the `COPI` pin by setting a member variable corresponding to this pin and then calling `ADS114S08_Emulator::simulate_op()` to simulate 8 clock cycles.

Inside the ADC emulation, the bus read is simulated in `simulate_op()` by reading the value from `COPI` via a stored pointer to the member variable in the SPI emulator, doing whatever it's going to do in response to the received data byte, and then simulating writing back on the SPI bus by setting the value of the `CIPO` member variable of the SPI emulator. I could (arguably, should) have used references or shared pointers, but this is just an emulation of hardware and not really part of the driver itself. Calling `SpiEmulator::read()` from the driver simply returns the value clocked in on `CIPO` during the previous communication cycle between the bus and the IC.

And so, we finally get to the `DeviceDriver` class itself. This was the easiest part; all it really does is read and write data via SPI and (simulate) reading and setting GPIO pins to control the IC. The various functions are essentially just wrappers to send and receive SPI data one byte at a time in whatever manner the datasheet describes in order to perform a given task.

The application consists of a simple function that runs once and then exits. This function instantiates an object of the `SpiEmulator` class and passes it to the `DeviceDriver` constructor and then excercises various `DeviceDriver` functions.

## Testing
### The app itself verifies the basic functionality of the device driver by performing the following actions:
- Calls `DeviceDriver::initialize()`, which in turn initializes the SPI bus and the ADC IC
- Calls `DeviceDriver::read_adc_by_rdata_cmd()` for each ADC channel, which sets `CS` low, writes data to the `INPMUX` register such that the desired channel is the source of sampled data, sends the `RDATA` command to initiate sending data from the ADC, and then reads back two bytes corresponding to the raw value from the IC (and sets `CS` high again)
- Calls `DeviceDriver::read_register()` for each register present on the ADC, collecting the default values present in the registers after a reset
- Calls `DeviceDriver::write_register()` for each register, writing each register's own number into it for ease of verification. NOTE: read-only behavior of the actual IC is not modeled by the emulator, so all previous values are overwritten
- Calls `DeviceDriver::read_register()` again for each register, verifying the previous contents have been replaced with the new values.

### GoogleTest framework: SpiEmulator - test_spi.cpp:

`TEST(SPITests, Send123)`
- tests (simulated) writing a value to the SPI bus by calling SpiEmulator::write() for a single value and then checking the internal buffer to verify the value was written correctly

`TEST(SPITests, SendWalkingBit)`
- is similar to the previous test, except that 0b00000001 is written, followed by 0b00000010, etc.

`TEST(SPITests, SendWalkingZero)`
- is similar to the previous test, except that 0b11111110 is written, followed by 0b11111101, etc.

`TEST(SPITests, Receive123)`
- tests (simulated) receiving a value from the SPI bus by forcing a single value into the receive buffer via a pointer and then verifying the same value is received by a call to SpiEmulator::read()

`TEST(SPITests, ReceiveWalkingBit)`
- is similar to the previous test, except that 0b00000001 is expected, followed by 0b00000010, etc.

`TEST(SPITests, ReceiveWalkingZero)`
- is similar to the previous test, except that 0b11111110 is expected, followed by 0b11111101, etc.

`TEST(SPITests, LoopBack)`
- simulates writing 255 to the bus, then writing all the numbers from 0 to 255. The output is "looped back" to the input, so that value of each read() should be the same value written by test_loopback() on the previous cycle

### GoogleTest framework: DeviceDriver - test_driver.cpp

`TEST(DeviceDriverTests, test_init)`
- instantiates device driver, verifies device_id and number of channels are both 0. Initializes driver and verifies device_id is as expected in accordance with the datasheet and that the number of channels reported is correct for the device

`TEST(DeviceDriverTests, test_read_and_write_reg)`
- writes a randomly-generated uint8_t to each register and then goes back through each register to confirm the value stored is equal to the value that was written. NOTE: ADC emulator does not currently simulate read-only register values, so this test would need to be updated to run on actual hardware or if read-only registers are implemented in the emulator

`TEST(DeviceDriverTests, test_set_channel_and_read)`
- cycles through all available ADC channels in non-consecutive order and stores the recorded values (which are randomly generated by the ADC emulator upon simulated reset). The channels are again cycled through, this time in a different order, and the same values recorded during the previous cycle are expected. If this test were to be implemented in hardware, constant voltage sources would be used instead of randomly-generated values and it would be unreasonable to expect the exact same values, so range-based expected values would need to be implemented in the test

## Potential next steps:
- Choose a hardware platform and get GPIO working for the relevent pins
- Create or obtain/adapt code for a hardware SPI controller on the chosen platform that implements the ISpiInterface
- Calibrate the ADC, store calibration data, set ADC gain via appropriate register values, and use this information to read actual voltages
- Optionally, implement multi-threaded, continuous ADC reads using direct_read_adc_task() and a Real-Time Operating System (RTOS)
- Improve automated test coverage, potentially incorporate the software into a Hardware In The Loop test setup, attach automated testing (on hardware and software) to CI/CD pipeline so that new pull requests are regression-tested before being pushed into main
