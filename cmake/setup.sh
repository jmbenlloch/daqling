export DAQ_CONFIG_DIR=$PWD/configs/
export DAQ_BUILD_DIR=$PWD/build/
export DAQ_SCRIPT_DIR=$PWD/scripts/
daqpy_path=$(find -name daq.py | cut -c3-)
alias daqpy='python3 $PWD/$daqpy_path'
daqtree_path=$(find -name daqtree.py | cut -c3-)
alias daqtree='python3 $PWD/$daqtree_path'

echo "Custom compiler, installed by Ansible from OHPC."

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/ohpc/pub/compiler/gcc/8.3.0/lib64/:/usr/local/lib64/:/usr/local/lib/:/opt/boost/lib/
export PATH=$PATH:/opt/ohpc/pub/compiler/gcc/8.3.0/bin

# Export package specific environmental variables

export CC='/opt/ohpc/pub/compiler/gcc/8.3.0/bin/gcc'
export CXX='/opt/ohpc/pub/compiler/gcc/8.3.0/bin/g++'

export TBB_VERSION=2019.0
export TBB_ROOT_DIR=/opt/tbb-2019_U5

export BOOST_VERSION=1.70
export BOOST_ROOT_DIR=/opt/boost

if [ -f "/opt/rh/llvm-toolset-7/enable" ]; then
  source /opt/rh/llvm-toolset-7/enable
fi
