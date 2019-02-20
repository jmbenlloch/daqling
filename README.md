# rd51-daq

DAQ fwk for RD51

## Getting Started

TBA

# Install the rd51 framework

## Set up your host and install supervisord

### Install ansible on CentOS7

    sudo yum install -y ansible

### Run ansible-playbook to configure your host

    cd ansible/
    ansible-playbook set-up-host.yml  --ask-become-pass

## Software compilation

    git submodule init
    git submodule update
    mkdir build
    cd build
    cmake3 ../
    make

### Running

TBA

```
pwd
```


