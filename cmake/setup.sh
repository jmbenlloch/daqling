if [ $# -gt 1 ];
then
  echo "Invalid number of arguments: $#"
  return 1
fi

BASEDIR="$(dirname "$(realpath "$BASH_SOURCE")" )"
CONFIG_FILE="${BASEDIR}/setup.cfg"
if [ $# -eq 1 ]
then
  touch ${CONFIG_FILE}
  echo "DAQLING_SPACK_REPO_PATH=$1" > ${CONFIG_FILE}
  if [ -d "/opt/ohpc/pub/compiler/gcc/8.3.0/" ]
  then  
    echo "CUSTOM_GCC_PATH=/opt/ohpc/pub/compiler/gcc/8.3.0/" >> ${CONFIG_FILE}
  else
    echo "CUSTOM_GCC_PATH=" >> ${CONFIG_FILE}
  fi
fi

if [ -s ${CONFIG_FILE} ]
then
  . ${CONFIG_FILE}
else
  echo "No config file - Exiting."
  return 1
fi

export DAQ_CONFIG_DIR=$PWD/configs/
export DAQ_BUILD_DIR=$PWD/build/
export DAQ_SCRIPT_DIR=$PWD/scripts/
daqpy_path=$(find -name daq.py | cut -c3-)
alias daqpy='python3 $PWD/$daqpy_path'
daqtree_path=$(find -name daqtree.py | cut -c3-)
alias daqtree='python3 $PWD/$daqtree_path'

# runtime requirements
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DAQ_BUILD_DIR/lib
export TDAQ_ERS_STREAM_LIBS=DaqlingStreams

echo "Activate Spack DAQling environment"
source ${DAQLING_SPACK_REPO_PATH}/spack/share/spack/setup-env.sh
spack env activate daqling
spack find
spack load cmake

if [ ! -z $CUSTOM_GCC_PATH ]
then 
  echo "Custom compiler, installed from OHPC."
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CUSTOM_GCC_PATH/lib64/
  #export PATH=$PATH:$CUSTOM_GCC_PATH/bin

  #export CC=$CUSTOM_GCC_PATH/bin/gcc
  export CXX=$CUSTOM_GCC_PATH/bin/g++
fi

#export TBB_VERSION=2019.0
#export TBB_ROOT_DIR=/opt/tbb-2019_U5

export BOOST_VERSION=1.70

if [ -f "/opt/rh/llvm-toolset-7/enable" ]
then
  source /opt/rh/llvm-toolset-7/enable
fi
