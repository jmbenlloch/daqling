FROM rockylinux/rockylinux:9.4

RUN dnf install -y git epel-release cmake gcc-c++ git boost-devel libasan libubsan clang-tools-extra libcurl-devel
RUN dnf config-manager --enable crb
RUN dnf install -y cppzmq-devel openssl openssl-devel xmlrpc-c xmlrpc-c-client xmlrpc-c-devel vim python3 python3-pip
RUN dnf clean all

#RUN dnf install -y git epel-release cmake gcc-c++ git boost-devel libasan libubsan clang-tools-extra libcurl-devel
#RUN dnf config-manager --enable crb
#RUN dnf install -y cppzmq-devel
#RUN dnf install -y openssl openssl-devel
#RUN dnf install -y xmlrpc-c xmlrpc-c-client xmlrpc-c-devel
#RUN dnf install -y vim python3 python3-pip

RUN pip install virtualenv ansible

RUN adduser next
USER next
WORKDIR /home/next
RUN git clone --recurse-submodules https://github.com/jmbenlloch/daqling
RUN git clone --recurse-submodules https://gitlab.cern.ch/ep-dt-di/daq/daqling-spack-repo
#RUN virtualenv daqling_env && \
#	pip install -r /home/next/daqling/etc/control-requirements.txt && \
#	pip install -r /home/next/daqling/etc/web-requirements.txt

RUN cd /home/next/daqling && ./cmake/install.sh -d /home/next/daqling-spack-repo/ -c /home/next/daqling/configs/
USER root
#RUN cd /home/next/daqling && ./cmake/install.sh -s -t -w
RUN cd /home/next/daqling && ./cmake/install.sh -s -t -w ; echo finish
RUN chown next:next /log
USER next
RUN cd /home/next/daqling && source cmake/setup.sh && \
	mkdir build && cd build && cmake .. && make -j 15

# as root: supervisord -c /etc/supervisor/supervisord.conf
# as next: scripts/ControlGUI/run.sh
# map ports 5000 and 9001
