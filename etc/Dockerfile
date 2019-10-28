FROM gitlab-registry.cern.ch/ci-tools/ci-worker:cc7

RUN yum install -y ansible
COPY daqling daqling
WORKDIR daqling
RUN source cmake/setup.sh
RUN ansible-playbook ansible/set-up-host.yml --tags minimal

