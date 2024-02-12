BASEDIR="$(dirname "$(realpath "$BASH_SOURCE")" )"
CONFIG_FILE="${BASEDIR}/setup.cfg"
if [ -s ${CONFIG_FILE} ]
then
  . ${CONFIG_FILE}
else
  echo "No config file. Creating default"
  echo "DAQ_CONFIG_DIR=$PWD/configs" >> $CONFIG_FILE
fi

DAQLING_REPO_PATH="$(dirname "$(realpath "${BASEDIR}")" )"
export DAQ_CONFIG_DIR=${DAQ_CONFIG_PATH}
export DAQ_BUILD_DIR=$PWD/build/
export DAQ_SCRIPT_DIR=${DAQLING_REPO_PATH}/scripts/
export DAQLING_LOCATION=${DAQLING_REPO_PATH}
daqpy_path=$(find -name daq.py | cut -c3-)
alias daqpy='python3 $PWD/$daqpy_path'
daqtree_path=$(find -name daqtree.py | cut -c3-)
alias daqtree='python3 $PWD/$daqtree_path'

# runtime requirements
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DAQ_BUILD_DIR/lib
export TDAQ_ERS_STREAM_LIBS=DaqlingStreams

#check if virtualenv exists
if [ -f ${DAQLING_REPO_PATH}/etc/daqling_venv/bin/activate ]; then
  echo "Activating Python DAQling virtual environment"
  source ${DAQLING_REPO_PATH}/etc/daqling_venv/bin/activate
fi

if [ -f "/opt/rh/llvm-toolset-7/enable" ]
then
  source /opt/rh/llvm-toolset-7/enable
fi
