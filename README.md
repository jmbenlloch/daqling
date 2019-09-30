# DAQling

Software framework for development of modular and distributed data acquisition systems.

## Documentation

Documentation can be found at the following links:

- [CodiMD][codimd]

[codimd]: <https://codimd.web.cern.ch/s/B1oArin-r>

## Install the framework

### Run ansible-playbook to configure your CentOS7 host

The playbook will set up your host with the system libraries and tools

    sudo yum install -y ansible
    source cmake/setup.sh
    cd ansible/
    ansible-playbook set-up-host.yml --ask-become

#### (Optional)

Cassandra

    ansible-playbook install-cassandra.yml --ask-become

Web dependencies

    ansible-playbook install-webdeps.yml --ask-become

Redis

    ansible-playbook install-redis.yml --ask-become

Boost 1.70

    ansible-playbook install-boost-1_70.yml --ask-become

### Build

    source cmake/setup.sh

then:

    mkdir build
    cd build
    cmake3 ../
    make

It is possible to build only selected targets. Check `make help` in order to obtain the list of available ones.

#### (Optional) Build the CassandraDataLogger

In order to build the CassandraDataLogger it is necessary to:

- have a Cassandra C++ driver installation under `/opt/cassandra-driver/` (optional Ansible playbook)
- from a fresh terminal:

      source cmake/setup.sh
      cd build
      cmake3 ../ -DENABLE_CASSANDRA=1
      make

#### (Optional) Build with Boost 1.70

In order to include Boost 1.70 in the build it is necessary to:

- have a Boost 1.70 installation under `/opt/boost/` (optional Ansible playbook)
- from a fresh terminal:

      source cmake/setup.sh
      cd build
      cmake3 ../ -DENABLE_BOOST=1
      make

#### (Optional) Build with TBB 2019.0

In order to include TBB 2019.0 in the build it is necessary to:

- have a TBB 2019.0 installation under `/opt/tbb-2019_U5/` with `include/` and `lib/` folders
- from a fresh terminal:

      source cmake/setup.sh
      cd build
      cmake3 ../ -DENABLE_TBB=1
      make

## Running the demo

    source cmake/setup.sh
    daqinterface configs/valid-config.json
    start
    stop
    down

`daqinterface -h` shows the help menu.

## Development

In order to develop your own module, check the existing modules in `src/Modules` and for guidance.

Copy and adapt the template folder `src/Modules/Dummy` and develop your custom module.

The custom module will be discovered (don't forget to modify the name of the module in the `CMakeLists.txt` file) and built by CMake as part of the project.
