# DAQling

Software framework for development of modular and distributed data acquisition systems.

# Documentation
WIP: Scattered documentation can be found at the following links:
- [Google documents][drive]
- [Overleaf][overleaf]

[drive]: <https://drive.google.com/drive/folders/1sMiRltFLZY9HFLqsrGpXrNlBZx4Yx3qN?usp=sharing>
[overleaf]: <https://www.overleaf.com/9291872198hhwbjgmdstpv>

# Install the daq framework
### Run ansible-playbook to configure your CentOS7 host
The playbook will set up your host with the system libraries and will install supervisord

    sudo yum install -y ansible
    cd ansible/
    source daq-ansible.sh
    cd playbooks/
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

## Software compilation
For `GCC 6.2`:

    source cmake/setup-cvmfs.sh

For `GCC 8.3` (required by Boost 1.70 and TBB 2019):

    source cmake/setup-openhpc.sh

then:

    mkdir build
    cd build
    cmake3 ../
    make

You can also do incremental compilation like:

    make utilities
    make core

### Build with Boost 1.70
In order to include Boost 1.70 in the build it is necessary to:
- have a Boost 1.70 installation under `/opt/boost/` (optional Ansible playbook)
- from a fresh terminal:

      source cmake/setup-openhpc.sh
      cd build
      cmake3 ../ -DENABLE_BOOST=1
      make

### Build with TBB 2019.0
In order to include TBB 2019.0 in the build it is necessary to:
- have a TBB 2019.0 installation under `/opt/tbb-2019_U5/` with `include/` and `lib/` folders
- from a fresh terminal:

      source cmake/setup-openhpc.sh
      cd build
      cmake3 ../ -DENABLE_TBB=1
      make

## Running

    cd scripts/Control

The `settings.json` and `*config*.json` files must be edited to match the desired system.

`python3 daqinterface.py -h` shows the help menu. In order to run a complete example, use:

    python3 daqinterface.py valid-config.json complete
    start
    stop
    down

### Development
In order to develop your own module, check the existing demonstration modules under `src/Modules` and `include/Modules` and adapt the `NewModule*_template` to your use case.

The `dev` option of `daqinterface.py`

    python3 daqinterface.py invalid-config.json complete dev

  - skips the `json-config.schema` validation, therefore allowing to experiment with new fields in the configuration json files.
  - sets the process logging level to `DEBUG`.
