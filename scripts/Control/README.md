### `settings.json`

Set the `"env"` depending on the gcc distribution version used to build the framework:

- sourcing `setup-cvmfs.sh` (6.2.0):

      "env": "LD_LIBRARY_PATH=/usr/local/lib64/:/cvmfs/sft.cern.ch/lcg/releases/gcc/6.2.0-b9934/x86_64-centos7/lib64"
  
- sourcing `setup-openhpc.sh` (8.3.0):

      "env": "LD_LIBRARY_PATH=/usr/local/lib64/:/opt/ohpc/pub/compiler/gcc/8.3.0/lib64/