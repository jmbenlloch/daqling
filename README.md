# DAQling

A software framework for the development of modular and distributed data acquisition systems.

## Documentation

Detailed documentation can be found at [https://daqling.docs.cern.ch](https://daqling.docs.cern.ch).

Subscribe to the "daqling-users" CERN e-group for updates.

To contact the developers: daqling-developers@cern.ch (only for "daqling-users" members).

## Host configuration and framework build

### Build dependencies installation

In order to build the framework, dependencies must be installed following the instructions at:

https://gitlab.cern.ch/ep-dt-di/daq/daqling-spack-repo

Dependencies installation does not require root access, unless you are installing on CentOS 7.

### Configure the host for running the framework

The Ansible playbook will set up the host with the system libraries and tools

    sudo yum install -y ansible
    cd ansible/
    ansible-playbook set-up-host.yml --ask-become

Note: to set up an Ubuntu server follow the same procedure, using `apt` instead of `yum`.

#### (Optional)

Web dependencies:

    ansible-playbook install-webdeps.yml --ask-become

Local InfluxDB + Grafana stack:

    ansible-playbook install-influxdb-grafana.yml --ask-become

Redis

    ansible-playbook install-redis.yml --ask-become

### Build

For the first-time sourcing of `cmake/setup.sh`, pass the location of the daqling-spack-repo to `cmake/setup.sh`:

    source cmake/setup.sh </full/path/to/daqling-spack-repo/>

More info in `README_SPACK.md`.

Then:

    mkdir build
    cd build
    cmake ../
    make

It is possible to build specified targets. `make help` will list the available ones.

#### Additional build options

The `ccmake` command:

    source cmake/setup.sh
    cd build
    ccmake ../

allows browsing available build options, such as selection of Modules to be built and Debug flags. E.g.:

    ENABLE_SANITIZE [ON, OFF]
    CMAKE_BUILD_TYPE [Debug, Release]

To generate *Doxygen* documentation for DAQling:

    source cmake/setup.sh
    cd build
    make doc

After generation it is possible to browse the pages by opening `doxygen_html/index.html` with a browser.

## Running the data acquisition system demo

`daqpy` is a command line tool that spawns and configures the components listed in the JSON configuration file passed as argument.
It showcases the use of the `daqcontrol` library, which can be used in any other Python control tool.

It then allows to control the components via standard commands such as `start` (with optional run number), `stop`, as well as custom commands.

    source cmake/setup.sh
    daqpy configs/demo.json
    start [run_num]
    stop
    down

`daqpy -h` shows the help menu.

### (NEW) DAQ control tree

`daqtree` is a command line tool that allows to control the data acquisition system as a "control tree", with advanced Finite State Machine (FSM) options.
It showcases the use of the `nodetree` and `daqcontrol` libraries, which can be used in any other Python control tool.

    source cmake/setup.sh
    daqtree configs/demo-dict.json

`daqtree -h` shows the help menu.

The `render` command allows to print the current status of the control tree. The render will print the tree structure with the name of the node, the status and its key flags:

- Included [`True`/`False`]. Nodes can be excluded or included from the controlled tree with `<node> exclude` and `<node> include`. Excluded nodes will not participate to the parent's status and will not receive commands from the parent node.
- Inconsistent [`True`/`False`]. If the children of a node are in different state, the Inconsistent flag is raised.

The control of the system is granular:

- commands can be issued to any node in the tree (for example to `readoutinterface01` only).
- the full FSM of the control system can be navigated (as opposed to `daqpy`).

                  ---add-->         ---boot-->           --configure-->         --start-->
      (not_added)           (added)             (booted)                (ready)            (running)
                  <-remove-         <-shutdown-          <-unconfigure-         <--stop--- 

More details on the JSON configuration required by `daqtree` can be found in the `configs/README.md`.

## Development

To develop a custom module, the existing modules in `src/Modules` can provide guidance.

It is necessary to copy and rename the template folder `src/Modules/Dummy` and its files to start developing the new module.

The custom module will be discovered and built by CMake as part of the project.

### Run custom Modules

To run a newly created Module (e.g. `MyDummyModule`), it is necessary to add a corresponding entry in `components:` to a JSON configuration file. Note that the name of the Module needs to be specified in the `type:` field. E.g.:

```json
{
  "name": "mydummymodule01",
  "host": "localhost",
  "port": 5555,
  "modules": [
   {
     "type": "MyDummyModule",
     "name":"mydummymodule",
     "connections": {
     }
   }
  ],
  "loglevel": {"core": "INFO", "module": "DEBUG","connection":"WARNING"},
  "settings": {
  }    
}
```

## Contributors

DAQling has been developed by the EP-DT-DI section at CERN and is maintained by:

- Enrico Gamberini, CERN, @engamber
- Roland Sipos, CERN, @rsipos
- Marco Boretto, CERN, @mboretto
  
The following authors, in alphabetical order, have contributed to DAQling:

- Wojciech Brylinski, Warsaw University of Technology, @wobrylin
- Zbynek Kral, Czech Technical University in Prague, @zkral
- Jens Noerby Kristensen, CERN, @jkristen
- Giovanna Lehmann Miotto, CERN, @glehmann
- Viktor Vilhelm Sonesten, CERN, @vsoneste
- Clement Claude Thorens, Université de Genève, @cthorens
