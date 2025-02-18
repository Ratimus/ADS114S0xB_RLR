# ADS114S0xB Device Driver for Embedded Platforms

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
Automated testing

## Building and Running the Project

### Prerequisites:
- Linux OS (I used Windows Subsystem for Linux (WSL) with Ubuntu)
- Project targets C++17
- CMake 3.1 or greater
- GCC compiler
- Git

The following instructions are for Ubuntu (including Ubuntu under WSL). If you're using another distro or Windows stand-alone, use the equivalent commands for your OS.

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

Navigate to the folder where you want to store this project (maybe create a repos folder) and clone the repo:
```bash
$ mkdir ~/repos
$ cd ~/repos
$ git clone https://github.com/Ratimus/ADS114S0xB_RLR
```

Create the <build> folder and navigate into it:
```bash
$ cd ~/repos/ADS114S0xB_RLR
$ mkdir build
$ cd build
```

### Step 3: Build it
```bash
$ cmake ..
$ cmake --build .
```

### Step 4: Run it
Run app:
```bash
$ app/app
```

Run all tests:
```
$ ctest
```
