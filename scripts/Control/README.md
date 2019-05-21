### `settings.json`

Set the `"env"` depending on the GCC distribution version used to build the framework:

- sourcing `setup-cvmfs.sh` (GCC 6.2):

      "env": "LD_LIBRARY_PATH=/usr/local/lib64/:/cvmfs/sft.cern.ch/lcg/releases/gcc/6.2.0-b9934/x86_64-centos7/lib64"
  
- sourcing `setup-openhpc.sh` (GCC 8.3):

      "env": "LD_LIBRARY_PATH=/usr/local/lib64/:/opt/ohpc/pub/compiler/gcc/8.3.0/lib64/