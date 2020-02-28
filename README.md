# DAQling

A software framework for the development of modular and distributed data acquisition systems.

## Documentation

Detailed documentation can be found [here][codimd].

Subscribe to the "daqling-users" CERN e-group for updates.

To contact the developers: daqling-developers@cern.ch (only for "daqling-users" members).

[codimd]: <https://codimd.web.cern.ch/s/B1oArin-r>

## Host configuration and framework build

### Configure the CERN CentOS 7 host

The Ansible playbook will set up the host with the system libraries and tools

    sudo yum install -y ansible
    source cmake/setup.sh
    cd ansible/
    ansible-playbook set-up-host.yml --ask-become

#### (Optional)

Web dependencies

    ansible-playbook install-webdeps.yml --ask-become

Redis

    ansible-playbook install-redis.yml --ask-become

Boost 1.70

    ansible-playbook install-boost-1_70.yml --ask-become

Cassandra (experimental)

    ansible-playbook install-cassandra.yml --ask-become

### Build

    source cmake/setup.sh

then:

    mkdir build
    cd build
    cmake3 ../
    make

It is possible to build specified targets. `make help` will list the available ones.

#### Advanced build options

The `ccmake` command:

    source cmake/setup.sh
    cd build
    ccmake3 ../

allows browsing available build options, such as selection of Modules to be built and Debug flags. E.g.:

    ENABLE_SANITIZE [ON, OFF]
    CMAKE_BUILD_TYPE [Debug, Release]

To generate *Doxygen* documentation for DAQling:

    source cmake/setup.sh
    cd build
    make doc

After generation it is possible to browse the pages by opening `doxygen_html/index.html` with a browser.

#### (Optional) Build with Boost 1.70

To include Boost 1.70 in the build it is necessary to:

- have a Boost 1.70 installation under `/opt/boost/` (optional Ansible playbook)
- from a fresh terminal:

      source cmake/setup.sh
      cd build
      cmake3 ../ -DENABLE_BOOST=1
      make

#### (Optional) Build the CassandraDataLogger

To build the (experimental) CassandraDataLogger it is necessary to:

- have a Cassandra C++ driver installation under `/opt/cassandra-driver/` (optional Ansible playbook)
- from a fresh terminal:

      source cmake/setup.sh
      cd build
      cmake3 ../ -DENABLE_CASSANDRA=1
      make

#### (Optional) Build with TBB 2019.0

To include TBB 2019.0 in the build it is necessary to:

- have a TBB 2019.0 installation under `/opt/tbb-2019_U5/` with `include/` and `lib/` folders
- from a fresh terminal:

      source cmake/setup.sh
      cd build
      cmake3 ../ -DENABLE_TBB=1
      make

## Running the data acquisition system demo

`daqpy` is a command line tool that spawns and configures the components listed in the JSON configuration file passed as argument.

It then allows to control the components via standard commands such as `start` (with optional run number), `stop`, as well as custom commands.

    source cmake/setup.sh
    daqpy configs/demo.json
    start [run_num]
    stop
    down

`daqpy -h` shows the help menu.

## Development

To develop a custom module, the existing modules in `src/Modules` can provide guidance.

It is necessary to copy and rename the template folder `src/Modules/Dummy` and its files to start developing the new module.

The custom module will be discovered and built by CMake as part of the project.

### Run custom Modules

To run a newly created Module (e.g. `MyDummyModule`), it is necessary to add a corresponding entry in `components:` to a JSON configuration file. Note that the name of the Module needs to be specified in the `type:` field. E.g.:

    {
      "name": "mydummymodule01",
      "host": "localhost",
      "port": 5555,
      "type": "MyDummyModule",
      "loglevel": {"core": "INFO", "module": "DEBUG"},
      "settings": {
      },
      "connections": {
      }    
    }
