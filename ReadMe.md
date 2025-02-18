###This project demonstrates the general approach to developing a device driver for embedded
###platforms, allowing the microcontroller to communicate with and control an ADS114S0xB Analog
##to Digital Converter (ADC) via Serial Peripheral Interface (SPI).

##Project structure:

#/driver
#Bare metal device driver implementing the following functionality:
 - initialization of the device
 - reading and writing data and commands over the SPI bus
 - reading and writing the registers on the device
 - reading ADC values from device

#/app
#User-space application that uses the driver to:
 - read ADC values
 - read register values
 - change ADC channels


##Building and running the project:

#Prerequisites:
- Linux OS (I used Windows System for Linux (WSL) and Ubuntu)
- Project targets C++ 17
- cmake 3.1 or greater
- gcc compiler
- git

The following instructions are for Ubuntu (including Ubuntu under WSL). If you're using some other distro
or trying to use Windows stand alone, use the equivalent commands for your OS.

#Step 1. Set up your build environment
// Install cmake, the gcc compiler, etc.
$ sudo apt-get update
$ sudo apt-get install build-essential

// Install git:
$ sudo apt-get install git

// Hopefully don't need it, but you might have to tell WSL how to find git (you'll have to run the install cmd again)
$ sudo add-apt-repository ppa:git-core/ppa -y && sudo apt-get update && sudo apt-get upgrade && sudo apt-get install git -y

#Step 2. Clone this repository to your machine and set up the project
// Navigate to the folder where you want to store this project (maybe create a repos folder) and clone the repo
$ mkdir ~/repos
$ cd ~/repos
$ git clone https://github.com/Ratimus/ADS114S0xB_RLR

// Create the <build> folder and cd into it
$ cd ~/repos/ADS114S0xB_RLR
$ mkdir build
$ cd build

#Step 3. Build it
$ cmake ..
$ cmake --build .

#Step 4. Run it
// Run app
$ app/app

// Run all tests
$ ctest

// Run individual tests
$ ./build/driver/<name_of_test>

