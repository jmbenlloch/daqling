
### Manually install ERS (temporary)

A Boost installation is required. Execute:

    ansible-playboook install-boost-1_70.yml --ask-become

Step 1:
Clone ERS-git repository from https://gitlab.cern.ch/ep-dt-di/daq/ers.git

    git clone https://gitlab.cern.ch/ep-dt-di/daq/ers.git

Step 2:
Generate build information with cmake
setup.sh from daqling can be used to set environment.
install ers by going to the ers root folder and run the following commands:

    mkdir build
    cd build
    cmake3 ../ -DCMAKE_INSTALL_PREFIX=/opt/ers
    sudo mkdir /opt/ers
    sudo make install

This will install ERS files to `/opt/` in order to maintain separation from system files.

### Build daqling

build daqling usual way:
go to daqling root folder and do

    source cmake/setup.sh
    mkdir build
    cd build
    cmake3 ../
    make

### Run demo (optional)

    cd ..
    daqpy configs/ers-test.json
or
run on of the other demos.
check log file afterwards to see log entries.
