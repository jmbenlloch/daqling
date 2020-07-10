# Build system
The project's build system is based heavily on CMake. We ship a set of CMake modules inspired from other CERN projects. Some FindXYZ modules exist for some tools here as examples, and in case the build system needs extensions.
The CERN_ENVIRONMENT flag toggles between CC7 packaged installed libraries with includes in /opt to non-CC7 environment with includes in 3rdparty.

# Off-CERN environments
You can build the project on non-CC7 systems, provided you have a relatively modern toolchain, and the 3rdparty dependencies are cloned under your DAQling directory. To do so, please have a look on the 3rdparty directory's README file. Eventually this structure will change, and 3rdparty tools will be fully handled through CMake and Ansible. 
Tested operating systems include:

    Ubuntu 18.04 LTS
    Arch Linux

# Cross-build
One of the main principles of DAQling is to make it run in "lightweight" mode: sockets and queues on embedded systems. For this, we provide some setup scripts that can be easily modified to pick-up Linaro toolchains from x86 to ARM builds. We tested the following list with success:

    aarch64-linux-gnu
    arm-linux-gnueabihf
    armv8l-linux-gnueabihf

Provided you have your Linaro toolchains under /opt/toolchains, run one of the setup scripts:

    source setup-armv8l.sh

At the moment, we don't support HTTP and REST calls from the framework itself. Hence the reason, issue the following CMake command in your build area:

    cmake .. -DCERN_ENVIRONMENT=OFF -DWITH_DOCS=OFF -DBUILD_WITH_CPR=OFF -DBUILD_MetricSimulator=OFF -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DBUILD_CPR_TESTS=OFF

Ansible playbook for cross-build and better 3rdparty support is coming.

