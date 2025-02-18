WORK IN PROGRESS, THIS IS JUST SOME INITIAL NOTES!

// install WSL OR use Linux, then fire it up and type:
>>> sudo apt-get update

// install cmake, the gcc compiler, etc.
>>> sudo apt-get install build-essentials

// install git:
>>> sudo apt-get install git

// hopefully don't need it, but you might have to tell WSL how to find git
>>> sudo add-apt-repository ppa:git-core/ppa -y && sudo apt-get update && sudo apt-get upgrade && sudo apt-get install git -y

// cd into where ever (maybe create a repos folder) and clone the repo
>>> mkdir ~/repos
>>> cd ~/repos/
>>> git clone https://github.com/Ratimus/ADS114S0xB_RLR

// create the <build> folder and cd into it
>>> cd ~/repos/ADS114S0xB_RLR
>>> mkdir build
>>> cd build

// run cmake
>>> cmake ..

// build the project
>>> cmake --build .

// run it
>>> cd ..
>>> ./build/app/app
// or
>>> ./build/driver/<name_of_test>
// or even just
>>> ./driver/ctest

