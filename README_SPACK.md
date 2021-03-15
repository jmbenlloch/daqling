### Build daqling using spack environment.

## Prerequisites
To build daqling using spack environment, first follow the instruction from the repository at:

https://gitlab.cern.ch/ep-dt-di/daq/daqling-spack-repo

## Building daqling

For the first-time sourcing of `cmake/setup.sh`, pass the location of the daqling-spack-repo to `cmake/setup.sh`:

    source cmake/setup.sh </full/path/to/daqling-spack-repo/>

Note that the path supplied should point to the daqling-spack-repo folder. E.g.:

    /home/user/daqling-spack-repo

A configuration file will be created, persisting the needed information: `cmake/setup.cfg`. This file can be edited to tune the environment.
    
Then proceed with the build:

    mkdir build
    cd build
    cmake ../
    make
