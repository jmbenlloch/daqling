# DAQling

A software framework for the development of modular and distributed data acquisition systems.

## Documentation

Detailed documentation can be found at [https://daqling.docs.cern.ch](https://daqling.docs.cern.ch).

Subscribe to the "daqling-users" CERN e-group for updates.

To contact the developers: daqling-developers@cern.ch (only for "daqling-users" members).

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

### Build

    source cmake/setup.sh

then:

    mkdir build
    cd build
    cmake3 ../
    make

It is possible to build specified targets. `make help` will list the available ones.

#### Additional build options

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

## Contributors

DAQling has been developed by the EP-DT-DI section at CERN and is maintained by:

- Enrico Gamberini, CERN, @engamber
- Roland Sipos, CERN, @rsipos
- Marco Boretto, CERN, @mboretto
  
The following authors, in alphabetical order, have contributed to DAQling:

- Wojciech Brylinski, Warsaw University of Technology, @wobrylin
- Giovanna Lehmann Miotto, CERN, @glehmann
- Viktor Vilhelm Sonesten, CERN, @vsoneste
- Clement Claude Thorens, Université de Genève, @cthorens
